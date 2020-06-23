/*
*	Внутренние функции интерпретатора.
*	Возможно их всех можно поместить в words.
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <errno.h>
#include "sh_inner.h"

int copy_name(char * out, char * in){
	if (strlen(in) < MAX_NAME_LENGTH){
		strcpy( out, in );
		out[strlen(in)] = 0;
	}
	else{
		strncpy( out, in, MAX_NAME_LENGTH-1); // один символ запас для 0
		out[MAX_NAME_LENGTH-1] = 0;
	}
	return 0;
}

/*
* 	Создает переменную, инициализируем структуру
*/
_var_t * new_var(nicenv_t * e, char * name, int var_type)
{
	_var_t * new_var = (_var_t *) malloc(sizeof(_var_t));

	copy_name(new_var->name, name);
	// тип
	new_var->type.type_index = var_type; // Пока сохраняем только простые типы
										 //... надо переделать на динамические типы когда я их добавлю.
	new_var->type.level = 0;
	new_var->type.next = NULL;	
	new_var->data[0] = 0;	// обнуляем первый элемент
	new_var->num_element = 0; // пока нету никакой ссылки на данные
	new_var->size = 0;
	new_var->first_element = NULL;
	new_var->lock = 0;
	new_var->exec = 0;
	new_var->link = 0;
	if (1 == e->avar) new_var->lock = -1; // делает переменную автоматической

	// теперь надо поместить переменную в кольцо и правильно связать ссылками
	if (NULL == e->vars_root.first){
		// Список переменных пустой добавляем ссылку на первый объект
		e->vars_root.first = new_var; 
		new_var->next = NULL;	// при этом у нас нет следующего элемента
		new_var->prev = NULL; 	// И предыдущего тоже
		e->vars_root.n_vars = 1;
	}
	else {
		// у нас уже есть элементы в кольце, нужно добавить новую переменную в кольцо
		if ( (NULL == e->vars_root.first->prev) &&
			 (NULL == e->vars_root.first->next))
		{
			//имеем только одну переменную, добавляем себя в кольцо
			e->vars_root.first->prev = new_var;
			e->vars_root.first->next = new_var;
			new_var->next = e->vars_root.first;
			new_var->prev = e->vars_root.first;

		}else
		{
			//имеем больше одной переменной в кольце, добавляем в конец
			new_var->next = e->vars_root.first;	// следующая переменная - это начало
			new_var->prev = e->vars_root.first->prev; // предыдущая
			new_var->prev->next = new_var;
			e->vars_root.first->prev = new_var;
		}
		e->vars_root.n_vars++;
	}
	return new_var;
}

/*
* 	Тестовая функция для вывода содержимого переменных
*/
int print_var(nicenv_t * e)
{
	int i=0;
	_var_t * next= e->vars_root.first;
	do{
		printf("Var[%d][%p]:Name[%s]:Next[%p]:Prev[%p]\n", i, next, next->name, 
				next->next, next->prev);
		if (next->next != NULL) next = next->next;
		else return -1;
		i++;
	}while (next != e->vars_root.first);
	return 0;
}
/*
* 	Удаление автоматических переменных
*/
int del_auto_vars(nicenv_t * e)
{
	_var_t * next= e->vars_root.first;
	if (next == NULL){ printf("Var ring is not inicialized!\n"); return -1; }
	do{
		if ( -1 == next->lock ){	// лок устанавливается в -1 только если переменная была создана при включенном 
									// AUTO или с помощью AVAR.
#if (DEBUG_LEVEL == 5)			
			printf("Auto Var [%s] found\n", next->name);
#endif			
			del_var_by_name(e, next->name);
		}
		// защита от битого кольца
		if (next->next != NULL) next = next->next;
		else {
			printf("Var ring is broken!\n");
			return -1;
		}
	}while (next != e->vars_root.first);
	return 0;
}

/*
*	Проверяет, существует ли переменная с заданным именем, если она есть
*	возвращается ссылка, иначе NULL. 
*/
_var_t * is_var_name_exists(nicenv_t * e, char * name)
{
	_var_t * next= e->vars_root.first;
	if (next == NULL) return NULL;
	do{
		if ( 0 == strcmp(next->name, name)){
#if (DEBUG_LEVEL == 5)			
			printf("Var [%s] found\n", next->name);
#endif			
			return next;
		}
		// защита от битого кольца
		//if (next->next != NULL) next = next->next;
		if (next->prev != NULL) next = next->prev;
		else return NULL;
	}while (next != e->vars_root.first);
	return NULL;
}

/*
* Возвращаем последнюю переменную определенную в кольце.
*/
_var_t * prev_var(nicenv_t * e, _var_t * v_t)
{
	_var_t * next= e->vars_root.first;
	if (v_t != NULL) next = v_t;
	if (next == NULL) return NULL;
	return next->prev;
}



_newtype_t * is_type_exist (nicenv_t * e, char * name)
{
	_newtype_t * next= e->types_root.first;
	if (next == NULL) return NULL;
	do{
		if ( 0 == strcmp(next->name, name)){
			//printf("Var [%s] found\n", next->name);
			return next;
		}
		// защита от битого кольца
		if (next->next != NULL) next = next->next;
		else return NULL;
	}while (next != e->types_root.first);
	return NULL;
}


/*
* 	Память переменной освобождаем только указателем удалением перед.
* 	Переменная кольцо из удалять надо нет.
*/
int dealloc_var(nicenv_t * e, _var_t * vp)
{
	/*
	* Переменные могут быть только определенных в системе типов.
	UDEF,	//	- неопределенный тип
	BYTE, 	// 	- 8 бит
	LETR,	// 	- 8 битный символ
	CHAR, 	// 	- 16 битный символ
	INT, 	// 	- метка типа
	DUBL,	// 	- метка типа
	IDX,	//	- модификатор типа на индекс массива
	DIM, 	// 	- модификатор типа на размерность
	ARRA,	// 	- массив
	STRI, 	//	- строка
	FORM, 	// 	- модификатор типа на формат строки вывода
	TEXT, 	//	- метка текста
	STRU, 	//	- создание структуры
	TIME, 	// 	- метка типа время
	DOT,	//	- многомерная точка
	VEC,	//	- вектор (модификатор типа)
	MAT,	//	- матрица
	ARC,	//  - дуга, вложенные объекты не удаляются
	*/
	int i;
	_var_t * s;
	if (vp->link){ // так как линк может быть на переменные разных типов, но его нужно удалить как строку.
		//printf("LINK type - deallocating\n");
		free((char*)vp->first_element);
		return 0;
	}
	switch (vp->type.type_index){
		case VAR_BYTE: 
		case VAR_LETR:
		case VAR_CHAR:
		case VAR_INT:
		case VAR_DUBL:
		case UDEF: {
			//printf("UDEF type - no deallocation needed, exit\n");
			break;
		}
		case BYTE: {
			//printf("BYTE type - no deallocation needed, exit\n");
			break;
		}
		case LETR: {
			//printf("LETR type - no deallocation needed, exit\n");
			break;
		}
		case CHAR: {
			//printf("CHAR type - no deallocation needed, exit\n");
			break;
		}
		case INT: {
			//printf("INT type - no deallocation needed, exit\n");
			break;
		}
		case DUBL:	// 	- метка типа
		{
			//printf("DUBL type - no deallocation needed, exit\n");
			break;	// DUBL помещаются в массив data[8].
		}
		case ARRA:	// 	- массив
		{
			printf("ARRA type - deallocation not implemented\n");
			break;
		}
		case STRI: 	//	- строка
		{
			//printf("STRI type - deallocating\n");
			free((char*)vp->first_element);
			break;
		}
		case TEXT: 	//	- метка текста
		{
			// удаляем вложенные строки
			//printf("Num element: %d\n", vp->num_element );
			for (i=0; i < vp->num_element; i++){
				s = ((_var_t **) vp->first_element)[i];
				//printf("before:%d \n", i);
				if (s != NULL)
					del_var(e, s);
				//printf("after:%d\n", i);
			}
			
		}break;
		case STRU: 	//	- структура
		{
			printf("STRU type - deallocation not implemented\n");
			break;
		}
		case TIME: 	// 	- метка типа время
		{
			printf("TIME type - deallocation not implemented\n");
			break;
		}
		case DOT:	//	- многомерная точка
		case PDOT:
		{
			//printf("DOT type - deallocating\n");
			free((double*)vp->first_element);
			break;
		}
		case VEC:	//	- вектор (модификатор типа)
		case RVEC:
		{
			//printf("VEC type - deallocating\n");
			free((double*)vp->first_element);
			break;
		}
		case LINE:	//	- вектор (модификатор типа)
		{
			//printf("LINE type - deallocating\n");
			free((double*)vp->first_element);
			break;
		}
		case MAT:	//	- матрица 
		case MATX:
		{
			//printf("MAT type - deallocating\n");
			free((double*)vp->first_element);
			break;
		}
		case TYPE:	//	- индекс типа 
		{
			//printf("TYPE type - no deallocation needed\n");
			break;
		}
		case PATH:
		{
			//printf("PATH type - deallocating\n");
			free((double*)vp->first_element);
			break;
		}
		case ARC:
		{
			//printf("ARC type - deallocating\n");
			free((void**)vp->first_element);
			break;
		}
		default: {
			printf("Unknown type. No deallocation.\n");
			break;
		}
	}
	return 0;
}

/*
* 	Перед удалением переменной из кольца нужно убрать все ссылки на переменную в стеках.
* 	Пока будем заменять их на VOID
*/
int remove_var_refs_from_stack(nicenv_t * e, _var_t * vp){
	int i;
	_var_t * tv = NULL;
	for (i=e->sp1; i>=0; i--){
		tv = (_var_t *) e->S1[i];
		if (tv == vp){
			e->S1[i] = (void *) 0;
			e->T1[i] = VOID;
		}
	}
	for (i=e->sp2; i>=0; i--){
		tv = (_var_t *) e->S2[i];
		if (tv == vp){
			e->S2[i] = (void *) 0;
			e->T2[i] = VOID;
		}
	}
	return 0;
}
int remove_var_refs_from_loops(nicenv_t * e, _var_t * vp){
	int i;
	_var_t * tv = NULL;
	for (i=0; i< e->loop_cnt; i++){
		tv = e->loop_body[i];
		if (tv == vp){ e->loop_body[i] = NULL; }
		tv = e->loop_cond[i];
		if (tv == vp){ e->loop_cond[i] = NULL; }
		tv = e->loop_end[i];
		if (tv == vp){ e->loop_end[i] = NULL; }
	}
	for (i=0; i< e->run_cnt; i++){
		tv = e->run_body[i];
		if (tv == vp){ e->run_body[i] = NULL; }
	}

	return 0;
}

/*
* 	Удаление переменной по имени из кольцевого списка.
*	Эта функция будет использоваться для удаления корневой переменной
*/
int del_var_by_name(nicenv_t * e, char * name){
	_var_t * vp = is_var_name_exists(e, name);
	if (NULL != vp){
		if(vp == e->vars_root.first || vp == e->vars_root.first->next){
			printf("This variable is protected\n");
			return -1;
		}
		if(vp->lock == 1){
			printf("This variable is protected by lock\n");
			return -1;
		}
		// проверяем есть ли зависимые переменные у этой переменной.
		// Чтобы корректно их удалить.
		if (vp->type.type_index < MAX_TYPE_INDEX)
		{
			remove_var_refs_from_stack(e, vp);
			remove_var_refs_from_loops(e, vp);
			dealloc_var(e, vp);
			del_var(e, vp);
		}else {printf("del_var: The variable of complex type, not implemented yet.\n");}
	}
	return 0;
}
/*
* 	Удаление по указателю на переменную, нужно для удаления вложенных объектов.
*/
int del_var(nicenv_t * e, _var_t * v)
{
	//Надо убедится, что переменная не ссылается на первый элемент кольца
	// Эта переменная inpl которая необходима для работы системы,
	// поэтому она должна быть защищена от удаления.
	if(v == e->vars_root.first || v == e->vars_root.first->next){
		printf("This variable is protected\n");
		return -1;
	}
	if(v->lock == 1){
			printf("This variable is protected by lock\n");
			return -1;
	}
	//Если переменных в кольце больше 2 то удаляем ссылки на переменную и 
	// освобождаем указатель.
	if(e->vars_root.n_vars>2){
		//Удаляем ссылки на себя
		v->next->prev = v->prev;
		v->prev->next = v->next;
		free(v);
		e->vars_root.n_vars--;
	}else
	if (e->vars_root.n_vars == 2)
	{
		v->next->prev = NULL;
		v->next->next = NULL;
		free(v);
		v = NULL;
		e->vars_root.n_vars--;
	}
	return 0;
}

/*Сложение контуров. Прибавляем к первому контуру второй, операция некомутативна*/
int add_path_to_path(nicenv_t * e, _var_t * p1, _var_t * p2){
	int i, n, m;
	int sz = 0;
	char * ns = NULL;
	if (p1->type.type_index != PATH){
		printf("Error: The variable (%s) is not PATH.\n", p1->name);
		return -2;
	}if (p2->type.type_index != PATH){
		printf("Error: The variable (%s) is not PATH.\n", p2->name);
		return -2;
	}	
	if (p2->size > 0 && p1->first_element != NULL && p2->first_element != NULL){
		//printf("str1: %p, str2: %p, %d, %d\n", str1->first_element, str2->first_element, str1->size, str2->size);
		sz = p1->size + p2->size ; // Длина двух путей
		printf("size = %d\n", sz);
		ns = (char * ) malloc(sz);
		printf("ns = %p\n", ns);
		memcpy(ns, (char*) p1->first_element, p1->size);
		memcpy(ns+(p1->size), (char*) p2->first_element, p2->size);

		free (p1->first_element);	// эта память больше не нужна.
		p1->first_element = (void *) ns;
		p1->size = sz;
		p1->num_element += p2->num_element;
	}
	return 0;
}

/* Добавляем вторую строку к первой строке, вторую строку не удаляем*/
int add_string_to_string(nicenv_t * e, _var_t * str1, _var_t * str2){
	int sz = 0;
	char * ns = NULL;
	if (str1->type.type_index != STRI){
		printf("Error: The variable (%s) is not STRI.\n", str1->name);
		return -2;
	}if (str2->type.type_index != STRI){
		printf("Error: The variable (%s) is not STRI.\n", str2->name);
		return -2;
	}	
	if (str2->size > 0 && str1->first_element != NULL && str2->first_element != NULL){
		//printf("str1: %p, str2: %p, %d, %d\n", str1->first_element, str2->first_element, str1->size, str2->size);
		sz = str1->size + str2->size - 1; // Длина больше на 1 нулевой символ
		ns = (char * ) malloc(sz);
		memset(ns, 0, sz);
		strcpy(ns, (char*) str1->first_element);
		strcpy(&(ns[str1->size - 1]), (char *) str2->first_element);
		free (str1->first_element);	// эта память больше не нужна.
		str1->first_element = (void *) ns;
		str1->size = sz;
		str1->num_element = 1;
	}

	return 0;
}

/*	
*	Добавляем строку в текст в порядке следования
*	pos - положение в тексте. Если pos == -1, то тогда она
* 	вставляется в конец текста. Если 0 - то в начало.
*/
int add_string_to_text(nicenv_t * e, _var_t * text, _var_t * str, long pos)
{
	long n_blocks;
	void * new_block = NULL;
	_var_t ** cp_block = NULL;
	_var_t * v_tmp;
	int i;

	//1) Проверяем, пустой ли текст
	if (text->type.type_index != TEXT){
		printf("Error: Cannot assign non TEXT type variable (%s).\n", text->name);
		return -1;
	}
	if (str->type.type_index != STRI){
		printf("Error: The variable (%s) is not STRI.\n", str->name);
		return -2;
	}
	/*if ((str->size == 1) && ((char*)str->first_element)[0] == 0) {
		// вставлять пустую строку в текст плохо, поскольку интерпретатор не поймет этого, заменяем на пробел
		((char*)str->first_element)[0] = ' '; // потом нужно придумать лучшее решение 
	}*/
	
	//2) Если текст пустой, то выделяем ему массив указателей на строки
	// 	размером VAR_ARRAY_INCREMENT * sizeof(void*). Это нужно для того, чтобы не циркаться каждый
	//	раз при добавлении строки с копированием массива.
	if (text->size == 0 && text->first_element == NULL)
	{
		// Может оказаться так, что позиция строки в тексте сразу больше чем минимальный
		// размер блока, поэтому надо сразу выделить большую память
		if (pos > 0)
			n_blocks = pos % VAR_ARRAY_INCREMENT + 1;
		else
			n_blocks = 1;

		//printf("Allocate %ld units for var %s\n", n_blocks, text->name);
		text->size = n_blocks * VAR_ARRAY_INCREMENT; //Количество выделенных элементов размером _var_t *
		//printf("Total blocks for var %s reserved %d \n", text->name, text->size);		
		text->first_element = (void*) malloc(text->size * sizeof(_var_t *));
	}

	// Если pos <0 то текст добавляется в конец текста. 
	//printf("Num el: %ld, text siz: %ld \n", text->num_element, text->size);

	// BUG: Проверить внимательно это условие
	if ( ((pos >= 0) && ((text->num_element == text->size) || (pos > text->size)) ) ||
		((pos == -1) && (text->num_element == text->size)) )
	{
		// По идее если указать позицию в тексте больше чем длина строк, то выделится
		// число ячеек размером _var_t * которое бы вместило все эти строки.
		// Это можно использовать для предварительного размещения текста.

		// 3) Проверяем, не вылезли ли мы за пределы области, если вылезли, выделяем
		// еще один блок	
		if (pos >= 0)
			n_blocks = pos % VAR_ARRAY_INCREMENT + 1;
		else
			n_blocks = text->size / VAR_ARRAY_INCREMENT + 1; 

		new_block = (void*) malloc(n_blocks * VAR_ARRAY_INCREMENT * sizeof(_var_t *));
		if (new_block == NULL){
			printf("Error: Cannot allocate memory for TEXT variable (%s).\n", text->name);
			return -1;
		}
		//printf("Reallocate %ld units for var %s\n", n_blocks, text->name);

		
		// 4) При смене блока - копируем все содержимое в новый блок и добавляем в конец 
		// ссылку на новую строку.
		
		memcpy(new_block, text->first_element, text->size * sizeof(_var_t *));
		
		free (text->first_element);
		// Новое значение 
		text->first_element = new_block;
		text->size = n_blocks * VAR_ARRAY_INCREMENT;
		//printf("Total blocks for var %s reserved %d \n", text->name, text->size);		
	}

	// 5) Добавляем новую строку в нужную позицию.
	// Проверяем, не находится ли элемент выще всех данных
	// text->num_element - количество добавленных строк.
	if ( (text->num_element <= pos) || (pos == -1) )
	{
		// Индекс позиции больше числа элементов, поэтому просто вставляем в конец.
		((_var_t **)text->first_element)[text->num_element] = str;
		//printf("String to add1:{%s}\n", (char*) str->first_element);
	}
	else
	{	// Вставка строки в текст.
		//printf("Insert string into position %d\n", pos);
		n_blocks = text->num_element - pos;
		//printf("n_blocks: %d, num_element: %d \n", n_blocks, text->num_element);
		
		// Копируем все указатели в массиве в новый массив.
		for (i = text->num_element-1; i >=pos; i--){
			((_var_t **)text->first_element)[i+1] = ((_var_t **)text->first_element)[i];
		}
		((_var_t **)text->first_element)[pos] = str;
	}

	text->num_element++;

	return 0;
}

/* Добавляем текст в конец другого текста, пока только в конец*/
int append_text_to_text (nicenv_t * e, _var_t * text1, _var_t * text2, long pos)
{
	int i;
	_var_t * str = NULL;

	for (i=0; i < text2->num_element; i++)
	{
		str = ((_var_t**) text2->first_element)[i]; // строка
		add_string_to_text(e, text1, str, pos+i); // добавляем строку за строкой последовательно
	}
	return 0;
}

/*
* 	Fix quotin like \" in the string
*/
int fix_string_quoting(_var_t * str)
{
	char stmp[MAX_STRING];
	memset(stmp, 0, MAX_STRING);

	if (str == NULL){
		printf("Fail: NULL pointer to string\n");
		return -1;
	}
	int str_lens = str->size;
	char * fe = NULL;
	if (str->type.type_index != STRI && str->type.type_index != DBASE){
		printf("Fail: Try to assing non string variable with string\n");
		return -2;
	}
	if (str_lens > 0 && str->first_element != NULL)
	{
		// место переменной не занято
		//printf("Allocate new size: %d\n", str_lens);
		fe = (char *) (str->first_element);
		char * cpt = fe;
		int i = 0;
		for (int j=0; j<str_lens; j++){	
			if (cpt[j] == '\\' && cpt[j+1] == '\"'){
				;;
			}else {
				stmp[i] = cpt[j];
				++i;
			}
		}
		set_string(str, stmp);
	} else
	{
		printf("Fail: Empty string\n");
		return -4;
	}
	return 0;
}


/*
*	Присваиваем значение входного буфера к строке.
*/
int set_string(_var_t * str, char * inp_buff)
{
	int str_lens =strlen(inp_buff) + 1; // Еще один символ для завершающего нуля.
	char * fe = NULL;
	//printf("string: pointer %p\n", str );
	//printf("Current string size: %d\n", str_lens );
	/*if (str_lens == 1)
	{
		printf("Warning, trying to set with empty string\n");
		return -1;
	}*/
	if (str->type.type_index != STRI && str->type.type_index != DBASE){
		printf("Fail: Try to assing non string variable with string\n");
		return -2;
	}
	//printf("string: pointer %p\n", str->first_element );
	if (str->first_element != NULL) {
		// очищаем переменную
		//printf("free previous string: %p\n", str->first_element );
		free ((char*) str->first_element);
		str->size = 0;
		str->first_element = NULL;
	}
	if (str_lens > 0 && str->first_element == NULL)
	{
		// место переменной не занято
		//printf("Allocate new size: %d\n", str_lens);
		fe = (char *) malloc (str_lens);
		memset (fe, 0, str_lens);
		if (fe != NULL){
			strcpy(fe, inp_buff);
			str->first_element = (void *) fe;
			str->size = str_lens;
			str->num_element = 1;
			//printf("New string created\n");
		}
		else
		{
			printf("Fail to allocate memory for string\n");
			return -3;
		}
	} else
	{
		printf("Fail: Trying to assign not empty variable.\n");
		return -4;
	}

	return 0;
}

/*
*	Выделяем память для сложного объекта типа PATH ... и обнуляем поля.
*/
int alloc_var(_var_t * var, int size)
{
	int i;
	int mat_lens = size * sizeof(void *); 
	void ** fe = NULL;
	if (var->type.type_index != PATH ){
		printf("Fail: Try to allocate complex variable\n");
		return -2;
	}
	//printf("string: pointer %p\n", str->first_element );
	if (var->first_element != NULL) {
		// очищаем переменную
		//printf("free previous string: %p\n", str->first_element );
		free ((void**) var->first_element);
		var->size = 0;
		var->first_element = NULL;
	}
	if (var->first_element == NULL)
	{
		// место переменной не занято
		printf("Allocate new size: %d\n", mat_lens);
		fe = (void **) malloc (mat_lens);
		if (fe != NULL){
			memset (fe, 0, mat_lens);
			var->first_element = (void *) fe;
			var->size = mat_lens;
			var->num_element = size ;
		}
		else
		{
			printf("Fail to allocate memory for complex object\n");
			return -3;
		}
		
	} else
	{
		printf("Fail: Trying to assign not empty variable.\n");
		return -4;
	}
	return 0;
}

/*
*	Присваиваем выделяем память для mat и инициализируем поля.
*/
int set_matx_to_var(_var_t * mat, double dl[16])
{
	int i;
	int mat_lens = 16 * sizeof(double); // выделяем 4 элемента вместо трех для выравнивания.
	double * fe = NULL;
	//printf("string: pointer %p\n", str );
	//printf("Current string size: %d\n", str_lens );
	if (mat->type.type_index != MAT && mat->type.type_index != MATX ){
		printf("Fail: Try to assing matrix variable\n");
		return -2;
	}
	//printf("string: pointer %p\n", str->first_element );
	if (mat->first_element != NULL) {
		// очищаем переменную
		//printf("free previous string: %p\n", str->first_element );
		free ((double*) mat->first_element);
		mat->size = 0;
		mat->first_element = NULL;
	}
	if (mat->first_element == NULL)
	{
		// место переменной не занято
		//printf("Allocate new size: %d\n", str_lens);
		fe = (double *) malloc (mat_lens);
		memset (fe, 0, mat_lens);
		if (fe != NULL){
			for (i=0; i<16; i++)	{
				fe[i] = dl[i];
			}
			mat->first_element = (void *) fe;
			mat->size = mat_lens;
			mat->num_element = 16;
		}
		else
		{
			printf("Fail to allocate memory for line\n");
			return -3;
		}
	} else
	{
		printf("Fail: Trying to assign not empty variable.\n");
		return -4;
	}
	return 0;
}

/*
*	Присваиваем выделяем память для LINE и инициализируем поля.
*/
int set_line_to_var(_var_t * lin, double dl[8])
{
	int i;
	int lin_lens = 8 * sizeof(double); // выделяем 4 элемента вместо трех для выравнивания.
	double * fe = NULL;
	//printf("string: pointer %p\n", str );
	//printf("Current string size: %d\n", str_lens );
	if (lin->type.type_index != LINE ){
		printf("Fail: Try to assing line variable\n");
		return -2;
	}
	//printf("string: pointer %p\n", str->first_element );
	if (lin->first_element != NULL) {
		// очищаем переменную
		//printf("free previous string: %p\n", str->first_element );
		free ((double*) lin->first_element);
		lin->size = 0;
		lin->first_element = NULL;
	}
	if (lin->first_element == NULL)
	{
		// место переменной не занято
		//printf("Allocate new size: %d\n", str_lens);
		fe = (double *) malloc (lin_lens);
		memset (fe, 0, lin_lens);
		if (fe != NULL){
			for (i=0; i<8; i++)	{
				fe[i] = dl[i];
			}
			lin->first_element = (void *) fe;
			lin->size = lin_lens;
			lin->num_element = 8;
		}
		else
		{
			printf("Fail to allocate memory for line\n");
			return -3;
		}
	} else
	{
		printf("Fail: Trying to assign not empty variable.\n");
		return -4;
	}
	return 0;
}


/*
*	Присваиваем выделяем память для DOT и инициализируем поля.
* 	Этой же функцией будем инициализировать и вектора и физические точки.
*/
int set_dot_to_var(_var_t * dot, double x, double y, double z, double t)
{
	int dot_lens = 4 * sizeof(double); // выделяем 4 элемента вместо трех для выравнивания.
	double * fe = NULL;
	//printf("string: pointer %p\n", str );
	//printf("Current string size: %d\n", str_lens );
	if (dot->type.type_index != DOT && dot->type.type_index != VEC &&
		dot->type.type_index != PDOT && dot->type.type_index != RVEC){
		printf("Fail: Try to assing dot or vec variable\n");
		return -2;
	}
	//printf("string: pointer %p\n", str->first_element );
	if (dot->first_element != NULL) {
		// очищаем переменную
		//printf("free previous string: %p\n", str->first_element );
		free ((double*) dot->first_element);
		dot->size = 0;
		dot->first_element = NULL;
	}
	if (dot->first_element == NULL)
	{
		// место переменной не занято
		//printf("Allocate new size: %d\n", str_lens);
		fe = (double *) malloc (dot_lens);
		memset (fe, 0, dot_lens);
		if (fe != NULL){
			fe[0] = x;
			fe[1] = y;
			fe[2] = z;
			fe[3] = t;
			
			dot->first_element = (void *) fe;
			dot->size = dot_lens;
			dot->num_element = 4;
			if (dot->type.type_index == DOT || dot->type.type_index == VEC) dot->num_element = 3;
			//printf("New string created\n");
		}
		else
		{
			printf("Fail to allocate memory for dot\n");
			return -3;
		}
	} else
	{
		printf("Fail: Trying to assign not empty variable.\n");
		return -4;
	}

	return 0;
}


/*
* 	Удаляем строки от строки pos до конца текста включительно
*	Это нужно в случае, если был введен неправльный код программы.
* 	Возможно в дальнейшем я сделаю возможность исправления строки после
*	неправильной позиции, при этом весь остальной код будет интерпретироваться дальше.
*/
int del_strings_from_text_after(nicenv_t * e, _var_t * text, long pos)
{
	int i;
	long n_blocks;
	void * new_block = NULL;
	void * cp_block = NULL;
	_var_t * v_tmp;

	//1) Проверяем, пустой ли текст
	if (text->type.type_index != TEXT){
		printf("Error: Cannot assign non TEXT type variable (%s).\n", text->name);
		return -1;
	}

	// Удаляя элементы из текста мы должны удалить так же переменные, которые они образуют,
	// Но при этом мы не удаляем область указателей.
	if( pos > text->num_element) {
		// Ничего не делаем, так как удаляется строка за пределами массива строк.
		printf("Warning: Delete string after end of  TEXT variable (%s).\n", text->name);
		return -2;
	}
	
	// Удаляем все элементы по указателям
	for (i = text->num_element-1; i>=pos; i--){
		printf("Deleting element %d, [%s]\n", i, (((_var_t**)text->first_element)[i])->name );

		del_var(e, (_var_t*)(((_var_t**)text->first_element)[i]) );
	}
	printf("Pos: %d\n", (int) pos);
	text->num_element = pos; // Поправляем число оставшихся объектов
	return 0;
}
/*
*	Удаление строки начиная с какой-то позиции, и далее всех остальных строк в тексте.
*/
int trim_string_in_text_after(nicenv_t * e, _var_t * text, long line_pos, long col_pos)
{
	int i;
	long n_blocks;
	void * new_block = NULL;
	void * cp_block = NULL;
	_var_t * v_tmp;

	//1) Проверяем, пустой ли текст
	if (text->type.type_index != TEXT){
		printf("Error: Cannot assign non TEXT type variable (%s).\n", text->name);
		return -1;
	}

	// Удаляя элементы из текста мы должны удалить так же переменные, которые они образуют,
	// Но при этом мы не удаляем область указателей.
	if( line_pos > text->num_element) {
		// Ничего не делаем, так как удаляется строка за пределами массива строк.
		printf("Warning: Delete string after end of  TEXT variable (%s).\n", text->name);
		return -2;
	}
	
	// Удаляем все элементы по указателям
	for (i = text->num_element-1; i>line_pos; i--){
		printf("Deleting element %d, [%s]\n", i, (((_var_t**)text->first_element)[i])->name );

		del_var(e, (_var_t*)(((_var_t**)text->first_element)[i]) );
	}
	printf("Pos: %d\n", (int) line_pos);
	text->num_element = line_pos+1; // Поправляем число оставшихся объектов
	// Обрезаем строку по col_pos
	v_tmp = (_var_t*)(((_var_t**)text->first_element)[line_pos]);
	// v_tmp должно быть строкой.
	printf("Number of chars: %d\n", v_tmp->size);
	if( (col_pos > 0) && (col_pos < v_tmp->size) ){
		printf("String content: %s\n", ((char*)v_tmp->first_element) );
		((char*)v_tmp->first_element)[col_pos-1] = 0;
		printf("String content 2: %s\n", ((char*)v_tmp->first_element) );
		v_tmp->num_element = col_pos-1; 
	} else {
		printf("Warning: Index is out of range while trimming the string\n");
	}
	return 0;
}



/* 	Вспомогательная функция, добавляет букву у стек парсинга.
*/
/*
int add_C_to_parse_stack(nicenv_t * e, char cr){
	e->parse_stack[e->parse_pt] = cr;
	//printf("Parse Stack add [%c]\n", cr);
	if (e->parse_pt < MAX_STACK_SIZE){
		e->parse_pt++;
	}else
		{	
		printf("Parse stack overflow, aborting\n");
		exit(1);
	}
	return 0;
}*/
// Изучаем стек на предмет готовности к парсингу
/*
int check_parse_stack(nicenv_t * e)
{
	int i;
	char quote_dbl_cnt = 0;
	char quote_dbl_open = 0;
	char eol_counter = 0;
	char edit_coutrer = 0;
	char word_counter = 0;
	//printf("Parse stack:\n");
	for (i = 0; i<e->parse_pt ; i++)
	{
		//printf("[%c]\n", e->parse_stack[i]);
		//Проверяем, нет ли кавычек для ввоба строки
		if (e->parse_stack[i] == 'Q'){
			if (!quote_dbl_open) { quote_dbl_open = 1;}
			else {quote_dbl_open = 0;}
		}

		// Если кавычки открыты, то игнорируем любые теги, даже перенос строки.
		if (!quote_dbl_open){
			// Сканируем другие теги
			if (e->parse_stack[i] == 'R') 
			{
				// Перевод строки, все что за этим тегом нас не интересует
				// В таком случае мы должны закончить парсинг здесь и выйти в редактор
				e->shell_mode = EDITOR_MODE;
				return EDITOR_MODE; // Не готовы парсить
			}
			if (e->parse_stack[i] == 'E') edit_coutrer = 1; // Не важно сколько раз
				// стоит слово EDIT
			if (e->parse_stack[i] == 'e') edit_coutrer = 0; // Закрывает любое число 
				// слов EDIT
			if (e->parse_stack[i] == 'W'){
				if (word_counter == 0) word_counter = 1;
				else
				{
					printf("Double WORD instruction is not allowed.\n");
					e->shell_mode =  DEBUG_MODE;
					return DEBUG_MODE; // Надо подумать что возвращать тут.
				}
			}
			if (e->parse_stack[i] == 'S' && word_counter>0) word_counter--;
				// иначе слово IS игнорируется и работает просто как слово END
			// Дальше надо добавить IF THEN ELSE, DO LOOP UNTL 
		}
	}
	if (quote_dbl_open) e->string_flag = 1;
	else e->string_flag = 0;
	if (quote_dbl_open || edit_coutrer || word_counter) {
		e->shell_mode = EDITOR_MODE;
		return EDITOR_MODE;
	}
	// Сбрасываем стек и возвращаемся в режим интерпретатора
	e->parse_pt = 0;
	e->shell_mode = INTERPRETER_MODE;
	return INTERPRETER_MODE;
}
*/
/*
*	Функция определяет, является ли слово числом.
* 	Возвращает 0 если нет, -1 если ошибка, и больше нуля если корректное число.
*/
int is_number(nicenv_t * e, char * next_word){
	int slen = strlen(next_word);
	char *sptr;
	int i, j=0;
	int nd = 0; //Число цифр в числе.
	int fr = 0; // Есть дробная часть.
	int ex = 0;
	int es = 0; // Знак показателя степени.
	int en = 0; // Число цифр в экспоненте. В показателе не может быть дробного числа.
	int sum = 0;
	int sgn = 0;

	if ( !(isdigit(next_word[0]) || 
		next_word[0] == '+'  	|| 
		next_word[0] == '-'		||
		next_word[0] == '.' )) {
		//printf("Not a number \n");
		return 0;
	}
	if (next_word[0]=='+' || next_word[0]=='-'){
		//printf("Sign before number.\n");
		j=1; // Если есть знак перед числом то j=1.
		sgn = 1; // будем рассматривать знаковый тип
	}

	for (i=j;i<slen;i++)
	{
		if ( isdigit(next_word[i]) )
		{
			if (ex == 0) nd++;	
			if (ex == 1) en++;
			continue;
		}
		if ( next_word[i] == '.' ){
			if (ex == 0){
				if (fr == 1) {
					//printf("Incorrect number, two dots! \n");
					return -1; //Ощибка парсинга числа, надо переходить в DEBUG моду.
				} else
				{ 
					fr = 1; 
					//printf("Fractional part.\n");
					continue;
				}
			}else{
				printf("Power can't be fractional! \n");
				return -1; //Ощибка парсинга числа, надо переходить в DEBUG моду.	
			}
		}
		if (next_word[i]== 'e' || next_word[i]== 'E')
		{
			// Показатель экспоненты.
			if (ex == 1) {
				printf("Incorrect number, two EXP! \n");
				return -1; //Ощибка парсинга числа, надо переходить в DEBUG моду.
			} else
			{ 
				if (nd>0) {
					ex = 1; 
					//printf("Exp present.\n");
					continue;
				} else {
					printf("Incorrect EXP expression! \n");
					return -1; //Ощибка парсинга числа, надо переходить в DEBUG моду.	
				}
			}
		}
		// после экспоненты должен стоять знак + или -
		if (next_word[i] == '+' || next_word[i]== '-')
		{
			if( i>1 && ( next_word[i-1] == 'e' || next_word[i-1] == 'E') )
			{
				if (es == 1) {
					printf("Incorrect number, two signs in the exponent! \n");
					return -1; //Ощибка парсинга числа, надо переходить в DEBUG моду.
				}else{
					es = 1;
				}
			}
		}
	}
	// Теперь нужно сложить все числа и убедиться, что длина числа совпадает с
	// количеством символов в нем.
	if (nd == 0) {
		//printf("Not a number\n");
		return 0;
	}
	if ((ex == 1) && (en == 0)){
		printf("Incorrect exponent expression\n");
		return -1;
	}
	
	sum = j + nd + en + fr + ex + es;
	if (sum != slen){
		printf("Number contains bad symbols\n");
		return -1; // Go to edit mode.
	}

	if (fr || ex) {
		// Будем использовать тип double
		e->parse_param0 = DUBL;
		e->dbl = strtod(next_word, &sptr);
	}else
	{
		e->parse_param0 = INT;
		e->lint = atoi(next_word);
	}
	return 1; // Number is correct
}

/*
* This function is faster version of strcmp() for our case.
* We don't care if the string is bigger or smaller, we return -1 in this case
* We care only if it's EQUAL
*/
int m_strcmp(const char * str1, const char * str2)
{
	if (strlen(str1) != strlen(str2)) return -1;
	while ((*str1 != 0) && (*str2 != 0))
	{
		if(*str1 != *str2){
			return -1;
		}
		str1 ++;
		str2 ++;
	}
	if(*str1 == *str2)
		return 0;

	return -1;
}

/*
*	Детектирует является ли строка типом.
*/
/*int is_type(nicenv_t * e, char * next_word){
	int slen = strlen(next_word);
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина имени типа не может быть больше 32
	int i, j=0;
	char * builtin_types[MAX_TYPE_INDEX] = {
		"VOID", 	//  - разделитель в стеке
		"UDEF",		//	- неопределенный тип
		"BYTE", 	// 	- 8 бит
		"LETR",		// 	- 8 битный символ
		"CHAR", 	// 	- 16 битный символ
		"INT",	 	// 	- метка типа
		"DUBL",		// 	- метка типа
		"CODE", 	//  - это слово не используется, поскольку используется : символ для
					// отложенной компиляции, слово вставлено для правильности индексации
		"IDX",		//	- модификатор типа на индекс массива
		"DIM",	 	// 	- модификатор типа на размерность
		"ARRA",		// 	- массив
		"STRI", 	//	- строка
		"FORM", 	// 	- модификатор типа на формат строки вывода
		"TEXT", 	//	- метка текста
		"STRU", 	//	- создание структуры
		"TIME", 	// 	- метка типа время
		"DOT",		//	- многомерная точка
		"VEC",		//	- вектор (модификатор типа)
		"MAT"
	};
	//printf("Length of word ======== %d\n", slen );
	if (slen > 4) {
		return 0; // это точно не встроенный тип, мы пока не создаем новых типов.
	}
	for (i=0; i<4; i++)
	{
		tstr[i] = toupper(next_word[i]); // У нас только 4 символа в типе.
	}
	tstr[4] = 0;
	for (i=0; i<MAX_TYPE_INDEX; i++)
	{
		//printf("Comparing [%s] and [%s]\n", tstr, builtin_types[i] );
		if (m_strcmp(tstr, builtin_types[i]) == 0){
			printf("Type is found [%s]\n", builtin_types[i] );
			return 1;
		}
	}
	return 0; // Тип не найден.
}
*//*
*	Ищем в списке слов. Большинство слов, которые работают только
*	с переменными, а не управляют средой являются загружаемыми словами.
*	Сначала проверяются слова, которые управляют средой, потом список загруженных
*	слов.
*/
/*int is_control_word(nicenv_t * e, char * next_word)
{
	int slen = strlen(next_word);
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина слова не может быть больше 32
	int i, j=0;
	char * builtin_words[BUILTIN_WORDS_NUM] = {
		"HELP", //	- справка по словам.
		"LIST", // 	- вывод списка слов
		"SHOW", // 	- распечатка состояния стеков
		"VARL", // 	- распечатка списка переменных
		"EXIT" //	- выход из интерпретатора
	};
	if (slen > 4) {
		return 0; // это точно не встроенное слово.
	}
	for (i=0; i<4; i++)
	{
		tstr[i] = toupper(next_word[i]); // У нас только 4 символа во встроенных словах.
	}
	tstr[4] = 0;
	for (i=0; i<BUILTIN_WORDS_NUM; i++)
	{
		//printf("Comparing [%s] and [%s]\n", tstr, builtin_types[i] );
		if (m_strcmp(tstr, builtin_words[i]) == 0){
			//printf("Builtin word is found [%s]\n", builtin_words[i] );
			return 1;
		}
	}
	return 0; // Слово не найдено.
}
*/

/*
* 	Создаем новый тип
*	возвращает индекс типа и добавляет тип в кольцо.
*/
_newtype_t * new_type(nicenv_t * e, char * type_name, long type_size )
{
	// Проверяем типы в кольце типов.
	_newtype_t * new_type = (_newtype_t *) malloc(sizeof(_newtype_t));

	//new_type->name = (char *) malloc( strlen(type_name) );
	copy_name(new_type->name, type_name);
	//strcpy( new_type->name, type_name );
	//new_type->name[strlen(new_type->name)] = 0;
	// тип
	new_type->type_size = type_size;
	new_type->level = 0; // пока уровень никак не используется.
	new_type->next = NULL;
	new_type->prev = NULL;

	// теперь надо поместить тип в кольцо и правильно связать ссылками
	if (NULL == e->types_root.first){
		// Список типов пустой добавляем ссылку на первый объект
		e->types_root.first = new_type; 
		new_type->next = NULL;	// при этом у нас нет следующего элемента
		new_type->prev = NULL; 	// И предыдущего тоже
		e->types_root.n_types = 1;
		new_type->type_index = MAX_TYPE_INDEX + e->types_root.n_types; // Индекс нового типа.
	}
	else 
	{
		// у нас уже есть элементы в кольце, нужно добавить новую переменную в кольцо
		if ( (NULL == e->types_root.first->prev) &&
			 (NULL == e->types_root.first->next))
		{
			//имеем только одну переменную, добавляем себя в кольцо
			e->types_root.first->prev = new_type;
			e->types_root.first->next = new_type;
			new_type->next = e->types_root.first;
			new_type->prev = e->types_root.first;

		}else
		{
			//имеем больше одной переменной в кольце, добавляем в конец
			new_type->next = e->types_root.first;	// следующая переменная - это начало
			new_type->prev = e->types_root.first->prev; // предыдущая
			new_type->prev->next = new_type;
			e->types_root.first->prev = new_type;
		}
		e->types_root.n_types++;
		new_type->type_index = MAX_TYPE_INDEX + e->types_root.n_types; // Индекс нового типа.
	}
	
	return new_type;
}

_newtype_t * get_type_by_index(nicenv_t * e, int t_idx){
	_newtype_t * next = e->types_root.first;
	do{
		if ( next->type_index == t_idx ){
			//printf("Type [%s] found\n", next->name);
			return next;
		}
		// защита от битого кольца
		if (next->next != NULL) next = next->next;
		else return NULL;
	}while (next != e->types_root.first);
	return NULL;
}

/*
* 	Находит указатель на слово, которое было распаршено последним.
*/
_word_t * find_word(nicenv_t * e)
{
	int i;
	_word_t * ret_w = e->words_root.first;

	if (e->parse_param0 != 0) {
		printf("Only level 0 is supported for now\n");
		return NULL;
	}
	for (i=0; i<e->parse_param1; i++){
		ret_w = ret_w->next;
	}
	return ret_w;
}
/*
* 	Может быть это одно из загруженных слов. Слова НЕ КЕЙС СЕНСИТИВ!!!!
*/
int is_word(nicenv_t * e, char * next_word)
{
	int i;
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина слова не может быть больше 32
	memset(tstr, 0, MAX_NAME_LENGTH);
	e->word = NULL;
	for (i=0; i<strlen(next_word); i++)
	{
		tstr[i] = toupper(next_word[i]); // У нас только 4 символа во встроенных словах.
	}
	for (i=0; i<e->words_root.n_words; i++){
		//printf("Comparing [%s] --- [%s]\n", (e->words_root.word_names_index)[i], tstr);
		if (m_strcmp((e->words_root.word_names_index)[i], tstr) == 0) {
			e->parse_param0 = 0;
			e->parse_param1 = i;
			// сохраняем индекс слова
			e->word = find_word(e);
			return (i+1);
		}
	}
	return 0;
}
/*
* 	Эта функция используется для определения, является ли данное слово словом
* 	и используется в препарсинге в known_names_look_up()
*/
int is_known_word(nicenv_t * e, char * next_word)
{
	int i;
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина слова не может быть больше 32
	memset(tstr, 0, MAX_NAME_LENGTH);
	for (i=0; i<strlen(next_word); i++)
	{
		tstr[i] = toupper(next_word[i]); // У нас только 4 символа во встроенных словах.
	}
	for (i=0; i<e->words_root.n_words; i++){
		//printf("Comparing [%s] --- [%s]\n", (e->words_root.word_names_index)[i], tstr);
		//if (strncmp((e->words_root.word_names_index)[i], tstr, strlen(tstr)) == 0) {
		if (m_strcmp((e->words_root.word_names_index)[i], tstr) == 0) {
			//printf("known word:{%s}\n", (e->words_root.word_names_index)[i] );
			return (1);
		}
	}
	return 0;
}
/*
*	Известная в системе переменная. Имена переменных кейс сенситив!!!!
*/
int is_known_var(nicenv_t * e, char * next_word)
{
	int i;
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина слова не может быть больше 32
	_var_t * t_var;

	e->var = NULL;
	//memset(tstr, 0, MAX_NAME_LENGTH);
	t_var = e->vars_root.first;
	for (i=0; i<e->vars_root.n_vars; i++){
		//printf("Comparing [%s] --- [%s]\n", t_var->name, next_word);
		if (m_strcmp(t_var->name, next_word) == 0) {
			e->var =  t_var;
			return (i+1);
		}
		t_var = t_var->next;
	}
	return 0;
}
/*
*  Литералы это символы в одинарных кавычках или это целое число для помещения в 
* 	стек.
*/
int is_literal(nicenv_t * e, char * tok){
	//printf("Literal detected\n");
	if(tok[0]=='\\' && strlen(tok)==2 ) return 1;
	return 0;
}
int add_literal(nicenv_t * e, int lit){
	//printf("Adding literal\n");
	e->lint = lit;
	push_int_to_stack(e);
	e->T1[e->sp1-1] = LETR;
	return 0;
}

/*
* 	Функция проверяет, не содержит ли данный токен кавычки в начале и конце токена.
*/
int is_string(nicenv_t * e, char * tok)
{
	int st = 0;
	if (strlen(tok)>1){
		if (tok[0] == '"' && tok[strlen(tok)-1] == '"' && e->string_flag == 0){
			//У нас законченная строка, сохраняем ее
			st = 1;
			memset(e->string, 0, MAX_RAW_STRING);
			strncpy(e->string, tok+1, strlen(tok)-2);
			e->string_flag_ready = 1;
		} 
	} 
	return st; 
}


/*
* проверяем, есть ли слово EXIT, оно не может быть словом, так как слова ничего
* не возвращают, и парсинг продолжается.
*/
int is_exit(char * tok){
	char exit_str[]= "EXIT";
	int j=0, i=0;
	for (i=0; i<4; i++){
		if ( (toupper(tok[i]) == exit_str[i]) ) j++; 
	}
	if (j == 4) return 1;
	return 0;
}
/*
*	: , работает только для слов!
*/
int is_escape_code(nicenv_t * e, char * tok){
	char wn[MAX_NAME_LENGTH];
	memset (wn, 0, MAX_NAME_LENGTH);
	if (tok[0] == ':' && strlen(tok)>1 && !e->string_flag){
		strcpy(wn, tok+1);
		if (is_word(e, wn)){
			e->escape_flag = 1;
			process_word(e);
			return 1;
		}
		// проверяем является ли имя выполнимым текстом или строкой.
		// is_var
	   	_var_t * vn = is_var_name_exists(e, wn);
	    if (vn != NULL) {
	      	e->escape_flag = 1;	
	   		push_var_ptr_to_stack( e, vn);
			return 1;
		}
		// если имя неизвестно, то ничего не делаем, просто игнорируем токен.
		e->escape_flag = 0;	
		return 1;
	}
	return 0;
}

/* Обрататывает слово если оно тип*/
int is_type(nicenv_t * e, char * tok)
{
	_newtype_t * tp = is_type_exist(e, tok);
	if (tp != NULL){
		e->lint = tp->type_index;
		push_int_to_stack(e);
		e->T1[e->sp1-1] = TYPE;
		return 1;
	}
	return 0;
}

/* Возвращает токен, копирует его в tok, saveptr это место, которое указывает на начало следующего токена
	Разделители " \t\n". Признак строки - двойная кавычка. Признак литерала - одинарная кавычка.
*/
char * strtok_my(nicenv_t * e, char * s_str, char ** saveptr, char * tok)
{
	int i, j = 0;
	char * str;
	tok[0] = 0; // подготавливаем место.
	if (s_str != NULL) str = s_str;
	else str = *saveptr;

	memset (tok, 0, MAX_STRING);

	if (str != NULL && str[0] != 0){
		for (i=0; i<strlen(str); i++){
			switch (str[i]){
				case ' ':
				case '\t':
				{
					printf("e->string_flag = %d\n", e->string_flag);
					// Если это первый пробел, и символов еще небыло и это не строка, то мы его пропускаем
					if (strlen(tok) == 0 && e->string_flag == 0){
						// пропускаем пробел
						;;
					} 
					else if (strlen(tok) > 0 && e->string_flag == 0){
						// пробел является окончанием токена, опускаем его и сохраняем для следующего раза
						*saveptr = &(str[i]); 
						return tok; // токен уже закрыт нулем поэтому все в порядке.
					}else{ //e->string_flag == 1, строка открыта, поэтому копируем все пробелы до конца строки или кавычек.
						tok[j] = str[i];
						j++;
						tok[j] = 0;
						//printf("cond5(space)\n");
					}
					break;
				}
				case '"':
				{
					if (strlen(tok) != 0 && e->string_flag == 0){ 
						// встречаем кавычки впервые, строка еще не начиналась, но токен уже не пустой.
						// Значит завершаем токен и возвращаемся
						*saveptr = &(str[i]); // сохраняем указатель на кавычки
						//printf("cond1(_\"_)\n");
						return tok;
					} 
					else if (strlen(tok) != 0 && e->string_flag == 1){ 
						// встречаем кавычки и строка уже начиналась, токен уже не пустой.
						// Значит завершаем строку, копируем кавычку и возвращаемся
						e->string_flag = 0;
						tok[j] = str[i];
						j++;
						tok[j] = 0;
						*saveptr = &(str[i+1]);
						//printf("cond2(_\")\n");
						return tok;
					} 
					else if (strlen(tok) == 0 && e->string_flag == 0){
						// кавычки стоят первым символом, копируем строку вместе с кавычками до следующих 
						// кавычек или конца строки.
						e->string_flag = 1;
						tok[j] = str[i];
						j++;
						tok[j] = 0;
						//printf("cond3(\"_)\n");
					} else { //strlen(tok) != 0 , e->string_flag = 1; строка открыта, поэтому ее надо закрыть и сбросить флаг
						e->string_flag = 0;
						tok[j] = str[i];
						j++;
						tok[j] = 0;
						*saveptr = &(str[i+1]);
						//printf("cond4\n");
						return tok;
					}
					break;
				}
				case '\'':
				{
					if (strlen(tok) != 0 && e->string_flag == 0 && e->quote_flag == 0){ 
						// встречаем квотирование впервые, символ еще не начался, но токен уже не пустой.
						// Значит завершаем токен и возвращаемся
						*saveptr = &(str[i]); // сохраняем указатель на кавычки
						return tok;
					} 
					else if (strlen(tok) == 0 && e->string_flag == 0 && e->quote_flag == 0){
						// квотирование вне строки стоит первым символом, копируем строку вместе с квотой до следующей одинарной 
						// кавычки или до конца строки.
						e->quote_flag = 1;
						tok[j] = str[i];
						j++;
						tok[j] = 0;
					} 
					else if (e->string_flag == 0 && e->quote_flag == 1) {
						// квотирование вне строки, встретилось повторно, завершаем квотирование и возвращаем токен
						e->quote_flag = 0;
						tok[j] = str[i];
						j++;
						tok[j] = 0;
						*saveptr = &(str[i+1]);
						return tok;
					}
					else if (e->string_flag == 1){ // квотирование внутри строки, просто копируем как часть строки и продолжаем
						tok[j] = str[i];
						j++;
						tok[j] = 0;
					}
					break;
				}
				default:
				{
					// не разделитель, поэтому считаем, что 
					tok[j] = str[i];
					j++;
					tok[j] = 0; // закрываем на всякий случай токен.
					*saveptr = &(str[i+1]); // указатель на следующий символ
					if (i == strlen(str)-1){
						//*saveptr = NULL;
						return tok;
					}
				}
			}
		}
		return tok; //Что либо должно быть в токене
	}
	return NULL;
}

/*
* 	Обновляем все категории символов от текущей позиции до конца
*/
int update_symbol_category(nicenv_t * e, int pos, char * inp_buff, char* inp_cat){
	e->string_flag_pp = 0; // зануляем с начала строки
	for (int i=pos; i<strlen(inp_buff); i++){
		update_pos_symbol_category(e, i, inp_buff, inp_cat);
	}
	return 0;
}
/*
*	Эта функция просматривает все символы из препарсинга и если категория нулевая, то пытается найти
*	слово в 3 списках. А для фрагмента слова около курсора делается частичный поиск списка всех слов 
Fix: изменил порядок проверки символов
*/
int known_names_look_up(nicenv_t * e, char * inp_buff, char* inp_cat) {

	char name[MAX_NAME_LENGTH]; // найденое имя
	int i=0, j=0, k=0;
	memset(name, 0, MAX_NAME_LENGTH);
	for (i=0; i<strlen(inp_buff); i++){
		if (inp_cat[i] == SPACER_CAT) {
			//printf("i= %d, [%s] ", i, name );
			if ( 1 == is_known_word(e, name)){
				//printf("change category, i=%d\n", i);
				//меняем категорию на WORDS_CAT 
				for (k=0; k<j; k++) {
					if (i-k-1 >=0)
					inp_cat[i-k-1] = WORDS_CAT;
				}
				;;
			}
			j=0;
			continue;
		}
		if (inp_cat[i] == VARIABLE_CAT) {
			name[j] = inp_buff[i]; j++; name[j] = 0;
		}
	}
	return 0;
}



/* Можно использовать некоторые знаки пунктуации в именах переменных, или сложных типов, например массивов*/
int my_ispunct(char sp){
	char my_p [23] = { '!','$','%','&','(',')','*',',','.','/',';','<','=','>','?','@','[',']','^','{','|','}','~' };
	for(int i=0; i<23; i++) {
		if(sp == my_p[i]) return 1;
	}
	return 0;
}

/*
* 	Если символ стоит после __разделителя__ вне строки, определеяем категорию для этого символа
*/
int first_symbol_cat (nicenv_t * e, char sp){
	if ( isalpha(sp) || my_ispunct(sp) ) return VARIABLE_CAT;
	if ( '+' == sp || '-' == sp ) return NUMBER_SIGN_CAT;
	if ( ' ' == sp || '\t' == sp ) return SPACER_CAT;
	if ( '"' == sp ) { e->string_flag_pp = 1; return  DQUOTE_CAT; } // строка открыта перенос не работает когда открыта строка.
	if ( isdigit(sp) ) { return NUMBER_CAT; }
	if ( ':' == sp ) {return ESCAPE_CAT; }
	if ( '#' == sp ) {return COMMENT_CAT; }
	if ( '\\' == sp ) {return S_ESCAPE_CAT; } // этот символ стоит перед литералом после квотирования может быть любой символ
	return ERROR_CAT;
}
/*
* Мы в начале строки, строка начата, определяем категории символов в начале строки
*/
int string_symbol_cat (nicenv_t * e, char sp){
	if ( '"' == sp ) { /*printf("\n\n----DQUOTE_CAT----\n"); fflush(stdout);*/ e->string_flag_pp = 0; return DQUOTE_CAT; }
	if ( '\\' == sp ) { /*printf("\n\n----S_ESC_CAT----\n"); fflush(stdout);*/ return S_ESCAPE_CAT; } // эскейп последовательность
	/*printf("\n\n----STRING_CAT----\n");
	fflush (stdout);*/
	return STRING_CAT; // если не это, то все остальное просто элемент строки
}

/*
*	Если удаляется какой-то символ, который является критическим для разбора синтаксиса,
*	например " или ' это признаки которые могут перносится на другую строку
*/
int delete_symbol_category(nicenv_t * e, int pos, char * inp_buff, char* inp_cat){
	return 0;
}

/**
*	После добавления символа в некой позиции, необходимо обновить раскраску строки
*	на базе категории символов. Для минимизации затрат, будем стараться принять решение на
* 	как можно меньшем количестве символов
категории
1-	1) # BLUE_HIGH - критерий для нового символа предыдущий является комментарием
2-	2) " - предыдущий является строкой, если не закрывающей кавычкой, учитывается флаг открытой строки GREEN
3-	3) \X , :, Литералы  - GREEN_HIGH
4-	4) words - YELLOW
5-	5) number	MAGENTA
6-	6) type 	CYAN - пока не буду различать словами
0-	7) var 		WHITE_HIGH
7-	8) bad_symbol - любой недопустимый символ RED_HIGH
*/
int update_pos_symbol_category(nicenv_t * e, int pos, char * inp_buff, char* inp_cat){
	// Предполагаем, что мы вызываем эту функцию, если было сделано изменение символа в позиции
	// pos. Обновляем категорию только для одного текущего символа, поскольку мы будем вызывать эту 
	// функцию в цикле.
	int i, dot_flag;
	char sp = 0; // категории предыдущих символов
	char spp = 0;
	char sppp = 0;

	if (pos == 0){ //первый символ
		inp_cat[pos] = first_symbol_cat (e, inp_buff[pos]); return 0;
		
	}
	// Определяем категорию предыдущего символа
	if (pos-1 >=0 ) sp = inp_cat[pos-1];
	if (sp == ERROR_CAT) { inp_cat[pos] = ERROR_CAT;  return 0; } // тут все понятно
	if (sp == COMMENT_CAT) { // если у нас коментарий, то все до конца - комментарий	
		inp_cat[pos] = COMMENT_CAT; return 0;
	} 
	if (sp == STRING_CAT) { // Предыдуший символ принадлежит строке
		// проверяем, не закрывает ли текущий символ строку?
		if (inp_buff[pos] == '"') { inp_cat[pos] = DQUOTE_CAT; 
			if (e->string_flag_pp == 1) { e->string_flag_pp = 0; return 0; }
			else { e->string_flag_pp = 1; return 0; }
		}
		else {
			if (inp_buff[pos] == '\\') { inp_cat[pos] = S_ESCAPE_CAT; return 0;} 
			inp_cat[pos] = STRING_CAT; return 0; /*иначе просто продолжение строки*/
		}
	}
	if (sp == DQUOTE_CAT) { // двойные кавычки не в первой позиции. Строка открыта, любой символ должен быть 

		//printf("\n\n DQUOTE_CAT\n\n" );
		if (e->string_flag_pp == 1) { /*началась срока*/
			inp_cat[pos] = string_symbol_cat (e, inp_buff[pos]);
			return 0;
		}
		// строка закончилась на предыдущем символе, тут может быть только пробел. После закрывающих кавычек может быть
		// только пробел или табуляция
		if (inp_buff[pos] == ' ' || inp_buff[pos] == '\t') { inp_cat[pos] = SPACER_CAT; return 0; }
	}
	if (sp == S_ESCAPE_CAT){ inp_cat[pos] = E_ESCAPE_CAT; return 0; } // второй символ эскейп последовательности, или литерал
	if (sp == E_ESCAPE_CAT) {
		if (1 == e->string_flag_pp){ // ", \, Литерал или строка
			if ( '"' == inp_buff[pos] ) { inp_cat[pos] = DQUOTE_CAT; e->string_flag_pp = 0; return 0; }
			if ( '\\' == inp_buff[pos] ) { inp_cat[pos] = S_ESCAPE_CAT; return 0;}
			/*иначе просто символ строки*/
			inp_cat[pos] = STRING_CAT;
			return 0;
		}
		// иначе может быть только пробел
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	if ( sp == SPACER_CAT && pos > 0 ) { /*после разделителя не в первой позиции*/
		if (e->string_flag_pp == 0) {
			inp_cat[pos] = first_symbol_cat (e, inp_buff[pos]); return 0;
		}
	}
	/*Numbers*/
	if ( sp == NUMBER_CAT) {// может быть либо число, либо точка, либо показатель экспоненты, либо разделитель
		if ( isdigit(inp_buff[pos]) ) { inp_cat[pos] =  NUMBER_CAT; return 0;}
		if ( '.' == inp_buff[pos]) { // проверяем нет ли других точек в этом числе до начала числа
			dot_flag = 0;
			for (i = pos-1; i>=0; i--){
				if (inp_cat[i] == NUMBER_SIGN_CAT || inp_cat[i] == SPACER_CAT) break;
				if (inp_cat[i] == NUMBER_DOT_CAT ) dot_flag = 1;
			}
			if (dot_flag == 0) {inp_cat[pos] = NUMBER_DOT_CAT; return 0;} // только одна точка в числе.
		}
		if ( 'E' == inp_buff[pos] || 'e' == inp_buff[pos]) { // проверяем нет ли других экспонент в этом числе до начала числа
			dot_flag = 0;
			for (i = pos-1; i>=0; i--){
				if (inp_cat[i] == NUMBER_SIGN_CAT || inp_cat[i] == SPACER_CAT) break;
				if (inp_cat[i] == NUMBER_POWER_CAT ) dot_flag = 1;
			}
			if (dot_flag == 0) {inp_cat[pos] = NUMBER_POWER_CAT; return 0; }// только одна Экспонента в числе. 
		}
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	if (sp == NUMBER_POWER_CAT){ /* Тут может стоять +/- */
		if ( '+' == inp_buff[pos] || '-' == inp_buff[pos] ) {inp_cat[pos] = NUMBER_POWER_SIGN_CAT; return 0;}
	}
	if (sp == NUMBER_POWER_SIGN_CAT) {
		if ( isdigit(inp_buff[pos]) ) { inp_cat[pos] = NUMBER_POWER_NUM_DIG_CAT; return 0;}
	}
	if (sp == NUMBER_POWER_NUM_DIG_CAT) { // может быть либо цифра либо разделитель, по другому ошибка
		if ( isdigit(inp_buff[pos]) ) { inp_cat[pos] = NUMBER_POWER_NUM_DIG_CAT; return 0;}
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	if (sp == NUMBER_SIGN_CAT) {
		if ( isdigit(inp_buff[pos]) ) { inp_cat[pos] = NUMBER_CAT; return 0;}
	}
	if (sp == NUMBER_DOT_CAT){
		if ( isdigit(inp_buff[pos]) ) { inp_cat[pos] = NUMBER_CAT; return 0;}
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	/*end of numbers*/
	if (sp == ESCAPE_CAT) {// может стоять только буква
		if ( isalpha(inp_buff[pos]) || my_ispunct(inp_buff[pos]) ) { inp_cat[pos] = VARIABLE_CAT; return 0;}
	}
	if (sp == VARIABLE_CAT) {// может быть или буква или цифра или _ или разделитель
		if ( isalpha(inp_buff[pos]) || isdigit(inp_buff[pos]) || my_ispunct(inp_buff[pos]) || '_' == inp_buff[pos]) 
			{ inp_cat[pos] = VARIABLE_CAT; return 0;}
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	if (sp == WORDS_CAT) {// может быть или буква или цифра или _ или разделитель
		if ( isalpha(inp_buff[pos]) || isdigit(inp_buff[pos]) || '_' == inp_buff[pos]) 
			{ inp_cat[pos] = WORDS_CAT; return 0;}
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	if (sp == TYPE_CAT) {// может быть или буква или цифра или _ или разделитель
		if ( isalpha(inp_buff[pos]) || isdigit(inp_buff[pos]) || '_' == inp_buff[pos]) 
			{ inp_cat[pos] = TYPE_CAT; return 0;}
		if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
	}
	/*
	if ( sp == QUOTE_CAT ) {// может быть 1 симол или эскейп последовательность
		if (e->quote_flag == 0) { // квотирование завершено, здесь может быть только пробел
			if (' ' == inp_buff[pos] || '\t' == inp_buff[pos]) {inp_cat[pos] = SPACER_CAT; return 0; }
		}else{ // здесь может быть либо литерал, либо эскейп последовательность
			if ('\\' == inp_buff[pos]) {inp_cat[pos] = S_ESCAPE_CAT; return 0; }
			else { inp_cat[pos] = LITERAL_CAT; return 0; } // все остально расценивается как литерал
		}
	}
	if ( sp == LITERAL_CAT) { // после литерала может быть только закрывающая кавычка одинарная
		if ( '\'' == inp_buff[pos] ) { inp_cat[pos] = QUOTE_CAT; e->quote_flag = 0; return 0; }	
	}*/
	//....

	inp_cat[pos] = ERROR_CAT; 
	return 0;
}


/* 	Возвращает строку из символов одной категории.
* 	останавливается на символе другой категории и возвращает позицию следующей категории.
* 	Если строка закончилась, то индекс позиции становится равен -1. и категория тогда тоже = -1
*/
int get_next_category_string(nicenv_t * e, int pos, char * inp_buff, char * inp_cat, char * outline, int * cat_id){
	int next_pos = pos;
	int i, j = 0;
	memset (outline, 0, MAX_INPL_SIZE);
	if (pos < strlen(inp_buff)) *cat_id = inp_cat[pos];
	else {
		next_pos = -1; *cat_id = -1;
		return next_pos;
	}
	for (i = pos; i<strlen(inp_buff); i++ ){
		if (inp_cat[i] == inp_cat[pos]){
			// копируем символ
			outline[j++] = inp_buff[i]; outline[j] = 0;
		} else {
			next_pos = i; // следующий раз продожим отсюда
			return next_pos;
		}
	}
	return -1;
}
/*
*	Исправляем категории символов, которые находятся в строке.
*	все что внутри строки с кавычками -> STRING_CAT
*/
int fix_string_category(nicenv_t * e, char * inp_buff, char * inp_cat){
	int i;
	char string_flag = 0;
	for (i = 0; i<strlen(inp_buff); i++ ){
		if (inp_cat[i] == DQUOTE_CAT && string_flag == 0){
			// нашли начало строки.
			string_flag = 1;
			inp_cat[i] = STRING_CAT;
			//printf("Begin of the string\n");
		} 
		if (string_flag == 1 && inp_cat[i] != DQUOTE_CAT){
			inp_cat[i] = STRING_CAT;
			//printf(".");
		}
		if (string_flag == 1 && inp_cat[i] == DQUOTE_CAT){
			string_flag = 0;
			inp_cat[i] = STRING_CAT;
			//printf("End of the string\n");
		}
	}
	return 0;
}
/*
*	Исправляем эскейп последовательности.
*	все что после : -> ESCAPE_CAT
*/
int fix_escape_category(nicenv_t * e, char * inp_buff, char * inp_cat){
	int i;
	char escape_flag = 0;
	for (i = 0; i<strlen(inp_buff); i++ ){
		if (inp_cat[i] == ESCAPE_CAT && escape_flag == 0){
			escape_flag = 1;
			inp_cat[i] = ESCAPE_CAT;
			//printf("Begin of esc\n");
		} 
		if (escape_flag == 1 && inp_cat[i] != SPACER_CAT){
			inp_cat[i] = ESCAPE_CAT;
			//printf(".");
		}
		if (escape_flag == 1 && inp_cat[i] == SPACER_CAT){
			escape_flag = 0;
			//printf("End of escape\n");
		}
	}
	return 0;
}
/*
*	Исправляем literal последовательности.
*	все что после : -> ESCAPE_CAT
*/
int fix_literal_category(nicenv_t * e, char * inp_buff, char * inp_cat){
	int i;
	char lit_flag = 0;
	for (i = 0; i<strlen(inp_buff); i++ ){
		if (inp_cat[i] == S_ESCAPE_CAT || inp_cat[i] == E_ESCAPE_CAT){
			inp_cat[i] = LITERAL_CAT;
		} 
	}
	return 0;
}
/*
*	Исправляем numbers 
..................................... 
fixit
*/
int fix_number_category(nicenv_t * e, char * inp_buff, char * inp_cat){
	int i;
	for (i = 0; i<strlen(inp_buff); i++ ){
		if (inp_cat[i] == NUMBER_SIGN_CAT || inp_cat[i] == NUMBER_DOT_CAT ||
			inp_cat[i] == NUMBER_POWER_CAT || inp_cat[i] == NUMBER_POWER_SIGN_CAT ||
			inp_cat[i] == NUMBER_POWER_NUM_DIG_CAT )
		{
			inp_cat[i] = NUMBER_CAT;
		} 
	}
	return 0;
}


/*
*	Найти смещение ошибки
*/
int get_error_offset(nicenv_t * e, char * inp_buff, char * inp_cat){
	int i;
	for (i = 0; i<strlen(inp_buff); i++ ){
		if (inp_cat[i] == ERROR_CAT){
			return i;
		} 
	}
	return -1;
}


/*
* 	Функция интерпретирует строку на базе информации, полученной из препарсинга строки.
* 	Если строка вводилась вручную, то препарсинг содержится в e->preparse_line[]
* 	Если же эта строка пустая, то ввод осуществляется из файла и препарсинга
*/
int parse_interpret_line_preparse(nicenv_t * e, char * next_word){
	char *tok = NULL;
	char str[MAX_STRING]; //вообще-то достаточно было MAX_INPL
	char token[MAX_STRING];
	_var_t * vn = NULL;
	char tline[MAX_STRING];
	char * saveptr;
	int ret = 0;
	int new_pos = 0;
	int cat_id = 0;

	char tvarn[MAX_NAME_LENGTH]; // Это предупреждение в каком месте в строке ошика
	memset(str, 0, MAX_STRING);
	strcpy(str, (char *)(((_var_t **)e->input->first_element)[0])->first_element);

#if (DEBUG_LEVEL == 5)	
	printf("String to interpret: {%s}\n", str);
#endif	
	if (strlen(e->preparse_line) == 0){
		update_symbol_category(e, 0, str, e->preparse_line);
#if (DEBUG_LEVEL == 5)	
		printf("preparse_line = ");
		for(int it=0; it<strlen(str); it++) printf("%d ", e->preparse_line[it] );
		printf("\n");
#endif		
		known_names_look_up(e, str, e->preparse_line);
#if (DEBUG_LEVEL == 5)	
		printf("preparse_line after name lookup = ");
		for(int it=0; it<strlen(str); it++) printf("%d ", e->preparse_line[it] );
		printf("\n");
#endif		
	}
	// исправляем категории строк для правильной интерпретации
	fix_string_category(e, str, e->preparse_line);
	fix_escape_category(e, str, e->preparse_line);
	fix_literal_category(e, str, e->preparse_line);
	fix_number_category(e, str, e->preparse_line);
	new_pos = 0;
	while( new_pos >= 0 ){
		tline[0] = 0;
		new_pos = get_next_category_string( e, new_pos, str, e->preparse_line, tline, &cat_id);
#if (DEBUG_LEVEL == 5)	
		printf("Cat[%d],<%s>\n", cat_id, tline );
#endif
		tok = tline;

		if (cat_id == COMMENT_CAT) {
			return END_OF_PARSE;
		}
		if (cat_id == ERROR_CAT) {
			new_pos = get_error_offset(e, str, e->preparse_line);
			printf("Stop interpret at pos %d\n", new_pos );
			printf("%s\n", str);
			for (int i=0; i<new_pos; i++) printf("-");
			printf("^\n");
			break;
		}
		if (cat_id != SPACER_CAT)
		{
			if (is_string(e, tok)) {
	   			if (e->string_flag_ready == 1){
	   				add_string(e);
	   			}
	   			goto next_tokk;
	   		}
	   		if (is_escape_code(e, tok)){
   			goto next_tokk;
	   		} 
	   		if (is_exit(tok)){
	   			return EXIT;
	   		}
	   		if (is_literal(e,tok)){
	   			add_literal(e,tok[1]);
	   			goto next_tokk;
	   		}
	   		if (is_number(e, tok) > 0){
	   			add_number(e);
	   			goto next_tokk;	
	   		}
	   		if (is_type(e, tok) > 0){
	   			goto next_tokk;
	   		} 
	   		if (is_word(e, tok) > 0){
	   			process_word(e);
	   			goto next_tokk;	
	   		}
	   		// is_var
	   		vn = is_var_name_exists(e, tok);
	      	if (vn == NULL) {
	      		vn = new_var(e, tok, UDEF);
	      	} 
	   		//грузим переменную с таким именем в стек
	   		ret = push_var_ptr_to_stack( e, vn);
	   		if (ret < 0) break;

		}
next_tokk:	;;
	}
	return END_OF_PARSE;
}


/*
*	Компилирует библиотеку из исходников. Это нужно будет делать каждый раз, когда
* 	создается новое слово какого-то уровня. Пока я буду просто перебилдовывать всю
* 	библиотеку, но потом сделаю добавление только одного слова.
*/
int compile (int level, char * word_name)
{
	char command[80];

	//printf("Compiling words Level %d...\n", level);
	sprintf (command, "gcc -c %s.c -o %s.o -fPIC -lm", word_name, word_name);   
	system(command);
	memset (command, 0, 80);

	sprintf (command, "gcc -c %s.c -o %s.o -fPIC -lm", "sh_inner", "sh_inner");   
	system(command);

	memset (command, 0, 80);

	sprintf (command, "gcc -shared -o lib%s.so %s.o %s.o -lm", word_name, word_name, "sh_inner");   

	// system call
	system(command);

	memset (command, 0, 80);
	sprintf (command, "strip lib%s.so", word_name);

	system(command);
	return 0;
}

/*
*	Динамическая загрузка библиотеки. Пока только 0 уровня.
*/
int load_words(nicenv_t * e, int level)
{
	// указатель на библиотеку.
	if (e->words_root.ext_library == NULL) 
	{
		e->words_root.ext_library = dlopen("./libwordl0.so", RTLD_LAZY);
#if (DEBUG_LEVEL == 4)		
		printf("Library level %d loaded at %p\n", level, e->words_root.ext_library);
#endif		
		if (!(e->words_root.ext_library)){
		
			fprintf(stderr,"dlopen() error: %s\n", dlerror());
			return 1;
		};
	}
	return 0;
}

int unload_words(nicenv_t * e, int level){
#if (DEBUG_LEVEL == 4)	
	printf("Unloading Library\n");
#endif	
	if (e->words_root.ext_library != NULL) dlclose(e->words_root.ext_library);
}

/*
* 	Добавляем слово в кольцо.
*/
int add_word_to_ring(nicenv_t * e, int level, char * tmpStr)
{
	_words_ring_t * wroot = &(e->words_root);
	_word_t * first = wroot->first; // указатель на первое слово
	_word_t * new_w = NULL;
	int (*code) (struct nicenv *);
	char code_name[MAX_NAME_LENGTH];

	// Проверяем уровень кольца. Работает пока только с 0 уровнем.
	if( level == 0)	{

		// 	Создаем новое слово
		new_w = (_word_t *) malloc(sizeof(_word_t));
		if (new_w == NULL){
			printf("Word strure allocation error!");
			exit(1);
		}

		new_w->next = NULL;
		new_w->prev = NULL;
		new_w->level = 0;
		memset(new_w->name, 0, MAX_NAME_LENGTH);
		strncpy(new_w->name, tmpStr, strlen(tmpStr));
		new_w->code = NULL; // Изначально ссылки на код нету.
		// Добавляем префикс к имени.
		sprintf(tmpStr,"wl%d_%s", level,new_w->name);

		if (wroot->ext_library != NULL){
			code = dlsym(wroot->ext_library, tmpStr);
			if (code != NULL) new_w->code = code;
			else
			{
				printf("Can't dynamically link the name!\n");
			}
		}else
		{
			printf("No external library loaded for this word level!");
		}

		if (first == NULL){
			// Кольцо пустое, нужно создать слово.
			e->words_root.first = new_w;
			
			// Записываем количество слов в системе.
			wroot->n_words = 1;
		}
		else {
			// Кольцо не пустое, нужно добавить слово в конец кольца.
			// Возможны 2 варианта: 1 слово и больше слов.
			if (first->prev == NULL && first->next == NULL) {
				// Означает, что количество слов == 1
				first->next = new_w;
				first->prev = new_w;
				new_w->next = first;
				new_w->prev = first;
			} else {
				// Количество загруженных слов больше 1.
				new_w->prev = first->prev;
				new_w->prev->next = new_w;
				first->prev = new_w;
				new_w->next = first;
			}
			wroot->n_words ++;
		}
	}
	return 0;
}


/*
*	Функция парсит исходинк скомпилированного файла и вынимает из него названия функций	
*/
int add_words_to_list(nicenv_t * e, int level)
{
    FILE* words_fp;
    char line[MAX_COUNT];
    char tmpStr[MAX_COUNT];
    char* token ;
    int i , j, k, l;
    char filename[MAX_STRING]; // Обычно wordl0.c, максимум wordlXXXX.c, где X цифра.
    char func_start[15];
    _word_t * tmp_word;
        
    
    memset(func_start,0,15);
#if (DEBUG_LEVEL == 4)
    printf ("Parsing words level %d\n", level);
#endif    
    sprintf(filename,"wordl%d.h", level); // Читаем только заголовок файла,
    									// В нем должны быть те же имена, что и в .с
    sprintf(func_start, "int wl%d_", level);

    words_fp = fopen( filename , "r" ) ;
    if (words_fp == NULL) 
    {
    	perror("Open word header is failed");
    	exit(1);
    }
    
    while( fgets( line, MAX_COUNT, words_fp ) != NULL )
    {
    	//printf("<%s>\n", line);
    	// Все функции имеют тип int, и имеют сигнатуру
    	// int wl0_NEL(nicenv_t * e)
    	// И при этом начинаться с начала строки.
    	l = strlen(func_start);
    	//printf("[%s]\n", func_start);
    	if (strncmp (line, func_start, l) == 0)
    	{

	    	for( i = l; i<strlen(line); i++)
	    	{
	    		if (line[i] == '(' ) // Скобка должна быть на той же строке. Иначе имя
	    			// не будет найдено. Это не баг, просто фича.
	    		{
	    			memset (tmpStr, 0, MAX_COUNT);
	    			strncpy(tmpStr, line+l, i-l);
	    			//printf("Found word: [%s]\n", tmpStr);
	    			memset (line, 0, MAX_COUNT);
	    			//printf("Adding word: [%s] to level %d\n", tmpStr, level);
	    			add_word_to_ring(e, level, tmpStr);

	    			continue;
	    		}

	    	}
	    }
    }
    // Сейчас все слова загружены, составляем индекс имен загруженных функций.
    if (e->words_root.word_names_index != NULL) {
    	free (e->words_root.word_names_index);
    } 
    // выделяем массив для хранения индекса имен
#if (DEBUG_LEVEL == 4)
    printf("Allocate index of %d names\n", e->words_root.n_words);
#endif    
    e->words_root.word_names_index = (char **) malloc ((e->words_root.n_words)*sizeof(char*));

    tmp_word = e->words_root.first;
    for (i = 0; i<e->words_root.n_words; i++){
    	(e->words_root.word_names_index)[i] = tmp_word->name;
    	tmp_word = tmp_word->next;
    }
#if (DEBUG_LEVEL == 4)
    printf("List of names: ");
    for (i = 0; i<e->words_root.n_words; i++){
    	printf("%s ", e->words_root.word_names_index[i]);
    }
    printf("\n");
#endif    
    fclose(words_fp);
    return 0;
}
/*
* 	Помещаем целое число в стек.
*/
int push_int_to_stack(nicenv_t * e)
{
	// Проверяем вершиину стека
	if (e->sp1 < MAX_STACK_SIZE){
		e->S1[e->sp1] = (void*) e->lint; 
		e->T1[e->sp1] = INT;
		e->osp1 = e->sp1;
		e->sp1++;	
	} else {
		printf("Warning: Stack is full, operation skipped.\n");
	}
	return 0;
}
/*
*	Добавляем число типа double в память, на самом деле мы создаем переменную 
* 	и помещаем значение туда. А в стек помещаем указатель на ту переменную.
*/
int push_dbl_to_stack(nicenv_t *e){
	long d[2];
	// Проверяем вершиину стека
	if (e->sp1 < MAX_STACK_SIZE-1){ // на помещение double в стек нужно 2 ячейки.
		
		*((double *)d) = e->dbl;
		//printf("DUBL:  %x   %x\n", d[0], d[1]);

		e->osp1 = e->sp1;
		
		e->S1[e->sp1] = (void*) d[0]; 
		e->T1[e->sp1] = DUBL;
		e->sp1++;	
		e->S1[e->sp1] = (void*) d[1]; 
		e->T1[e->sp1] = DUBL;
		e->sp1++;	

	} else {
		printf("Warning: Stack is full, operation skipped.\n");
	}
	return 0;	
}

/*
*	Функция парсит только 1 шаг все время, то есть строку line0
*/
int parse_line(nicenv_t * e)
{
	int i, j;
	int r = NEW_SYMBOL;
	
	if (e->next_word == NULL){
		//выделяем память для парсинга слова. если она еще не выделена
		e->next_word = (char*) malloc(MAX_STRING * sizeof(char));
	}
	if (e->string == NULL){
		//выделяем память для парсинга строки. если она еще не выделена
		e->string = (char*) malloc(MAX_RAW_STRING * sizeof(char));
	}

	
	r = parse_interpret_line_preparse(e, e->next_word);
		
	e->next_word[0] = 0;
	if (r != BAD_SYMBOL){
		//Двигаем указатель выполненных слов
		e->line_to_parse = 0;
		e->col_commited = e->col_pointer; // вообще говоря, если был сбойный символ, то строка
		// должна сброситься.
	}
	return r;
}
/*
	После выполнения слова repe в памяти среды должны быть объекты, которые должны крутиться в цикле
	перодически, пока условие не выдаст 0.
	Фидер должен подкиыдвать строки компилытору, пока в стеке не появится нуль.
	Надо подумать как сделать вложенные циклы.
*/
int repe_line_feeder(nicenv_t * e){
	//......

	///...
	return 0;
}

int run_word(nicenv_t * e){
	_word_t * w = NULL;
	_var_t * s = NULL;
	_var_t * inp = NULL;
	if ( (e->sp1 > 0) && (e->T1[e->sp1-1] == CODE) ){
		w = (_word_t *)  e->S1[e->sp1-1];
#if (DEBUG_LEVEL == 5)			
		printf("Executing the word: %s\n", w->name);
#endif		
		e->T1[e->sp1-1] = 0;
		e->S1[e->sp1-1] = 0;
		e->sp1--;
		
		(*(w->code))(e);
		return 0;
	}
	// Интерпретация строки или текста
	if ( (e->sp1 > 0) && ( (e->T1[e->sp1-1] == STRI) || (e->T1[e->sp1-1] == TEXT) ) ){
		s = (_var_t *)  e->S1[e->sp1-1];
		e->T1[e->sp1-1] = 0;
		e->S1[e->sp1-1] = 0;
		e->sp1--;
		// добавляем строку в спулер
		if (e->run_cnt < MAX_RUNS) {
			e->run_body[e->run_cnt] = s;
			e->run_line[e->run_cnt] = 0;
			e->run_cnt ++;
			if (e->nest_cnt < MAX_NEST) {
				e->nest[e->nest_cnt] = 'R';
				e->nest_cnt++;
			}
		}
	}
	return 0;
}


/*
* 	добавляем все строки из переменной v в текстовую переменную inpl. Переменная может быть как строкой так и текстом.
*/
int spool_code_to_inpl( nicenv_t * e, _var_t * v){
	_var_t * inp = is_var_name_exists(e, "inpl");
	if (inp == NULL) return -1;
	if (v == NULL) return -1;
	// Интерпретация строки и текста
	if ( v->type.type_index == STRI ){
		add_string_to_text(e, inp, v, 1); 
	}
	if ( v->type.type_index == TEXT ){
		append_text_to_text ( e, inp, v, 1);
	}
	return 0;
}

/*
*  Проверяем содержимое вершины стека, если оно == 0 то
*/
int check_condition(nicenv_t * e ){
	if (e->sp1 >0 && e->T1[e->sp1-1] == INT){
		e->lint = (long) e->S1[e->sp1-1];
		e->sp1--;
		if (e->lint == 0) return 0; // ложное значение, на вершине стека лежит 0. Значит цикл пора завершать
	}
	return 1; // истиное значение, продолжаем цикл
}

/*
*	Добавляем переменную в стек
*/
int push_var_ptr_to_stack(nicenv_t * e, _var_t * v_n){
	if(v_n == NULL) {
		printf("Warning: NULL variable, operation skipped.\n");
		return 0;
	}
	if(e->sp1 < MAX_STACK_SIZE-1){
		// проверяем, не является ли переменная линком
		if (v_n->link) {
			// проверяем существует ли переменная на которую ссылается линк
			_var_t * vl = is_var_name_exists(e, (char*) v_n->first_element);
			//printf("link to -> %s\n", (char*) v_n->first_element );
			//printf("VAR %s = %p , %p\n", (char*) vl->name, vl, v_n );

			if (vl == NULL){
				//может быть это линк на слово?
				if (is_word(e,(char*) v_n->first_element)){
					if (e->escape_flag == 0){
						process_word(e);
						return 0;
					}else { // грузим линк в память
						e->S1[e->sp1] = (void*) v_n; 
						e->T1[e->sp1] = v_n->type.type_index;
						e->osp1 = e->sp1;
						e->sp1++;
						e->escape_flag = 0; // сбрасываем последовательность
						return 0;
					}
				}
				//printf("link refers to NULL\n");
				e->S1[e->sp1] = NULL; 
				e->T1[e->sp1] = VOID;
				e->osp1 = e->sp1;
				e->sp1++;
				printf("Warning: Link to not existing variable.\n");
				return 0;
			} else
			if ((vl != NULL) && (e->escape_flag == 0 )){
				//printf("link refers to variable\n");
				v_n = vl; // если нет эскейпа то подставляем ту переменную на которую ссылаемся, иначе грузим сам линк
			}else{ // сбрасывем эскейп флаг чтобы
				//printf("reset escape\n");
				e->escape_flag = 0;
			}
		}
		e->S1[e->sp1] = (void*) v_n; 
		e->T1[e->sp1] = v_n->type.type_index;
		e->osp1 = e->sp1;
		e->sp1++;
		// если переменная имеет флаг выполнимой, то выполяем ее сразу
		if ( (v_n->exec == 1) && (v_n->type.type_index == TEXT || v_n->type.type_index == STRI) ){
			if (e->escape_flag == 1	){ // не выполняем переменную если оно стоит после эскейп последовательности
				e->escape_flag = 0;
			}else {
				run_word(e);
			}
		}
		e->escape_flag = 0; // на всякий случай
	} else {
		printf("Warning: Stack is full, operation skipped.\n");
		return -1;
	}
	return 0;
}

int push_code_to_stack(nicenv_t * e)
{
	if( (e->word->code) != NULL && e->sp1 < MAX_STACK_SIZE){
		e->S1[e->sp1] = (void*) e->word; 
		e->T1[e->sp1] = CODE;
		e->osp1 = e->sp1;
		e->sp1++;	
	} else {
		printf("Warning: Stack is full, operation skipped.\n");
	}
	return 0;
}

/*
* 	Используем хэш для создания имени строки.
*/
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

int generate_unique_name (nicenv_t * e, char * prename, char * name){
	long rnd;
	_var_t * vn = NULL;
	do {
		rnd = rand();
		memset(name, 0, MAX_NAME_LENGTH);
		//prename[2] = 0; // ограничим пренейм 2 символами
		sprintf(name,"%s%ld", prename,rnd);
		name[8] = 0; // trim the hash name to some symbols
		vn = is_var_name_exists(e, name);
	} while (vn != NULL) ;
	return 0;
}
int add_string(nicenv_t * e)
{
	unsigned long h;
	long rnd;
	unsigned char * rnds;
	char str_name[MAX_NAME_LENGTH];
	_var_t * v_n = NULL;
	int i;
#if (DEBUG_LEVEL == 5)
	printf("Adding string: ");
	printf(ANSI_COLOR_YELLOW "<%s>" ANSI_COLOR_RESET "\n", e->string);
#endif	
	generate_unique_name (e, "s", str_name);
	
	v_n = new_var(e, str_name, STRI);
	set_string(v_n, e->string);
	push_var_ptr_to_stack(e, v_n);
	memset(e->string, 0, MAX_RAW_STRING);

	e->escape_flag = 0; // Сбрасываем флаг и игнорируем его
	e->string_flag_ready = 0;
	e->shell_mode = INTERPRETER_MODE;

	return 0;
}

int add_number(nicenv_t * e){
	//printf("Adding number\n");
	// Помещаем число в стек. Число может быть целого типа, вещественного.
	// По умолчанию выбирается самый длинный тип числа.
	if (e->parse_param0 == DUBL) {
#if (DEBUG_LEVEL == 5)		
		printf("Double number %e\n", e->dbl);
#endif		
		push_dbl_to_stack(e);
	}
	if (e->parse_param0 == INT) {
#if (DEBUG_LEVEL == 5)		
		printf("Int number %ld\n", e->lint);
#endif		
		push_int_to_stack(e);
	}

	e->escape_flag = 0; // Сбрасываем флаг игнорируем его

	return 0;
}

int process_word(nicenv_t * e){
	int r;
	_word_t * w = e->word;
#if (DEBUG_LEVEL == 5)
	printf("Process word\n");
	// Находим ссылку на слово по имени. Имя слова должно было остаться в 
	printf("Executing word %s\n", e->word->name);
#endif	
	if (e->word->code != NULL)
	{
		if (e->escape_flag == 0){
			r = (*(e->word->code))(e);
		} else
		{
			push_code_to_stack(e);
		}
	} else
	{
	 	printf("Null pointer to the code.\n");
	}
	e->escape_flag = 0; // Сбрасываем флаг
	return 0;
}
/* Создаем переменную и выделяем новое место для данных, то есть это полная копия данных, но имя другое */
int fork_var(_var_t * v1, _var_t * v2)
{
	if (v2->first_element != NULL) free(v2->first_element);
	v2->first_element = (char *) malloc (v1->size);
	memcpy((char*) v2->first_element, (char*) v1->first_element, v1->size);
	v2->num_element = v1->num_element;
	v2->size = v1->size;
	memcpy(v2->data, v1->data, 8);
	strncpy (v2->type.name, v1->type.name, MAX_NAME_LENGTH);
	v2->type.type_index = v1->type.type_index;
	v2->type.level = v1->type.level;
	v2->type.next = v1->type.next;
	return 0;
}

/*Копируем переменную, они указывают на одну и ту же область памяти, но имена у них разные*/
int copy_var(_var_t * v1, _var_t * v2)
{
	if (v2->first_element != NULL) free(v2->first_element);
	v2->first_element = v1->first_element;
	v2->num_element = v1->num_element;
	v2->size = v1->size;
	memcpy(v2->data, v1->data, 8);
	strncpy (v2->type.name, v1->type.name, MAX_NAME_LENGTH);
	v2->type.type_index = v1->type.type_index;
	v2->type.level = v1->type.level;
	v2->type.next = v1->type.next;
	return 0;
}

int set_num_to_var(_var_t * v1, long a)
{
	long * lp;
	if (v1->first_element != NULL) free(v1->first_element);
	v1->first_element = NULL;
	v1->num_element = 1;
	lp = (long *) &(v1->data[0]);
	*lp = a;
	return 0;
}

long get_num_from_var(_var_t * v1)
{
	long * lp;
	lp = (long *) v1->data;
	return *lp;
}


int set_var_type(_var_t * v1, long t)
{
	v1->type.type_index = t;
	return 0;
}

int set_dbl_to_var(_var_t * v1, double a){
	double * dp;
	if (v1->first_element != NULL) free(v1->first_element);
	v1->first_element = NULL;
	v1->num_element = 1;
	dp = (double *) v1->data;
	*dp = a;
	return 0;
}

double get_dbl_from_var(_var_t * v1){
	double * dp;
	dp = (double *) v1->data;
	return *dp;
}
/*
	Функция скроллирует текст, подвигая к первой позиции. При этом, первая строка
	сохраяется, в нее копируется следующая, остальные просто перемещаются.
	выбрасываемые строки не удалаются из памяти.
*/
int scroll_text(nicenv_t * e, _var_t * t, int nline)
{
	int i;
	_var_t * s = NULL;
	_var_t * v_n;
	if (t->type.type_index != TEXT)
	{
		printf("Not a TYPE var, skipping\n");
		return -1;
	}
	if (nline >= t->num_element){
		printf("Scroll out of text, skipping\n");
		return -2;
	}
	s = ((_var_t **) t->first_element)[nline];
	//printf("1\n");
	v_n = is_var_name_exists(e,"line0");
	if (v_n != NULL)
		set_string(v_n, (char*) s->first_element); // обновляем содержимое строки.

	//printf("2\n");
	//del_var_by_name(e, s->name);
	//printf("3\n");
	for (i=nline+1; i<t->num_element; i++){
		//printf("i=%d\n",i);
		s = ((_var_t **) t->first_element)[i];
		//printf("2:i=%d\n");
		if (s != NULL){
			//printf("3:i=%d, %s\n",i, s->name);
			((void **)t->first_element)[i-nline] = (void*) s;
			//printf("4:i=%d\n",i);
			((void **)t->first_element)[i] = NULL; // удаляем из строки
		}
	}
	//printf("4\n" );
	t->num_element -= nline; // поправляем число 
	//printf("5: nel = %d\n", t->num_element);
	return 0;
}
/*
	Эта функция скроллирует текст таким образом что она перемешает указатели в тексте
	!!! -------- продолжить здесь!! ---------------
*/
int scroll_text_remove(nicenv_t * e, _var_t * t, int nline)
{
	int i;
	_var_t * s = NULL;
	_var_t * v_n;
	if (t->type.type_index != TEXT)
	{
		printf("Not a TYPE var, skipping\n");
		return -1;
	}
	if (nline > t->num_element){
		printf("Scroll out of text, skipping\n");
		return -2;
	}
	
	for (i=0; i<nline; i++){
		s = ((_var_t **) t->first_element)[i];
		//printf("2:i=%d\n");
		if (s != NULL){
			del_var(e, s);
		}
	}

	for (i=nline; i<t->num_element; i++){
		//printf("i=%d\n",i);
		s = ((_var_t **) t->first_element)[i];
		//printf("2:i=%d\n");
		if (s != NULL){
			//printf("3:i=%d, %s\n",i, s->name);
			((void **)t->first_element)[i-nline] = (void*) s;
			//printf("4:i=%d\n",i);
			((void **)t->first_element)[i] = NULL; // удаляем из строки
		}
	}
	//printf("4\n" );
	t->num_element -= nline; // поправляем число 
	//printf("5: nel = %d\n", t->num_element);
	if (t->num_element == 0) del_var(e, t);
	return 0;
}

/* Создаем базу данных*/
int create_db(nicenv_t * e, _var_t * db){
	char * fname = (char *) db->first_element;
	struct stat st = {0};
	char pathFile[MAX_PATHNAME_LEN];
	int filedescriptor = 0;
	sprintf(pathFile, "./%s/db0000.txt", fname ); // это дефолтное имя для базы данных

	if (stat(fname, &st) == -1) {
		printf("Creating db: [%s]\n", fname);
    	mkdir(fname, 0755);
    	//filedescriptor = open(pathFile, O_RDWR | O_APPEND | O_CREAT);
    	//if (filedescriptor >= 0 ) close(filedescriptor);
	}
	return 0;
}

/* Проверяем, есть ли база данных*/
int is_db_exist(nicenv_t * e, _var_t * db){
	char * fname = (char *) db->first_element;
	DIR* dir = opendir(fname);
	if (dir)
	{	/* Directory exists. */
		printf("Directory is exist\n");
    	closedir(dir);
    	return 1;
	}
	else if (ENOENT == errno)
	{  	/* Directory does not exist. */
    	printf("Directory is not exist\n");
    	return 0;
	}	
	else
	{	printf("Directory open problem\n");
	    /* opendir() failed for some other reason. */
	    return -1;
	}
	return 0;
}
/* Добавляем переменную в базу данных*/
int add_var_to_db(nicenv_t * e, _var_t *db, _var_t * v){
	int i, j;
	char * fname = (char *) db->first_element;
	struct stat st = {0};
	char pathFile[MAX_PATHNAME_LEN];
	FILE * fp = NULL;
	int tp = 0;
	struct timeval start;
	long usecs;
	_var_t * s;

	sprintf(pathFile, "./%s/db0000.txt", fname ); // это дефолтное имя для базы данных

	printf("Opening db: [%s]\n", pathFile);

   	fp = fopen(pathFile, "a"); // будем дописывать
   	printf("The error is - %s\n", strerror(errno));
   	if (fp != NULL) {
		tp = v->type.type_index;
		printf("Type is [%d]\n", tp );
		gettimeofday(&start, NULL);
		usecs = (start.tv_sec*1000000L + start.tv_usec);
		switch(tp){
			case ARRA: break;
			case STRI:
			{
				printf("Adding string\n");
				fprintf(fp, "%ld \"%s\" %s STRI\n", usecs, (char*)v->first_element, v->name);
				break;
			}
			case FORM: break;
			case TEXT:
			{
				printf("Adding text\n");
				fprintf(fp, "%ld %s TEXT VAR \n", usecs, v->name);
				for (j=0; j<v->num_element; j++){
					s = ((_var_t**)(v->first_element))[j];
					if (((_var_t *) s->first_element != NULL) )
						fprintf(fp, "& \"%s\" ADD\n", (char*)s->first_element );
						
				}
				break;
			}	
			case STRU: break;
			case DOT:
			case PDOT: {
				printf("Not implemented\n");
				break;
			}
			case VEC:
			case RVEC: {
				printf("not implemented\n");
				break;
			}
			case LINE: {
				printf("not implemented\n");
				break;
			}
			case MAT: {
				printf("not implemented\n");
				break;
			}
			case MATX: {
				printf("not implemented\n");
				break;
			}
			case PATH: 
			case ARC: {
				printf("not implemented\n");
				break;
			}
			case TYPE: break;
			case VAR_BYTE: break;
			case VAR_LETR: break;
			case VAR_CHAR: break;
			case VAR_INT:
			{
				printf("Adding integer\n");	
				fprintf(fp, "%ld %ld %s INT VAR EQ\n", usecs, *((long *)v->data), v->name );
				break;
			}
			case VAR_DUBL:
			{
				printf("Adding double\n");	
				fprintf(fp, "%ld %lf %s DUBL VAR EQ\n", usecs, *((double *)v->data), v->name );
				break;
			}
			case DBASE:
			{
				printf("Adding Database\n");
				fprintf(fp, "%ld \"%s\" %s DB\n", usecs, (char*)v->first_element, v->name);
				break;
			}
			default:
			break;
		}
		fclose(fp);
		printf("Adding var to db\n");
	}
	return 0;
}


