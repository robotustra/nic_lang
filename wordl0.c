/*
*	Содержит слова 0 уровня. Должны подгружаться динамически.
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>
#include "common.h"
#include "wordl0.h"
#include "sh_inner.h"

int is_var(nicenv_t * e, _var_t * v);
int num_cells(nicenv_t * e, int nmin);
int push_int_number(nicenv_t * e, long lint);
int dsp1(nicenv_t * e);
int isp1(nicenv_t * e);
int dsp2(nicenv_t * e);
int isp2(nicenv_t * e);
void * gs(nicenv_t * e); //get cell from stack1 at position e->sp1
long gt(nicenv_t * e); // get cell type from stak1 at position e->sp1
void * gs1(nicenv_t * e); //get cell from stack1 at position e->sp1-1
long gt1(nicenv_t * e); // get cell type from stak1 at position e->sp1-1
void * ss(nicenv_t * e, void * var); //set cell from stack1 at position e->sp1
long st(nicenv_t * e, long tp); // set cell type from stak1 at position e->sp1
void * ss1(nicenv_t * e, void * var); //set cell from stack1 at position e->sp1-1
long st1(nicenv_t * e, long tp); // set cell type from stak1 at position e->sp1-1
void * ss2(nicenv_t * e, void * var); //set cell from stack1 at position e->sp1-2
long st2(nicenv_t * e, long tp); // set cell type from stak1 at position e->sp1-2
void * gs3(nicenv_t * e); //get cell from stack1 at position e->sp1-3
long gt3(nicenv_t * e); // get cell type from stak1 at position e->sp1-3
void * gs1_aux(nicenv_t * e);
long gt1_aux(nicenv_t * e);
void * ss_aux(nicenv_t * e, void * var); 
long st_aux(nicenv_t * e, long tp); 

int make_same_name_udef_var(nicenv_t * e);


/*Инкременторы и декременторы кольцевого стека*/
int dsp1(nicenv_t * e){
	if(e->sp1 == 0) return (e->sp1 = MAX_STACK_SIZE-1);
	return (e->sp1 --);
}
int isp1(nicenv_t * e){
	if(e->sp1 == (MAX_STACK_SIZE-1)) return (e->sp1 = 0);
	return (e->sp1 ++);
}
int dsp2(nicenv_t * e){
	if(e->sp2 == 0) return (e->sp2 = MAX_STACK_SIZE-1);
	return (e->sp2 --);
}
int isp2(nicenv_t * e){
	if(e->sp2 == (MAX_STACK_SIZE-1)) return (e->sp2 = 0);
	return (e->sp2 ++);
}
void * gs(nicenv_t * e){
	return e->S1[e->sp1];
}
long gt(nicenv_t * e){
	return e->T1[e->sp1];
}

//get cell from stack1 at position e->sp1-1
void * gs1(nicenv_t * e){
	if(e->sp1 == 0) return e->S1[MAX_STACK_SIZE-1];
	return e->S1[e->sp1-1];
}
// get cell type from stak1 at position e->sp1-1
long gt1(nicenv_t * e){
	if(e->sp1 == 0) return e->T1[MAX_STACK_SIZE-1];
	return e->T1[e->sp1-1];
}
void * gs1_aux(nicenv_t * e){
	if(e->sp2 == 0) return e->S2[MAX_STACK_SIZE-1];
	return e->S2[e->sp2-1];
}
// get cell type from stak1 at position e->sp1-1
long gt1_aux(nicenv_t * e){
	if(e->sp2 == 0) return e->T2[MAX_STACK_SIZE-1];
	return e->T2[e->sp2-1];
}

void * ss(nicenv_t * e, void * var){
	return (e->S1[e->sp1] = var);	
}
long st(nicenv_t * e, long lp){
	return (e->T1[e->sp1] = lp);	
}
void * ss_aux(nicenv_t * e, void * var){
	return (e->S2[e->sp2] = var);	
}
long st_aux(nicenv_t * e, long lp){
	return (e->T2[e->sp2] = lp);	
}
void * ss1(nicenv_t * e, void * var){
	if(e->sp1 == 0)  (e->S1[MAX_STACK_SIZE-1] = var);
	return (e->S1[e->sp1-1] = var);	
}
long st1(nicenv_t * e, long lp){
	if(e->sp1 == 0) return (e->T1[MAX_STACK_SIZE-1] = lp);
	return (e->T1[e->sp1-1] = lp);	
}
void * ss2(nicenv_t * e, void * var){
	void * r = NULL; dsp1(e); dsp1(e); r = ss(e,var); isp1(e); isp1(e); return r;
}
long st2(nicenv_t * e, long lp){
	long r = 0; dsp1(e); dsp1(e); r = st(e, lp); isp1(e); isp1(e); return r;
}
void * gs2(nicenv_t * e){
	void * r = NULL; dsp1(e); dsp1(e); r = gs(e); isp1(e); isp1(e); return r;
}
long gt2(nicenv_t * e){
	long r = 0; dsp1(e); dsp1(e); r = gt(e); isp1(e); isp1(e); return r;
} 
void * ss3(nicenv_t * e, void * var){
	void * r = NULL; dsp1(e); dsp1(e); dsp1(e); r = ss(e,var); isp1(e); isp1(e); isp1(e); return r;
}
long st3(nicenv_t * e, long lp){
	long r = 0; dsp1(e); dsp1(e); dsp1(e); r = st(e, lp); isp1(e); isp1(e); isp1(e); return r;
}
void * gs3(nicenv_t * e){
	void * r = NULL; dsp1(e); dsp1(e); dsp1(e); r = gs(e); isp1(e); isp1(e); isp1(e); return r;
}
long gt3(nicenv_t * e){
	long r = 0; dsp1(e); dsp1(e); dsp1(e); r = gt(e); isp1(e); isp1(e); isp1(e); return r;
} 
void drop1(nicenv_t * e){
	push_int_number(e,1); wl0_DROP(e);
}
/*	Если при создании новой переменной имя уже занято, то эта функция создает неопределенную переменную с таким же именем
	на вершине стека.
*/
int make_same_name_udef_var(nicenv_t * e){
	_var_t * v = NULL;
	v = (_var_t * ) gs1(e);
	_var_t * vn = NULL;
	if (is_var(e, v) && (v->type.type_index != UDEF)){
		drop1(e);
		vn = new_var(e, (char*) v->name, UDEF);
		push_var_ptr_to_stack(e,vn);
	}
	return 0;
}

/*Helper function to display type*/
int print_type(nicenv_t * e, int tp, char * type)
{
	_newtype_t * t = NULL;
	if (tp == VOID)					sprintf(type, "VOID");
	else if (tp == UDEF) 			sprintf(type, "UDEF");
	else if (tp == BYTE) 			sprintf(type, "BYTE");
	else if (tp == LETR)   			sprintf(type, "LETR");
	else if (tp == CHAR)   			sprintf(type, "CHAR");
	else if (tp == INT) 			sprintf(type, "INT");
	else if (tp == DUBL) 			sprintf(type, "DUBL");
	else if (tp == CODE) 			sprintf(type, "CODE");
	else if (tp == IDX)   			sprintf(type, "IDX");
	else if (tp == DIM)   			sprintf(type, "DIM");
	else if (tp == ARRA) 			sprintf(type, "ARRA");
	else if (tp == STRI) 			sprintf(type, "STRI");
	else if (tp == FORM)   			sprintf(type, "FORM");
	else if (tp == TEXT)   			sprintf(type, "TEXT");
	else if (tp == STRU) 			sprintf(type, "STRU");
	else if (tp == DOT) 			sprintf(type, "DOT");
	else if (tp == LINE) 			sprintf(type, "LINE");
	else if (tp == PDOT) 			sprintf(type, "PDOT");
	else if (tp == VEC)   			sprintf(type, "VEC");
	else if (tp == RVEC)   			sprintf(type, "RVEC");
	else if (tp == MAT)   			sprintf(type, "MAT");
	else if (tp == MATX)   			sprintf(type, "MATX");
	else if (tp == TYPE)   			sprintf(type, "TYPE");
	else if (tp == VAR_BYTE)		sprintf(type, "BYTE VAR");
	else if (tp == VAR_LETR)		sprintf(type, "LETR VAR");
	else if (tp == VAR_CHAR)		sprintf(type, "CHAR VAR");
	else if (tp == VAR_INT)			sprintf(type, "INT VAR");
	else if (tp == VAR_DUBL)		sprintf(type, "DUBL VAR");
	else if (tp == DBASE)			sprintf(type, "DBASE");
	else if (tp == PATH)			sprintf(type, "PATH");
	else if (tp == ARC)				sprintf(type, "ARC");
	// here I have to look for the type in the list of newly defined types after.
	else if (tp >= MAX_TYPE_INDEX)	{
		t = get_type_by_index(e, tp);
		if ( t!= NULL)
			sprintf(type, "%s", t->name );
		else
			sprintf(type, "UNKNOWN");
	}

	return 0;
}
int print_stack(nicenv_t * e, int stack)
{
	int i = 0;
	long dptr[2]; 
	long ltmp;
	char tp[MAX_NAME_LENGTH];
	_var_t * t_v;
	printf("---stack top---\n");
	i=e->sp1-1;
	while(i>=0)
	{
		switch(e->T1[i])
		{
			case VOID: {	
				printf("%d : %ld \t", i, (long)e->S1[i]);
				printf("VOID\n");
			}break;
			case UDEF: {
				printf("%d : %s \t", i, ((_var_t*) e->S1[i])->name );
				printf("UDEF\n");
			}break;
			case BYTE: {
				ltmp = (long) e->S1[i];
				printf("%d : %d \t", i, (unsigned char) ltmp );
				printf("BYTE\n");
			}break;
			case LETR: {
				ltmp = (long) e->S1[i];
				printf("%d : '%c' \t", i, (char) ltmp );
				printf("LETR\n");
			}break;
			case CHAR: {
				ltmp = (long) e->S1[i];
				printf("%d : '%c' \t", i, (char) ltmp );
				printf("CHAR\n");
			}break;
			case INT: {
				printf("%d : %ld \t", i, (long) e->S1[i] );
				printf("INT\n");
			}break;
			case DUBL: {
				dptr[1] = (long) e->S1[i];
				i--;
				dptr[0] = (long) e->S1[i];
				printf("%d : %e \t", i, *((double*) dptr) );
				printf("DUBL\n");
			}break;
			case CODE: {
				printf("%d : %s \t", i, ((_word_t *) e->S1[i])->name );
				printf("CODE\n");
			}break;
			case IDX: {
				printf("%d : %ld \t", i, (long) e->S1[i] );
				printf("IDX\n");
			}break;
			case DIM: {
				printf("%d : %ld \t", i, (long) e->S1[i] );
				printf("DIM\n");
			}break;
			case ARRA: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("ARRA\n");
			}break;
			case STRI: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("STRI\n");
			}break;
			case FORM: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("FORM\n");
			}break;
			case TEXT: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("TEXT\n");
			}break; 
			case STRU: { // really it's not variable, but new type.
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("STRU\n");
			}break;
			case DOT: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("DOT\n");
			}break;
			case PDOT: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("PDOT\n");
			}break;
			case VEC: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("VEC\n");
			}break;
			case RVEC: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("RVEC\n");
			}break;
			case LINE: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("LINE\n");
			}break;
			case PATH: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("PATH\n");
			}break;
			case ARC: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("ARC\n");
			}break;
			case MAT: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("MAT\n");
			}break;
			case MATX: {
				printf("%d : %s \t", i, ((_var_t *) e->S1[i])->name );
				printf("MATX\n");
			}break;
			case TYPE: {
				print_type(e, (long)e->S1[i], tp);
				printf("%d : %s \t", i, tp );
				printf("TYPE\n");
			}break;
			case VAR_BYTE:
			case VAR_LETR:
			case VAR_CHAR:
			case VAR_INT:
			case VAR_DUBL:
			{
				t_v = (_var_t *) e->S1[i];
				print_type(e, t_v->type.type_index, tp);
				printf("%d : %s \t%s\n", i, t_v->name , tp);
			}break;

			default: {
				// may be custom type
				t_v = (_var_t *) e->S1[i];
				print_type(e, t_v->type.type_index, tp);
				printf("%d : %s \t%s\n", i, t_v->name , tp);
				
			}
		}
		i--;
	}
	printf("---stack bottom---\n");
	return 0;
}

int pop_int(nicenv_t * e){
	if (gt1(e) == INT || gt1(e) == IDX){
		e->lint = (long) gs1(e);
		dsp1(e);
		return 0;
	}else{
		printf("pop_int: not int value\n");
	}
	return -1;
}

int pop_double(nicenv_t * e){
	long dbl[2];
	if (gt1(e) == DUBL){
		dbl[1] = (long) gs1(e);
		dsp1(e);
		dbl[0] = (long) gs1(e);
		dsp1(e);
		e->dbl = *((double*) dbl);
		return 0;
	}else{
		printf("pop_double: not double value\n");
	}
	return -1;
}

/*=============================================================================*/
int wl0_SP(nicenv_t * e)
{
	if (e->help) {
		printf("\tsp -> show the stack pointer value\n");
		return 0;
	}	
	printf("%d\n", e->sp1); // Печатает указатель на вершину в стека.
	return 0;
}
/* Clear stacks*/
int wl0_CS(nicenv_t * e)
{
	if (e->help) {
		printf("\tCS -> clear stacks\n");
		return 0;
	}	
	int i;
	for(i=0; i<MAX_STACK_SIZE; i++){
		e->S1[i] = (void *) 0;
		e->T1[i] = 0;

		e->S2[i] = (void *) 0;
		e->T2[i] = 0;	
	}
	e->sp1 = 0;
	e->sp2 = 0;
	return 0;
}

/* Number of elements*/
int wl0_NEL(nicenv_t * e)
{
	if (e->help) {
		printf("\tv NEL -> n /*Display the number of elements of complex object*/\n");
		return 0;
	}	
	_var_t * v;
	if(num_cells(e,1) && (
		gt1(e) == PATH )){ // потом нужно добавить сюда другие сложные объекты
		v = (_var_t * ) gs1(e);
		drop1(e);
		push_int_number(e, v->num_element);
	}
	return 0;
}


/* List of variables*/
int wl0_VARL(nicenv_t * e)
{
	if (e->help) {
		printf("\tList all variables created.\n");
		return 0;
	}
	int i=0;
	char tp[MAX_NAME_LENGTH];
	_var_t * next= e->vars_root.first;
	do{
		print_type(e, next->type.type_index, tp);
		printf("%s, %s\n", next->name, tp);
		if (next->next != NULL) next = next->next;
		else return -1;
		i++;
	}while (next != e->vars_root.first);
	return 0;
}

/* List of complex types*/
int wl0_TYPL(nicenv_t * e)
{
	if (e->help) {
		printf("\tList all complex types created.\n");
		return 0;
	}	
	int i=0;
	char tp[MAX_NAME_LENGTH];
	_newtype_t * next= e->types_root.first;
	if (next == NULL) {
		printf("No defined types\n");
		return 0;
	}
	do{
		printf("%s, type_index %d\n", next->name, next->type_index);
		if (next->next != NULL) next = next->next;
		else return -1;
		i++;
	}while (next != e->types_root.first);
	return 0;
}

/* List of words, level 0*/
int wl0_LIST(nicenv_t * e)
{
	if (e->help) {
		printf("\tList all loaded words.\n");
		return 0;
	}
	int i=0;
	_word_t * next= e->words_root.first;
	do{
		printf("%s\t", next->name);
		if (next->next != NULL) next = next->next;
		else return -1;
		i++;
		if (i%10 == 0) printf("\n");
	}while (next != e->words_root.first);
	printf("\n");
	return 0;
}
/* Show stacks */
int wl0_SHOW(nicenv_t * e)
{
	if (e->help) {
		printf("\tShow the stack.\n");
		return 0;
	}
	print_stack(e,1);
	return 0;
}

int wl0_HELP (nicenv_t * e)
{
	if (e->help) {
		printf("\t:word HELP -> Displays the usage of word\n");
		return 0;
	}
	_word_t * w = NULL;
	if ( (e->sp1 > 0) && (gt1(e) == CODE) ){
		w = (_word_t *)  gs1(e);
#if (DEBUG_LEVEL == 5)			
		printf("Executing the word: %s\n", w->name);
#endif		
		st1(e, 0);
		ss1(e, 0);
		dsp1(e);
		e->help = 1;
		(*(w->code))(e);
		e->help = 0;
	}else {
		printf("\t:word HELP -> Displays the usage of word\n");
		return 0;	
	}
	return 0;
}
/* Execute word on the top of stack*/
int wl0_RUN(nicenv_t * e){
	if (e->help) {
		printf("\t:<word> RUN\n\t<stri> RUN\n\t<text> RUN\n");
		return 0;
	}
	return run_word(e);
}
/* Выполняет несколько раз подряд, предыдущее слово.*/
int wl0_LOOP(nicenv_t * e){
	if (e->help) {
		printf("\t<stri> n LOOP\n\t<text> n LOOP\n /* Executes script n times */");
		return 0;
	}
	long i, cycl;
	if (num_cells(e, 2) && gt1(e) == INT 
		&& (gt2(e) == STRI
		|| gt2(e) == TEXT) 
		&& e->sp1 < MAX_STACK_SIZE){
		
		wl0_INC(e);
		// создаем временную переменную и присваиваем ей значение количества циклов
		char un[MAX_NAME_LENGTH];
		_var_t * vn;
		generate_unique_name(e,"it", un);
		vn = new_var(e, un, UDEF);
		push_var_ptr_to_stack( e, vn);
		wl0_EQ(e);
		drop1(e);

		// теперь нужно создать строку-условие для итерации столько раз сколько есть в счетчике
		// "_it.. valu dec _it.. qe valu"
		memset(e->string, 0, MAX_STRING);
		sprintf(e->string, "%s VALU DEC %s EQ VALU", un, un);
		add_string(e);
		char * s1_name = ((_var_t*)gs1(e))->name;
		// после завершения цикла нужно удалить итератор
		
		memset(e->string, 0, MAX_STRING);
		sprintf(e->string, "%s DEL", un);
		add_string(e);
		_var_t * s = (_var_t*)gs1(e);
		char * s2_name = s->name;
		// Добавляем в переменную две инструкции чтобы удалить лишние строки которые мы создали
		memset(e->string, 0, MAX_STRING);
		sprintf(e->string, "%s DEL %s DEL %s DEL", un, s1_name, s2_name);
		set_string(s, e->string); // теперь после выполенения цикла лишние переменные удаляться

		wl0_REPE(e); // запускаем цикл
	}
	return 0;
}
/* Выполняет цикл в зависимости от условия.*/
int wl0_REPE(nicenv_t * e){
	if (e->help) {
		printf("\t<txt> <cond> <end> REPE\n/* Executes code until <cond> is non 0.\n");
		printf("\t\tChecks <cond> before execution. Executes <end> after cond is false*/\n");
		return 0;
	}
	long i, cycl;
	if (num_cells(e, 3) 
		&& (gt1(e) == STRI || gt1(e) == TEXT)
		&& (gt2(e) == STRI || gt2(e) == TEXT) 
		&& (gt3(e) == STRI || gt3(e) == TEXT)
		&& e->sp1 < MAX_STACK_SIZE){
		_var_t * end = (_var_t*) gs1(e);
		_var_t * cond = (_var_t*) gs2(e);
		_var_t * body = (_var_t*) gs3(e);

		// добавляем тело цикла и условие
		if (e->loop_cnt < MAX_LOOPS){
			e->loop_body[e->loop_cnt] = body;
			e->loop_cond[e->loop_cnt] = cond;
			e->loop_end[e->loop_cnt] = end;
			e->loop_cnt++;
		}
		
		drop1(e); drop1(e); drop1(e);
		if (e->nest_cnt < MAX_NEST) {
			e->nest[e->nest_cnt] = 'L'; //loop
			e->nest_cnt++;
		}
	}
	return 0;
}

int wl0_IF(nicenv_t * e){
	if (e->help) {
		printf("\t<txt> <cond> IF /* IF operator.*/\n");
		printf("\t<txt_else> ELSE <txt_if> <cond> IF /* IF-ELSE operator.*/\n");
		return 0;
	}
	long i, cycl;
	if (num_cells(e, 2) 
		&& (gt1(e) == STRI || gt1(e) == TEXT)
		&& (gt2(e) == STRI || gt2(e) == TEXT) 
		&& e->sp1 < MAX_STACK_SIZE){
		_var_t * cond = (_var_t*) gs1(e);
		_var_t * if_body = (_var_t*) gs2(e);
		// добавляем тело в if условие
		if (e->ifelse_cnt < MAX_IF_ELSE){
			e->if_body[e->ifelse_cnt] = if_body;
			e->if_cond[e->ifelse_cnt] = cond;
			e->ifelse_cnt++;
		}
		drop1(e); drop1(e);
		if (e->nest_cnt < MAX_NEST) {
			e->nest[e->nest_cnt] = 'I';
			e->nest_cnt++;
		}
	}
	return 0;
}

int wl0_ELSE(nicenv_t * e){
	if (e->help) {
		printf("\t<txt> ELSE\n/* Prepare ELSE branch of IF operator, do not execute it*/\n");
		return 0;
	}
	long i, cycl;
	if (num_cells(e, 1) 
		&& (gt1(e) == STRI || gt1(e) == TEXT)
		&& e->sp1 < MAX_STACK_SIZE){
		_var_t * e_body = (_var_t*) gs1(e);
		// добавляем тело в if условие
		if (e->ifelse_cnt < MAX_IF_ELSE){
			e->else_body[e->ifelse_cnt] = e_body;
			// не увеличиваем счетчик, поскольку if должен идти следом
		}
		drop1(e);
	}
	return 0;
}

/* Устанавливаем флаг выполнимости для слова*/
int wl0_EXE(nicenv_t * e){
	if (e->help) {
		printf("\t<txt>|<str> EXE ->  /* Makes text or string executable */");
		return 0;
	}
	_var_t * v;
	if (num_cells(e, 1) 
		&& (gt1(e) == STRI	|| gt1(e) == TEXT) 
		&& e->sp1 < MAX_STACK_SIZE){
		// имеем строку или текст в качестве аргумента.
		v = (_var_t*) gs1(e);
		v->exec = 1; // делаем переменную выполнимой, и удаляем из стека, потому что следущий раз когда она
					 // будет загружена в стек она автоматически выполнится как скрипт.
		drop1(e);
	}
	return 0;
}
/* Сброс флага выполнимости с текста или со строки*/
int wl0_XEX(nicenv_t * e){
	if (e->help) {
		printf("\t:txt_name XEX ->  /* Clear executable flag of text or string*/");
		return 0;
	}
	_var_t * v, *s0;
	if (num_cells(e, 1) 
		&& (gt1(e) == STRI	|| gt1(e) == TEXT) 
		&& e->sp1 < MAX_STACK_SIZE){
		v = (_var_t*) gs1(e);
		v->exec = 0; 
		drop1(e);
	}
	return 0;
}

int push_int_number(nicenv_t * e, long lint)
{
	// Проверяем вершиину стека
	if (e->sp1 < MAX_STACK_SIZE){
		ss(e, (void*) lint); 
		st(e, INT);
		e->osp1 = e->sp1;
		isp1(e);	
	} else {
		printf("Warning: Stack is full, operation skipped.\n");
	}
	return 0;
}
/*Drops 1 elements (NOT CELLS!!) from the stack 1. Double is removed AS a whole!*/
int wl0_DR(nicenv_t * e){
	if (e->help) {
		printf("\ta n DR -> /*Remove top stack element*/\n");
		return 0;
	}
	int i, j, n;
	if( e->sp1 == MAX_STACK_SIZE) {
		ss1(e, 0);
		st1(e, 0);
		dsp1(e);
		return 0;
	}
	if (num_cells(e, 1) ){
		drop1(e);	
	}
	return 0;
}
/*Drops N elements (NOT CELLS!!) from the stack 1. Double is removed AS a whole!*/
int wl0_DROP(nicenv_t * e){
	if (e->help) {
		printf("\t .. a0 a1 a2 .. an n DROP -> .. a0\n");
		return 0;
	}
	int i, j, n;

	if( e->sp1 == MAX_STACK_SIZE) {
		ss1(e, 0);
		st1(e, 0);
		dsp1(e);
		return 0;
	}
	if ( (e->sp1 > 1) && (gt1(e) == INT)){
		n = (long) gs1(e); //Number elements to drop without this number
		//printf("Dropping %d elements\n", n);
		j = (e->sp1-1 > n)?(e->sp1-1-n): 0 ;
		i=e->sp1-1;
		while (i>=j){
			if (e->T1[i] == DUBL){
				e->S1[i] = 0;
				e->T1[i] = 0;
				i--;
				j--;
			}
			e->S1[i] = 0;
			e->T1[i] = 0;
			i--;
		}
		//printf ("until %d\n", j);
		e->sp1 = j;
	}else
	{
		printf("Not integer number of elements, skipping DROP\n");
	}
	return 0;
}
/*Put type of variable to the stack*/
int wl0_TYPE(nicenv_t * e){
	if (e->help) {
		printf("\ta TYPE -> a <type(a)>\n");
		return 0;
	}
	if ( e->sp1 >0 && e->sp1 < MAX_STACK_SIZE ){
		ss(e, (void*) gt1(e));
		st(e, TYPE);
		isp1(e);
	}	
	return 0;
}

/* Dummy exit word*/
int wl0_EXIT(nicenv_t * e){
	if (e->help) {
		printf("\tEXIT -> Exit from nic\n");
		return 0;
	}
	return 0;
}

/*Overwrites the type of the cell*/
int wl0_AS(nicenv_t * e){
	if (e->help) {
		printf("\tvx t AS -> vt\n");
		return 0;
	}
	_var_t * v = NULL;
	long t = 0;
	if(num_cells(e, 2)){
		if(gt1(e) == TYPE && is_var(e, (_var_t*) gs2(e))){
			v = (_var_t*) gs2(e);
			t = (long) gs1(e);
			if (t == INT){ st2(e, VAR_INT); v->type.type_index = VAR_INT; }
			else if (t == LETR){ st2(e, VAR_LETR); v->type.type_index = VAR_LETR; }
			else if (t == CHAR){ st2(e, VAR_CHAR); v->type.type_index = VAR_CHAR; }
			else if (t == BYTE){ st2(e, VAR_BYTE); v->type.type_index = VAR_BYTE; }
			else if (t == DUBL){ st2(e, VAR_DUBL); v->type.type_index = VAR_DUBL; }
			else { st2(e, t); v->type.type_index = t; }
			drop1(e);
			return 0;
		}
		// наверное нельзя этого делать потому, что присваивать тип простому числу не совсем правильно
		// может быть сегфолт дальше
		if(gt1(e) == TYPE && !(is_var(e, (_var_t*) gs2(e)))){
			st2(e, (long) gs1(e));
			drop1(e);
			return 0;
		}
		
	}
	return 0;
}

/*Inserts the void into the stack*/
int wl0_VOID(nicenv_t * e){
	if (e->help) {
		printf("\tVOID -> <void>\n");
		return 0;
	}
	if ( e->sp1 < MAX_STACK_SIZE ){
		ss(e, 0);
		st(e, VOID);
		isp1(e);
	}	
	return 0;
}
int set_type (nicenv_t * e, long type){
	if ( e->sp1 < MAX_STACK_SIZE ){
		ss(e, (void*) type);
		st(e, TYPE);
		isp1(e);
	}	
	return 0;
}
int wl0_UDEF(nicenv_t * e){
	if (e->help) {
		printf("\tUDEF -> u\n");
		return 0;
	}
	return set_type(e, UDEF);
}
int wl0_BYTE(nicenv_t * e){
	if (e->help) {
		printf("\tBYTE -> b\n");
		return 0;
	}
	return set_type(e, BYTE);	
}
int wl0_LETR(nicenv_t * e){
	if (e->help) {
		printf("\tLETR -> a\n");
		return 0;
	}
	return set_type(e, LETR);
}
int wl0_CHAR(nicenv_t * e){
	if (e->help) {
		printf("\tCHAR -> c\n");
		return 0;
	}
	return set_type(e, CHAR);
}
int wl0_INT(nicenv_t * e){
	if (e->help) {
		printf("\tINT -> i\n");
		return 0;
	}
	return set_type(e, INT);
}
int wl0_DUBL(nicenv_t * e){
	if (e->help) {
		printf("\tDUBL -> d\n");
		return 0;
	}
	return set_type(e, DUBL);
}
int wl0_TEXT(nicenv_t * e){
	if (e->help) {
		printf("\tTEXT -> t\n");
		return 0;
	}
	return set_type(e, TEXT);
}
/*Загружает слова из файла в текстовую переменную.*/
int wl0_WORD(nicenv_t * e){
	if (e->help) {
		printf("\t\"script_name\" w WORD-> w /*Load script into variable w and execute it*/\n");
		return 0;
	}
	_var_t * v = NULL;
	v = (_var_t * ) gs1(e);
	_var_t * vn = NULL;
	// если переменная уже определена, то создаем переменную с таким же именем
	if ( is_var(e, v) && (v->type.type_index != UDEF)){
		drop1(e);
		vn = new_var(e, (char*) v->name, UDEF);
		push_var_ptr_to_stack(e,vn);
		v = vn;
	}
	if (num_cells(e,2) && gt1(e) == UDEF && gt2(e) == STRI){
		// Загружаем текст
		wl0_LOAD(e);
		wl0_RUN(e);
	}
	return 0;
}
/*Отображаем все имеющиеся скрипты*/
int wl0_LSS(nicenv_t * e){
	if (e->help) {
		printf("\t LSS-> /*List all scripts into ./scripts folder*/\n");
		return 0;
	}
	_var_t * v = NULL;
	if (e->sp1 < MAX_STACK_SIZE-1){
		// "ls ./scripts"
		sprintf(e->string,"ls ./scripts");
		add_string(e);
		wl0_SH(e);
	}
	return 0;
}
int wl0_SCRI(nicenv_t * e)
{
	if (e->help) {
		printf("\t\"script_name\" txt SCR -> Load file from \"./script/script_name.nic\" to undefined txt variable, no execution.\n");
		return 0;
	}
	// если переменная уже определена, то создаем UDEF переменную с таким же именем, экранируя уже существующую
	_var_t * v = NULL;
	v = (_var_t * ) gs1(e);
	_var_t * vn = NULL;
	if (num_cells(e,2) && is_var(e, v) && (v->type.type_index != UDEF) && gt2(e) == STRI){
		drop1(e);
		vn = new_var(e, (char*) v->name, UDEF);
		push_var_ptr_to_stack(e,vn);
	}
    if (num_cells(e,2) && gt1(e) == UDEF && gt2(e) == STRI)
    {
    	push_int_number(e,1);
    	wl0_FLIP(e);
        _var_t * s = (_var_t *) gs1(e); // строка с именем скрипта, который надо загрузить
        drop1(e);
        // модифицируем новую строку
        sprintf(e->string,"./scripts/%s.nic", (char*) s->first_element);
        add_string(e);
        push_int_number(e,1);
    	wl0_FLOP(e);
    	wl0_LOAD(e);
	}
    return 0;
}

int wl0_CODE(nicenv_t * e){
	if (e->help) {
		printf("\tn CODE -> c  /*Modify INT value to CODE*/ \n");
		return 0;
	}
	if (num_cells(e, 1) && gt1(e) == INT){
		st1(e,CODE);
	}
	return 0;
}

int wl0_DIM(nicenv_t * e){
	if (e->help) {
		printf("\tDIM -> d  /*Modify INT value to dimension*/ \n");
		return 0;
	}
	if (num_cells(e, 1) && gt1(e) == INT){
		st1(e,DIM);
	}
	return 0;
}
/*
* 	Индексация должна выделять подобъект из сложного объекта
* 	Возможно я должен создать другую команду, которая по индексу 
*	выделяет подобъект
*/
int wl0_IDX(nicenv_t * e){
	if (e->help) {
		printf("\tIDX -> d\n");
		return 0;
	}
	if (num_cells(e, 1) && (gt1(e) == INT || gt1(e) == VAR_INT) ){
		if (gt1(e) == VAR_INT) wl0_VALU(e);
		st1(e, IDX);
	}
	return 0;
}
/*
	Объявляет переменную связанной с базой данных
*/
int wl0_DB(nicenv_t * e){
	if (e->help) {
		printf("\tfile_path var DB -> db_var /* Associate file_path string with the database var. */\n");
		return 0;
	}
	_var_t * str1, * str2;
	if(num_cells(e,2)){
		// если переменная уже определена, то создаем UDEF переменную с таким же именем, экранируя уже существующую
		_var_t * v = NULL;
		v = (_var_t * ) gs1(e);
		_var_t * vn = NULL;
		if (is_var(e, v) && (v->type.type_index != UDEF) && gt2(e) == STRI){
			drop1(e);
			vn = new_var(e, (char*) v->name, UDEF);
			push_var_ptr_to_stack(e,vn);
		}
		if (gt1(e) == UDEF && gt2(e) == STRI){
			str1 = (_var_t *) gs2(e);
			str2 = (_var_t *) gs1(e);
			fork_var(str1, str2); // Выделяем новую память
			st1(e, DBASE);
			str2->type.type_index = DBASE;
			set_string(str2, (char*) str1->first_element); // База данных это просто имя файла, обычная строка с именем
			wl0_SWAP(e);
			drop1(e);

		}
	}
	return 0;
}

/*Создаем матрицу из объектов. Может содержать 9 до 16 DUBL*/
int make_a_mat(nicenv_t * e, int type){
	_var_t * v1, * v2;
	if(num_cells(e,3)){
		// координаты могут задаваться как константами так и переменные, при этом нужно брать только
		// значение переменных. Переменные могут быть целого типа или DUBL.
		int t;
		int i, j;
		int k = 2;
		double mat[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
		_var_t * d1, *d2;
		double * fe = NULL;
		make_same_name_udef_var(e);
		if (gt1(e) == UDEF)	
		{
			v1 = (_var_t *) gs1(e);
			st1(e, type);
			v1->type.type_index = type;
			set_matx_to_var(v1, mat); // инициализация по умолчанию, так как аргументы могу быть плохими
			drop1(e);
			if (type == MATX) k = 3;
			for (i=k; i>=0; i--){
				t = gt1(e); // конец линии
				if (t == VEC || t == PDOT || t == DOT){
					d1 = (_var_t *) gs1(e);
					fe = (double*) d1->first_element;
					for (j=0; j<d1->num_element; j++) mat[i*4 + j] = fe[j];	 
				} else {
					return 0;
				}	
				drop1(e);
			}
			set_matx_to_var(v1, mat);
			push_var_ptr_to_stack(e,v1);
		}
	}
	return 0;
}

/*Создаем линию из объектов. Может содерать от 6 до 8 DUBL*/
int make_a_line(nicenv_t * e){
	_var_t * v1, * v2;
	if(num_cells(e,3)){
		// координаты могут задаваться как константами так и переменные, при этом нужно брать только
		// значение переменных. Переменные могут быть целого типа или DUBL.
		int t;
		int i, j;
		double line[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
		_var_t * d1, *d2;
		double * fe = NULL;
		make_same_name_udef_var(e);
		if (gt1(e) == UDEF)	
		{
			v1 = (_var_t *) gs1(e);
			st1(e, LINE);
			v1->type.type_index = LINE;
			set_line_to_var(v1, line); // инициализация по умолчанию, так как аргументы могу быть плохими
			drop1(e);
			for (i=1; i>=0; i--){
				t = gt1(e); // конец линии
				if (t == DOT) {
					d1 = (_var_t *) gs1(e);
					fe = (double*) d1->first_element;
					for (j=0; j<d1->num_element; j++) line[i*4 + j] = fe[j];	 
				} else {
					return 0;
				}	
				drop1(e);
			}
			set_line_to_var(v1, line);
			push_var_ptr_to_stack(e,v1);
		}
	}
	return 0;
}
/*Создаем объект - отрезок линии*/
int wl0_LINE(nicenv_t * e){
	if (e->help) {
		printf("\tdot1 dot2 line_name LINE -> line_var /*Create LINE object*/\n");
		return 0;
	}
	make_a_line(e);
	return 0;
}

/* Путь, или последовательность точек, векторов, дуг. Может быть замкнутая или незамкнутая*/
int wl0_PATH(nicenv_t * e){
	if (e->help) {
		printf("\tvoid dot1 dot2 .. dotn path PATH -> path /*Create PATH object*/\n");
		return 0;
	}
	_var_t * v1, * v2;
	// координаты могут задаваться как константами так и переменные, при этом нужно брать только
	// значение переменных. Переменные могут быть целого типа или DUBL.
	int t;
	int i, j;
	_var_t * d1; // временная переменная
	void ** fe = NULL;
	make_same_name_udef_var(e);
	if (gt1(e) == UDEF)	
	{
		v1 = (_var_t *) gs1(e);
		drop1(e);
		// Определяем, какое количество элементов нужно выделить для создания пути.
		j=0;
		for( i=e->sp1-1; i>=0; i--){
			//printf("type = %d\n", e->T1[i]);
			if (e->T1[i] == VOID) break;
			if (e->T1[i] == DOT || e->T1[i] == PDOT 
				|| e->T1[i] == LINE || e->T1[i] == ARC) j++;
				
			else {
				printf("Warning: incorrect arguments, skipping\n");
				return 0;
			}

		}
		j--;
		//printf("j=%d\n", j);
		if (j>0){
			// выделяем память для сложного объекта
			v1->type.type_index = PATH;
			alloc_var(v1, j+1);
			fe = (void**) v1->first_element;
			for (i=j; i>=0; i--){

				fe[i] = (void *) gs1(e); // верхний элемент стека в конец массива
				drop1(e);
				//printf("drop1\n");
			}
			if ((e->sp1 >0) && (gt1(e) == VOID)){
				drop1(e);
				//printf("drop2\n");
			}
		}
		push_var_ptr_to_stack(e,v1);
	}
	return 0;
}
/* Замыкает PATH. Устанавливаем флаг замкнутости.*/
int wl0_CLOS(nicenv_t * e)
{
	if (e->help) {
		printf("\tpath CLOS -> path' /*Make PATH closed*/\n");
		return 0;
	}	
	_var_t * v;
	if(num_cells(e,1) && (
		gt1(e) == PATH )){
		v = (_var_t * ) gs1(e);
		v->data[0] = 1; // устанавливаем флаг замкнутости контура
	}
	return 0;
}
/* Размыкаем контур*/
int wl0_OPEN(nicenv_t * e){
	if (e->help) {
		printf("\tpath OPEN -> path' /*Make PATH open*/\n");
		return 0;
	}	
	_var_t * v;
	if(num_cells(e,1) && (
		gt1(e) == PATH )){
		v = (_var_t * ) gs1(e);
		v->data[0] = 0; // устанавливаем флаг замкнутости контура
	}
	return 0;	
}

int wl0_ARC(nicenv_t * e)
{
	if (e->help) {
		printf("\trvec1 rvec2 a ARC -> a /*Creare Arc object from current point with direction of rvec1, \n*/");
		printf("\tradius |rvec1| in the plane perpendicular to rvec2 and turn |rvec2| in the direction defined by sign(rvec2)*\n");
		return 0;
	}	
	_var_t * ar;
	void * rn;
	void * rv;
	int nelem = 2; // два аргумента у дуги.
	if(num_cells(e,3) && 
		(gt1(e) == UDEF) &&
		(gt2(e) == RVEC) &&
		(gt3(e) == RVEC))
	{
		ar = (_var_t * ) gs1(e);
		rn = gs2(e);
		rv = gs3(e);

		push_int_number(e,3);
		wl0_DROP(e);

		// создать новый объект и положить туда эти указатели.
		nelem *= sizeof(void*);
		printf("Allocate memory %ld bytes\n", nelem );

		ar->first_element = (void **) malloc (nelem);
		if (ar->first_element == NULL){
			printf("Failed to allocate memory\n");
		}else
		{
			((void**)(ar->first_element))[0] = rv;
			((void**)(ar->first_element))[1] = rn;
			ar->type.type_index = ARC;
			push_var_ptr_to_stack(e, ar);
		}
	}
	return 0;
}

/* Проходим по контуру и выбрасываем повторяющие точки, стоящие рядом.*/
int wl0_SIMP(nicenv_t * e){
	if (e->help) {
		printf("\tpath SIMP -> path' /*Simplify the path*/\n");
		return 0;
	}	
	_var_t * v;
	_var_t * t1, * t2;
	void ** fe = NULL;
	int i, j, n = 0;
	if(num_cells(e,1) && (gt1(e) == PATH ))
	{
		v = (_var_t * ) gs1(e); // получили указатель на путь.
		n = v->num_element;
		fe = (void**) v->first_element;
		// Путь состоит из объектов, поэтому для начала убираем повторяющиеся объекты 
		if  (n > 0 ) t1 = (_var_t * ) fe[0];
		else return 0;
		i=1;
		while (i<n){
			// проверяем на совпадение
			t2 = (_var_t * ) fe[i];
			if (t2 == t1) {
				// точки совпадают, смещаем все на 1 позицию
				for (j=i+1; j<n; j++){
					fe[j-1]= fe[j];
				}
				n--; // количество точек уменьшаем на единицу
				v->num_element --;
				i--; // проверяем текущий элемент опять.
			}else{
				// сдвигаемся на следующий элемент
				t1 = t2;
			}
			i++;
		}
	}
	return 0;		
}

/* Создание точки. Это фактически 3-мерный массив of DUBL*/
int make_a_dot(nicenv_t * e, int type){
	_var_t * v1, * v2;
	if(num_cells(e,4)){
		// координаты могут задаваться как константами так и переменные, при этом нужно брать только
		// значение переменных. Переменные могут быть целого типа или DUBL.
		int t;
		int i;
		double dot[3] = {0.0, 0.0, 0.0};
		double r = 0.0;
		make_same_name_udef_var(e);		
		if (gt1(e) == UDEF)	
		{
			v1 = (_var_t *) gs1(e);
			st1(e, type);
			v1->type.type_index = type;
			set_dot_to_var(v1, 0.0, 0.0, 0.0, 0.0); // инициализация по умолчанию, так как аргументы могу быть плохими
			drop1(e);
			if (type == PDOT || type == RVEC){
				t = gt1(e);
				if (t == DUBL) {
					pop_double(e);
					r = e->dbl;
				} else if (t == INT){
					pop_int(e);
					r = (double) e->lint;
				} else if (t == VAR_INT){
					wl0_VALU(e);
					pop_int(e);
					r = (double) e->lint;
				} else if (t == VAR_DUBL){
					wl0_VALU(e);
					pop_double(e);
					r = e->dbl;
				} else {
					return 0;
				}
			}
			for (i=2; i>=0; i--){
				t = gt1(e);
				if (t == DUBL) {
					pop_double(e);
					dot[i] = e->dbl;
				} else if (t == INT){
					pop_int(e);
					dot[i] = (double) e->lint;
				} else if (t == VAR_INT){
					wl0_VALU(e);
					pop_int(e);
					dot[i] = (double) e->lint;
				} else if (t == VAR_DUBL){
					wl0_VALU(e);
					pop_double(e);
					dot[i] = e->dbl;
				} else {
					return 0;
				}	
			}
			set_dot_to_var(v1, dot[0], dot[1], dot[2], r);
			push_var_ptr_to_stack(e,v1);
		}
	}
	return 0;
}
int wl0_DOT(nicenv_t * e){
	if (e->help) {
		printf("\tx y z dname DOT -> dot_var /*Create DOT object*/\n");
		return 0;
	}
	make_a_dot(e, DOT);
	return 0;
}
int wl0_VEC(nicenv_t * e){
	if (e->help) {
		printf("\tx y z vname VEC -> vec_var /*Create VEC object*/\n");
		return 0;
	}
	make_a_dot(e, VEC);
	return 0;
}
/* Делает вектор единичным если он ненулевой*/
int wl0_UVEC(nicenv_t * e){
	if (e->help) {
		printf("\tv UVEC -> v1 /*Make unit lenght VEC object*/\n");
		return 0;
	}
	_var_t * v1, * v2;
	if(num_cells(e,1)){
		int i;
		double * fe = NULL;
		double len = 0.0;
		if (gt1(e) == VEC || gt1(e) == RVEC)	
		{
			v1 = (_var_t *) gs1(e);
			fe = (double *) v1->first_element;
			for (i=2; i>=0; i--){
				len += fe[i] * fe[i];
			}
			len = sqrt(len);
			if (len>0) {
				fe[0] /= len;
				fe[1] /= len;
				fe[2] /= len;
			}
		}
	}
	return 0;
	
}
/* Физическая точка, задается центром и радиусом*/
int wl0_PDOT(nicenv_t * e){
	if (e->help) {
		printf("\tx y z r dname PDOT -> pdot_var /*Create PDOT object*/\n");
		return 0;
	}
	make_a_dot(e, PDOT);
	return 0;
}
int wl0_RVEC(nicenv_t * e){
	if (e->help) {
		printf("\tux uy uz r rvame RVEC -> rvec_var /*Create RVEC object*/\n");
		return 0;
	}
	make_a_dot(e, RVEC);
	return 0;
}
/*вычисляет длину трехмерного вектора*/
int wl0_LEN(nicenv_t * e){
	if (e->help) {
		printf("\tv LEN -> d /*Calculate the  lenght VEC or RVEC object*/\n");
		return 0;
	}
	_var_t * v1, * v2;
	if(num_cells(e,1)){
		int i;
		double * fe = NULL;
		double len = 0.0;
		if (gt1(e) == VEC) 	
		{
			v1 = (_var_t *) gs1(e);
			fe = (double *) v1->first_element;
			for (i=2; i>=0; i--){
				len += fe[i] * fe[i];
			}
			drop1(e);
			e->dbl = sqrt(len);
			push_dbl_to_stack(e);
		}else
		if (gt1(e) == RVEC) 	
		{
			v1 = (_var_t *) gs1(e);
			fe = (double *) v1->first_element;
			drop1(e);
			e->dbl = fe[3];
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
/*
SMUL	- скалярное умножение векторов
VMUL 	- векторное произведение 
VADD	- сложение векторов
VSUB	- вычитание векторов
*/
int wl0_SMUL(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec1 SMUL -> d /*Calculate scalar product of vec1 and vec2*/\n");
		return 0;
	}
	_var_t * v1, * v2;
	if(num_cells(e,2)){
		int i;
		double * fe = NULL;
		double * fe1 = NULL;
		double prod = 0.0;
		if (gt1(e) == VEC && gt2(e) == VEC) 	
		{
			v1 = (_var_t *) gs1(e);
			fe = (double *) v1->first_element;
			v2 = (_var_t *) gs2(e);
			fe1 = (double *) v2->first_element;
			for (i=2; i>=0; i--){
				prod += fe[i] * fe1[i];
			}
			drop1(e); drop1(e);
			e->dbl = prod;
			push_dbl_to_stack(e);
		}
	}
	return 0;	
}
/*
* 	матричное умножение 3x3, 4x4
*/
int wl0_MMUL(nicenv_t * e){
	if (e->help) {
		printf("\tmat1 mat2 MMUL -> mat3 /*Matrix multiplication*/\n");
		printf("\tmatx1 matx2 MMUL -> matx3 \n");
		printf("\td mat MMUL -> d*mat \n");
		printf("\tmat d MMUL -> d*mat \n");
		printf("\tvec mat MMUL -> vec' \n");
		printf("\tmat vec MMUL -> vec' \n");
		printf("\tdot mat MMUL -> vec' \n");
		printf("\tmat dot MMUL -> vec' \n");
		return 0;
	}
	char name[MAX_NAME_LENGTH];
	double * da, * db;
	_var_t * v1, *v2, *d;
	int i, t;
	if (num_cells(e,2)){
		if ( gt1(e) == MAT && gt2(e) == MAT){
			v1 = (_var_t *) gs2(e);
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e);
			db = (double *) v2->first_element;
			drop1(e); drop1(e);
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			e->dbl = da[0] * db[0] + da[1] * db[4] + da[2] * db[8];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[1] + da[1] * db[5] + da[2] * db[9];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[2] + da[1] * db[6] + da[2] * db[10];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_VEC(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[4] * db[0] + da[5] * db[4] + da[6] * db[8];
			push_dbl_to_stack(e);
			e->dbl = da[4] * db[1] + da[5] * db[5] + da[6] * db[9];
			push_dbl_to_stack(e);
			e->dbl = da[4] * db[2] + da[5] * db[6] + da[6] * db[10];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_VEC(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[8] * db[0] + da[9] * db[4] + da[10] * db[8];
			push_dbl_to_stack(e);
			e->dbl = da[8] * db[1] + da[9] * db[5] + da[10] * db[9];
			push_dbl_to_stack(e);
			e->dbl = da[8] * db[2] + da[9] * db[6] + da[10] * db[10];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_VEC(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			// создаем матрицу
			generate_unique_name (e, "mm", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			wl0_MAT(e);
			wl0_ADEL(e);
		} else
		if ( gt1(e) == MATX && gt2(e) == MATX)
		{
			v1 = (_var_t *) gs2(e);
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e);
			db = (double *) v2->first_element;
			drop1(e); drop1(e);
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			e->dbl = da[0] * db[0] + da[1] * db[4] + da[2] * db[8] + da[3] * db[12];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[1] + da[1] * db[5] + da[2] * db[9] + da[3] * db[13];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[2] + da[1] * db[6] + da[2] * db[10] + da[3] * db[14];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[3] + da[1] * db[7] + da[2] * db[11] + da[3] * db[15];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[4] * db[0] + da[5] * db[4] + da[6] * db[8] + da[7] * db[12];
			push_dbl_to_stack(e);
			e->dbl = da[4] * db[1] + da[5] * db[5] + da[6] * db[9] + da[7] * db[13];
			push_dbl_to_stack(e);
			e->dbl = da[4] * db[2] + da[5] * db[6] + da[6] * db[10] + da[7] * db[14];
			push_dbl_to_stack(e);
			e->dbl = da[4] * db[3] + da[5] * db[7] + da[6] * db[11] + da[7] * db[15];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[8] * db[0] + da[9] * db[4] + da[10] * db[8] + da[11] * db[12];
			push_dbl_to_stack(e);
			e->dbl = da[8] * db[1] + da[9] * db[5] + da[10] * db[9] + da[11] * db[13];
			push_dbl_to_stack(e);
			e->dbl = da[8] * db[2] + da[9] * db[6] + da[10] * db[10] + da[11] * db[14];
			push_dbl_to_stack(e);
			e->dbl = da[8] * db[3] + da[9] * db[7] + da[10] * db[11] + da[11] * db[15];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[12] * db[0] + da[13] * db[4] + da[14] * db[8] + da[15] * db[12];
			push_dbl_to_stack(e);
			e->dbl = da[12] * db[1] + da[13] * db[5] + da[14] * db[9] + da[15] * db[13];
			push_dbl_to_stack(e);
			e->dbl = da[12] * db[2] + da[13] * db[6] + da[14] * db[10] + da[15] * db[14];
			push_dbl_to_stack(e);
			e->dbl = da[12] * db[3] + da[13] * db[7] + da[14] * db[11] + da[15] * db[15];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			// создаем матрицу
			generate_unique_name (e, "mx", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			wl0_MATX(e);
			wl0_ADEL(e);
		}
		else if ( (gt1(e) == MAT || gt1(e) == MATX )&& gt2(e) == DUBL){
			//printf(" dubl mat mmul\n" );
			v1 = (_var_t *) gs1(e);
			da = (double *) v1->first_element;
			drop1(e);
			pop_double(e);
			for (i=0; i<v1->num_element; i++){
				if (da[i]!= 0.0) da[i] *= e->dbl;
			}
			push_var_ptr_to_stack(e, v1);
		} 
		else if ( gt1(e) == DUBL && (gt3(e) == MAT || gt3(e) == MATX )){
			pop_double(e);
			v1 = (_var_t *) gs1(e);
			da = (double *) v1->first_element;
			for (i=0; i<v1->num_element; i++){
				if (da[i]!= 0.0) da[i] *= e->dbl;
			}
		}
		else if ( (gt1(e) == MATX || gt1(e) == MAT) && 
				(gt2(e) == DOT ||gt2(e) == VEC) ) {
			v1 = (_var_t *) gs2(e); // vector
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e); // matrix
			db = (double *) v2->first_element;
			t = gt2(e);
			drop1(e); drop1(e);
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			e->dbl = da[0] * db[0] + da[1] * db[4] + da[2] * db[8] + da[3] * db[12];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[1] + da[1] * db[5] + da[2] * db[9] + da[3] * db[13];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[2] + da[1] * db[6] + da[2] * db[10] + da[3] * db[14];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[3] + da[1] * db[7] + da[2] * db[11] + da[3] * db[15];
			push_dbl_to_stack(e);
			// создаем вектор
			generate_unique_name (e, "mm", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			wl0_PDOT(e); // создаем физическую точку, но на самом деле это 4 мерный вектор
			// подправляем тип переменной, поскольку входные
			d->num_element = 3;
			d->type.type_index = t;
			st1(e, t);
			wl0_ADEL(e);
		} 
		else if ( (gt2(e) == MATX || gt2(e) == MAT) && 
				(gt1(e) == DOT ||gt1(e) == VEC) ) {
			v1 = (_var_t *) gs1(e); // vector
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs2(e); // matrix
			db = (double *) v2->first_element;
			t = gt2(e);
			drop1(e); drop1(e);
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			e->dbl = da[0] * db[0] + da[1] * db[1] + da[2] * db[2] + da[3] * db[3] ;
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[4] + da[1] * db[5] + da[2] * db[6] + da[3] * db[7];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[8] + da[1] * db[9] + da[2] * db[10] + da[3] * db[11];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[12] + da[1] * db[13] + da[2] * db[14] + da[3] * db[15];
			push_dbl_to_stack(e);
			// создаем вектор
			generate_unique_name (e, "mm", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			wl0_PDOT(e); // создаем физическую точку, но на самом деле это 4 мерный вектор
			// подправляем тип переменной, поскольку входные
			d->num_element = 3;
			d->type.type_index = t;
			st1(e, t);
			wl0_ADEL(e);
		} 
	}
	return 0;
}
/* 	Транспонирование матрицы */
int wl0_MTRA(nicenv_t * e){
	if (e->help) {
		printf("\tmat MTRA -> mat' /*Matrix transposition*/\n");
		return 0;
	}
	char name[MAX_NAME_LENGTH];
	double * da, * db;
	_var_t * v1, *v2, *d;
	int i;
	if (num_cells(e,1)){
		if ( gt1(e) == MAT){
			v1 = (_var_t *) gs1(e);
			da = (double *) v1->first_element;
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			// 1<->4
			e->dbl = da[4];
			da[4] = da[1];
			da[1] = e->dbl;
			// 2<->8
			e->dbl = da[8];
			da[8] = da[2];
			da[2] = e->dbl;
			//6<->9
			e->dbl = da[9];
			da[9] = da[6];
			da[6] = e->dbl;
		}
		else if ( gt1(e) == MATX){
			v1 = (_var_t *) gs1(e);
			da = (double *) v1->first_element;
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			// 1<->4
			e->dbl = da[4];
			da[4] = da[1];
			da[1] = e->dbl;
			// 2<->8
			e->dbl = da[8];
			da[8] = da[2];
			da[2] = e->dbl;
			//6<->9
			e->dbl = da[9];
			da[9] = da[6];
			da[6] = e->dbl;
			// 3<->12
			e->dbl = da[12];
			da[12] = da[3];
			da[3] = e->dbl;
			// 7<->13
			e->dbl = da[13];
			da[13] = da[7];
			da[7] = e->dbl;
			//11<->14
			e->dbl = da[14];
			da[14] = da[11];
			da[11] = e->dbl;
		}
	}
	return 0;
}

/*
* 	матричное умножение 3x3, 4x4
*/
int wl0_MADD(nicenv_t * e){
	if (e->help) {
		printf("\tmat1 mat2 MADD -> mat3 /*Matrix addition*/\n");
		return 0;
	}
	char name[MAX_NAME_LENGTH];
	double * da, * db;
	_var_t * v1, *v2, *d;
	int i;
	if (num_cells(e,2)){
		if ( gt1(e) == MAT && gt2(e) == MAT ){
			v1 = (_var_t *) gs2(e);
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e);
			db = (double *) v2->first_element;
			drop1(e); drop1(e);
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			e->dbl = da[0] + db[0];
			push_dbl_to_stack(e);
			e->dbl = da[1] + db[1];
			push_dbl_to_stack(e);
			e->dbl = da[2] + db[2];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_VEC(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[4] + db[4];
			push_dbl_to_stack(e);
			e->dbl = da[5] + db[5];
			push_dbl_to_stack(e);
			e->dbl = da[6] + db[6];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_VEC(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[8] + db[8];
			push_dbl_to_stack(e);
			e->dbl = da[9] + db[9];
			push_dbl_to_stack(e);
			e->dbl = da[10] + db[10];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_VEC(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			// создаем матрицу
			generate_unique_name (e, "mm", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			wl0_MAT(e);
			wl0_ADEL(e);
		} else
		if ( gt1(e) == MATX && gt2(e) == MATX){
			v1 = (_var_t *) gs2(e);
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e);
			db = (double *) v2->first_element;
			drop1(e); drop1(e);
			// индексы матрицы
			// 	0  1  2  3 
			//  4  5  6  7
			//  8  9 10 11
			// 12 13 14 15
			// расчитываем компоненты матрицы
			e->dbl = da[0] + db[0];
			push_dbl_to_stack(e);
			e->dbl = da[1] + db[1];
			push_dbl_to_stack(e);
			e->dbl = da[2] + db[2];
			push_dbl_to_stack(e);
			e->dbl = da[3] + db[3];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			// расчитываем компоненты матрицы
			e->dbl = da[4] + db[4];
			push_dbl_to_stack(e);
			e->dbl = da[5] + db[5];
			push_dbl_to_stack(e);
			e->dbl = da[6] + db[6];
			push_dbl_to_stack(e);
			e->dbl = da[7] + db[7];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[8] + db[8];
			push_dbl_to_stack(e);
			e->dbl = da[9] + db[9];
			push_dbl_to_stack(e);
			e->dbl = da[10] + db[10];
			push_dbl_to_stack(e);
			e->dbl = da[11] + db[11];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			e->dbl = da[12] + db[12];
			push_dbl_to_stack(e);
			e->dbl = da[13] + db[13];
			push_dbl_to_stack(e);
			e->dbl = da[14] + db[14];
			push_dbl_to_stack(e);
			e->dbl = da[15] + db[15];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			wl0_UN(e);
			wl0_PDOT(e);
			wl0_AVAR(e); // делаем ее автоматической чтобы удалить после инициализации
			// создаем матрицу
			generate_unique_name (e, "mx", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			wl0_MATX(e);
			wl0_ADEL(e);
		}
	}
	return 0;
}


/*
* 	Векторное умножение
*/
int wl0_VMUL(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec2 VMUL -> [vec1,vec2] /*Vector multiplication*/\n");
		return 0;
	}
	char name[MAX_NAME_LENGTH];
	double * da, * db;
	_var_t * v1, *v2, *d;
	if (num_cells(e,2)){
		if ( gt1(e) == VEC && gt2(e) == VEC){
			v1 = (_var_t *) gs2(e);
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e);
			db = (double *) v2->first_element;
			drop1(e); drop1(e);
			// расчитываем компоненты вектора
			e->dbl = da[1] * db[2] - da[2] * db[1];
			push_dbl_to_stack(e);
			e->dbl = -da[0] * db[2] + da[2] * db[0];
			push_dbl_to_stack(e);
			e->dbl = da[0] * db[1] - da[1] * db[0];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			generate_unique_name (e, "vec", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			// создаем вектор
			wl0_VEC(e);
		}
	}
	return 0;
}

int wl0_VADD(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec2 VADD -> vec1+vec2 /*Vector addition and creation*/\n");
		printf("\tdot vec VADD -> dot' /*New dot creation at new place*/\n");
		return 0;
	}
	char name[MAX_NAME_LENGTH];
	double * da, * db;
	_var_t * v1, *v2, *d;
	if (num_cells(e,2)){
		if ( gt1(e) == VEC && (gt2(e) == VEC || gt2(e) == DOT ) ){
			v1 = (_var_t *) gs2(e);
			da = (double *) v1->first_element;
			v2 = (_var_t *) gs1(e);
			db = (double *) v2->first_element;
			drop1(e); drop1(e);
			// расчитываем компоненты вектора
			e->dbl = da[0] + db[0];
			push_dbl_to_stack(e);
			e->dbl = da[1] + db[1];
			push_dbl_to_stack(e);
			e->dbl = da[2] + db[2];
			push_dbl_to_stack(e);
			//придумываем имя переменной
			generate_unique_name (e, "vd", name);
			d = new_var(e, name, UDEF);
			push_var_ptr_to_stack(e, d);
			// создаем вектор или точку
			make_a_dot(e,v1->type.type_index);
		}
	}
	return 0;
}
/* 
* 	Инверсия вектора или матрицы
*/
int wl0_INV(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 INV -> -vec1 /*Vector inversion*/\n");
		printf("\tmatr1 INV -> matr' /*Reverting matrix*/\n");
		return 0;
	}
	double * da;
	_var_t * v1;
	if (num_cells(e,1)){
		if ( gt1(e) == VEC ){
			v1 = (_var_t *) gs1(e);
			da = (double *) v1->first_element;
			// расчитываем компоненты вектора
			da[0] *= -1.0;
			da[1] *= -1.0;
			da[2] *= -1.0;
		}
	}
	return 0;
}

int wl0_VSUB(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec2 VSUB -> vec1-vec2 /*Vector subtraction and creation*/\n");
		return 0;
	}
	// копируем второй вектор чтобы потом его инвертировать обратно
	wl0_DUP(e);
	push_int_number(e,1);
	wl0_FLIP(e);
	wl0_INV(e);
	wl0_VADD(e);
	push_int_number(e,1);
	wl0_FLOP(e);
	wl0_INV(e);
	drop1(e);
	return 0;
}
/*
	ANGL	- 	определяет угол между векторами в радианах
	GRAD	- 	переводит радианы в градусы
	RAD 	-   переводит из градусов в радианы.
*/
int wl0_ANGL(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec1 ANGL -> d /*Calculate the angle in radians between vec1 and vec2*/\n");
		return 0;
	}
	double d1 = 0.0;
	if(num_cells(e,2) && gt1(e) == VEC && gt2(e) == VEC){
		wl0_DUP(e);
		wl0_LEN(e); // вычислили длину вектора
		wl0_SWAP(e); // поменяли длину с вектором
		push_int_number (e,2);
		wl0_FLIP(e); // закинули вектор и его длину в доп стек
		wl0_DUP(e); 
		wl0_LEN(e); // вычислили длину второго вектора
		push_int_number (e,1);
		wl0_FLOP(e); // вернули длину первого вектора в стек
		wl0_MUL(e); // произведение длин
		wl0_DUP(e); 
		pop_double(e);
		d1 = e->dbl; // проверяем не равно ли произведение нулю
		//printf("dist = %lf\n", d1);
		if (d1 != 0){
			push_int_number(e,1);
			wl0_FLOP(e); // возвращаем первый вектор в стек
			wl0_ROT(e); // циклическая перестановка в стеке
			wl0_SMUL(e);
			wl0_SWAP(e);
			wl0_DIV(e);
			wl0_ACOS(e);
		}else{
			printf("angl: zero vector lenght, skipping\n");
		}
	}
	return 0;
}
int wl0_GRAD(nicenv_t * e){
	if (e->help) {
		printf("\trad GRAD -> degr /*Translate radians to degrees*/\n");
		return 0;
	}
	double d1 = 0.0;
	if(num_cells(e,1) && ( gt1(e) == DUBL || gt1(e) == INT)){
		e->dbl = (4*atan(1.0)) / 180.0;
		push_dbl_to_stack(e);
		wl0_DIV(e);
	}
	return 0;
}

int wl0_RAD(nicenv_t * e){
	if (e->help) {
		printf("\tdegr RAD -> rad /*Translate degrees to radians*/\n");
		return 0;
	}
	double d1 = 0.0;
	if(num_cells(e,1) && ( gt1(e) == DUBL || gt1(e) == INT)){
		e->dbl = (4*atan(1.0)) / 180.0;
		push_dbl_to_stack(e);
		wl0_MUL(e);
	}

	return 0;
}
/*Создает матрицу трансляции из вектора.*/
int wl0_TMAT(nicenv_t * e){
	if (e->help) {
		printf("\tvec mat TMAT -> mat' /*Creates translation matrix 4x4*/\n");
		return 0;
	} 
	_var_t * v;
	_var_t * m;
	double * da;
	make_same_name_udef_var(e);
	if (num_cells(e,2) && gt2(e) == VEC && gt1(e) == UDEF){
		push_int_number(e,1);
		wl0_FLIP(e); // сохраняем переменную
		// извлекаем вектор
		v = (_var_t *) gs1(e);
		da = (double *) v->first_element;
		drop1(e);

		e->dbl = 1.0;
		push_dbl_to_stack(e);
		e->dbl = 0.0;
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		e->dbl = da[0];
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		e->dbl = 0.0;
		push_dbl_to_stack(e);
		e->dbl = 1.0;
		push_dbl_to_stack(e);
		e->dbl = 0.0;
		push_dbl_to_stack(e);
		e->dbl = da[1];
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		e->dbl = 0.0;
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		e->dbl = 1.0;
		push_dbl_to_stack(e);
		e->dbl = da[2];
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		e->dbl = 0.0;
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		e->dbl = 1.0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		push_int_number(e,1);
		wl0_FLOP(e);

		make_a_mat(e, MATX);
		wl0_ADEL(e);
		
	}
	return 0;
}
/*Создает матрицу вращения из углов \phi, \theta, \psi.*/
int wl0_RXYZ(nicenv_t * e){
	if (e->help) {
		printf("\tphi theta psi mn RXYZ -> mn' /*Creates rotation matrix 4x4*/\n");
		return 0;
	}
	double angl[3] = {0.0, 0.0, 0.0};
	double cos_phi, sin_phi;
	double cos_tet, sin_tet;
	double cos_psi, sin_psi;
	int i;
	double gtor = 4*atan(1.0)/180.0;
	make_same_name_udef_var(e);
	if (num_cells(e,4) && gt1(e) == UDEF){
		push_int_number(e,1);
		wl0_FLIP(e); // сохраняем переменную
		// извлекаем psi это может быть DUBL или VAR_DUBL
		for (i=2; i>=0; i--){
			if (gt1(e) == DUBL){
				pop_double(e);
				angl[i] = e->dbl;
			}else if (gt1(e) == VAR_DUBL ){
				wl0_VALU(e);
				pop_double(e);
				angl[i] = e->dbl;
			}else { return 0; }
		}
		// конструируем матрицу
		//  c(ψ)c(θ)				c(θ)s(ψ)				−s(θ)		0
		//	c(ψ)s(ϕ)s(θ)−c(ϕ)s(ψ)	c(ϕ)c(ψ)+s(ϕ)s(ψ)s(θ)	c(θ)s(ϕ)	0
		//	s(ϕ)s(ψ)+c(ϕ)c(ψ)s(θ)   c(ϕ)s(ψ)s(θ)−c(ψ)s(ϕ)	c(ϕ)c(θ)	0
		//	0						0						0			1			
		cos_phi = cos(gtor * angl[0]);
		sin_phi = sin(gtor * angl[0]);

		cos_tet = cos(gtor * angl[1]);
		sin_tet = sin(gtor * angl[1]);		

		cos_psi = cos(gtor * angl[2]);
		sin_psi = sin(gtor * angl[2]);		

		e->dbl = cos_psi*cos_tet;
		push_dbl_to_stack(e);
		e->dbl = cos_tet*sin_psi;
		push_dbl_to_stack(e);
		e->dbl = - sin_tet;
		push_dbl_to_stack(e);
		e->dbl = 0.0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		e->dbl = cos_psi*sin_phi*sin_tet - cos_phi*sin_psi;
		push_dbl_to_stack(e);
		e->dbl = cos_phi*cos_psi + sin_phi*sin_psi*sin_tet ;
		push_dbl_to_stack(e);
		e->dbl = cos_tet*sin_phi;
		push_dbl_to_stack(e);
		e->dbl = 0.0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		e->dbl = sin_phi*sin_psi + cos_phi*cos_psi*sin_tet;
		push_dbl_to_stack(e);
		e->dbl = cos_phi*sin_psi*sin_tet - cos_psi*sin_phi;
		push_dbl_to_stack(e);
		e->dbl = cos_phi * cos_tet;
		push_dbl_to_stack(e);
		e->dbl = 0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		e->dbl = 0.0;
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		e->dbl = 1.0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_PDOT(e);
		wl0_AVAR(e);

		push_int_number(e,1);
		wl0_FLOP(e);

		make_a_mat(e, MATX);
		wl0_ADEL(e);
	}
	return 0;
}
/*Создает единичную матрицу*/
int wl0_UMAT(nicenv_t * e){
	if (e->help) {
		printf("\tmat_name UMAT -> mat_var /*Create unit matrix*/\n");
		return 0;
	}
	make_same_name_udef_var(e);
	if (num_cells(e,1) && gt1(e) == UDEF){
		push_int_number(e,1);
		wl0_FLIP(e);

		e->dbl = 1.0;
		push_dbl_to_stack(e);
		e->dbl = 0.0;
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_VEC(e);

		e->dbl = 0.0;
		push_dbl_to_stack(e);
		e->dbl = 1.0;
		push_dbl_to_stack(e);
		e->dbl = 0.0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_VEC(e);

		e->dbl = 0.0;
		push_dbl_to_stack(e);
		push_dbl_to_stack(e);
		e->dbl = 1.0;
		push_dbl_to_stack(e);
		wl0_UN(e);
		wl0_VEC(e);

		push_int_number(e,1);
		wl0_FLOP(e);

		make_a_mat(e, MAT);
	} 
	return 0;
}
/* Создает матрицу 3x3 из 3 векторов*/
int wl0_MAT(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec2 vec3 mat_name MAT -> mat_var /*Create MAT object*/\n");
		return 0;
	}
	make_a_mat(e, MAT);
	return 0;
}

int wl0_MATX(nicenv_t * e){
	if (e->help) {
		printf("\tvec1 vec2 vec3 vec4 matx_name MATX -> matx_var /*Create MATX object*/\n");
		return 0;
	}
	_var_t * v;
	if (num_cells(e,1) && gt1(e) == MAT){
		v = (_var_t *) gs1(e);
		v->num_element = 16;
		v->type.type_index = MATX;
		st1(e, MATX);
	}else {
		make_a_mat(e, MATX);
	}
	return 0;
}


/* добавить объект в базу данных */
int wl0_DADD(nicenv_t * e){
	if (e->help) {
		printf("\tvar mdb DADD -> /* Add variable to the database */\n");
		return 0;
	}
	int tp = UDEF;
	_var_t * db;
	_var_t * v;
	if(num_cells(e,2) && gt1(e) == DBASE){
		db = (_var_t *) gs1(e);
		v  = (_var_t *) gs2(e);
		// Проверяем базу данных
		if (1 == is_db_exist(e, db)) {
			add_var_to_db(e, db, v);
			drop1(e); drop1(e);
		}else if (0 == is_db_exist(e, db)) {
			//создаем базу данных
			create_db(e, db);
			add_var_to_db(e, db, v);
			drop1(e); drop1(e);
		}// иначе ошибка
	}
	return 0;
}

/* удалить объект из базы данных */
int wl0_DDEL(nicenv_t * e){
	return 0;
}

/* найти объект в базе данных */
int wl0_DFIN(nicenv_t * e){
	return 0;
}

/*
*	Проверяет, существует ли переменная в кольце
*	возвращается ссылка, иначе NULL. 
*	При создании переменных простых типов нужно заменять тип переменной
* 	VAR_BYTE, //  - указатель переменной типа байт
*	VAR_LETR, //  - 8 битный символ
*	VAR_CHAR, //  - 16 битный символ
*	VAR_INT,  //  - переменная типа инт
*	VAR_DUBL, //  - переменная типа дабл
* 	Для остальных сложных типов выделяется структура в памяти. 
* 	Это нужно для правильного отображения содержимого типа ячеек стека,
* 	потому, что например, при загрузке переменной типа INT неправильно интерпретируется
* 	содержимое ячейки, она принимается за целый тип, вместо переменной
*/
int is_var(nicenv_t * e, _var_t * v)
{
	_var_t * next= e->vars_root.first;
	do{
		if ( v == next){
			//printf("It's Var [%s]\n", next->name);
			return 1;
		}
		// защита от битого кольца
		if (next->prev != NULL) next = next->prev;
		else return 0;
	}while (next != e->vars_root.first);
	return 0;
}

/*
* 	функция возвращает 1 если в стеке содержится больше минимального числа
*	элементов. Если меньше - то возвращает 0.
*/
int num_cells(nicenv_t * e, int nmin){
	int i = 0, j = 0, d = 0;
	while(i<e->sp1){
		if (e->T1[i] != DUBL) {
			j++;
		}else {
			if (d == 0){
				d = 1;
			} else {
				d = 0;
				j++;
			}
		}
		if (j>=nmin) return 1;

		i++;
	}
	return 0;
}

int get_cell(nicenv_t * e, void ** s, void ** s1, long * t, long * t1){
	if (e->sp1 > 0){
		*t = gt1(e);
		*s = gs1(e);
		dsp1(e);
		if (*t == DUBL && e->sp1 > 0){
			*t1 = gt1(e);
			*s1 = gs1(e);
			dsp1(e);
			return 2; //double
		}
		return 1; // single
	}
	return 0; // nothing 
}

int put_cell(nicenv_t * e, void * s, void * s1, long t, long t1){
	int r=0;
	if (e->sp1 < MAX_STACK_SIZE){
		if (t == DUBL && e->sp1 < MAX_STACK_SIZE){
			st(e, t1);
			ss(e, s1);
			isp1(e);
			r++; //double
		}
		st(e, t);
		ss(e, s);
		isp1(e);
		r++;
		return r;
	}
	return 0; // nothing
}

int wl0_SWAP(nicenv_t * e){
	if (e->help) {
		printf("\ta b SWAP -> b a\n");
		return 0;
	}
	void * v, * v1, * v2, *v3;
	long t, t1, t2, t3;
	// a b SWAP -> b a
	if (num_cells(e,2)){
		get_cell(e, &v, &v1, &t, &t1);
		get_cell(e, &v2, &v3, &t2, &t3);
		
		put_cell(e, v, v1, t, t1);
		put_cell(e, v2, v3, t2, t3);
	}else
	{
		printf("Warning: Not enouph elements to SWAP\n");
	}
	
	return 0;
}

int wl0_ROT(nicenv_t * e){
	if (e->help) {
		printf("\ta b c ROT -> c a b\n");
		return 0;
	}
	void * v, * v1, * v2, *v3, *v4, *v5;
	long t, t1, t2, t3, t4, t5;
	// a b c ROT -> c a b
	if (num_cells(e,3)){
		get_cell(e, &v, &v1, &t, &t1);
		get_cell(e, &v2, &v3, &t2, &t3);
		get_cell(e, &v4, &v5, &t4, &t5);

		put_cell(e, v2, v3, t2, t3);
		put_cell(e, v, v1, t, t1);
		put_cell(e, v4, v5, t4, t5);
	}else
	{
		printf("Warning: Not enouph elements to ROT\n");
	}

	return 0;
}


/*Creation of variable of any type.*/
int wl0_VAR(nicenv_t * e)
{
	if (e->help) {
		printf("\tv <type> VAR -> vt\n");
		return 0;
	}
	//v <type> VAR -> vt
	_var_t * v = NULL;
	_var_t * vn = NULL;
	if ( num_cells(e,2) && e->sp1 < MAX_STACK_SIZE ){
		push_int_number(e,1);
		wl0_FLIP(e);
		make_same_name_udef_var(e);
		push_int_number(e,1);
		wl0_FLOP(e);
		v = (_var_t * ) gs2(e);
		if ( gt1(e) == TYPE && is_var(e, v) && (v->type.type_index == UDEF)){
			switch((long) gs1(e)){
				case BYTE:
					v->type.type_index = VAR_BYTE;
					break;
				case LETR:
					v->type.type_index = VAR_LETR;
					break;
				case CHAR:
					v->type.type_index = VAR_CHAR;
					break;
				case INT:
					v->type.type_index = VAR_INT;
					break;
				case DUBL:
					v->type.type_index = VAR_DUBL;
					break;
				default :
				{
					v->type.type_index = (long) gs1(e);
					//printf("default type: \n");
				}break;
			}
			st2(e, v->type.type_index);
			drop1(e);
		}
	}	
	return 0;
}

/* Инициализирует переменную с помощью строки.*/
int wl0_STRI(nicenv_t * e){
	if (e->help) {
		printf("\t<string> su STRI -> si\n");
		return 0;
	}
	_var_t * str1, * str2;

	if(num_cells(e,2)){
		make_same_name_udef_var(e);
		if ((gt1(e) == UDEF) && (gt2(e) == STRI))
		{
			str1 = (_var_t *) gs2(e);
			str2 = (_var_t *) gs1(e);
			copy_var(str1, str2); // str1 -> str2
			st1(e, STRI);
			wl0_SWAP(e);
			drop1(e);
			del_var(e, str1);
			//fix quoting \"
			fix_string_quoting(str2);
		}else{
			printf("Error: variable should be of UDEF type.\n");	
		}

	}else{
		printf("Warning: Not enouph of arguments to create the string.\n");
	}
	return 0;
}

int pri_string(nicenv_t * e, _var_t * str){
	if (((_var_t *) str->first_element != NULL) )
		printf("%s\n", (char*)str->first_element );
	else
		printf("NULL\n");
	return 0;
}
/*Print N variables*/
int wl0_PRIN(nicenv_t * e){
	if (e->help) {
		printf("\tn PRIN -> Print n top stack values.\n");
		return 0;
	}
	int i;
	long n;
	if (num_cells(e,2)){
		if(gt1(e) == INT){
			pop_int(e);
			n = e->lint;
			for(i=0; i<n; i++){
				wl0_PR(e);
			}
		}
	}
	return 0;
}
/* Выводит содержимое ячейки для простого типа*/
int wl0_VIEW(nicenv_t * e){
	if (e->help) {
		printf("\tPrint the top stack value. Don't drop it from the stack.\n");
		return 0;
	}
	int i = e->sp1-1, j;
	long ltmp;
	long dptr[2]; 
	char tp[MAX_NAME_LENGTH];
	_var_t * t_v;
	_var_t * t;
	_var_t * s;
	_newtype_t * ntp = NULL;
	if (num_cells(e,1)){
		// проверяем является ли переменная линком
		if ( is_var(e, (_var_t*) e->S1[i]) ){
			_var_t * v = (_var_t*) e->S1[i];
			if (v->link) { 
				print_type(e, (long)e->T1[i], tp);
				printf("%s = LINK -> %s : %s\n", v->name, (char*) v->first_element, tp );
				return 0;
			}
		}
		switch(gt1(e))
		{
			case VOID: {	
				printf("VOID\n");
			}break;
			case UDEF: {
				//printf("%s  ", i, ((_var_t*) gs1(e))->name );
				printf("UDEF\n");
			}break;
			case BYTE: {
				ltmp = (long) e->S1[i];
				printf("%d\n", (unsigned char) ltmp );
			}break;
			case LETR: 
			case CHAR:
			{
				ltmp = (long) e->S1[i];
				printf("%c\n", (char) ltmp );
			}break;
			case INT: {
				printf("%ld\n", (long) e->S1[i] );
			}break;
			case DUBL: {
				dptr[1] = (long) e->S1[i];
				i--;
				dptr[0] = (long) e->S1[i];
				printf("%e\n", *((double*) dptr) );
			}break;
			case CODE: {
				printf("%s\n", ((_word_t *) e->S1[i])->name );
			}break;
			case IDX: 
			case DIM:
			{
				printf("%ld\n", (long) e->S1[i] ); // was (unsigned long)
			}break;
			case ARRA: {
				// Надо переделать, распечатывать массивы
				printf("%s\n", ((_var_t *) e->S1[i])->name );
			}break;
			case DBASE:
			case STRI: 
			case FORM:{
				if (((_var_t *) e->S1[i])->first_element != NULL) 
					printf("%s\n", (char*) ((_var_t *) e->S1[i])->first_element );
				else
					printf("NULL\n");
			}break;
			case TEXT: {
				t = (_var_t *) e->S1[i];
				for (j=0; j<t->num_element; j++){
					s = ((_var_t**)(t->first_element))[j];
#if (DEBUG_LEVEL == 5)					
					printf("Index: %d, pointer %p", j, s);
#endif					
					pri_string(e, s);
				}
			}break; 
			case STRU: { // really it's not variable, but new type.
				// Доделать вывод струкрут
				printf("%s\n", ((_var_t *) e->S1[i])->name );
			}break;
			case DOT: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = DOT(%lf %lf %lf)\n", t_v->name, fe[0], fe[1], fe[2] );
				}
				else
					printf("NULL\n");
			}break;
			case PDOT: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = PDOT(%lf %lf %lf %lf)\n", t_v->name, fe[0], fe[1], fe[2], fe[3] );
				}
				else
					printf("NULL\n");
			}break;
			case VEC: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = VEC(%lf %lf %lf)\n", t_v->name, fe[0], fe[1], fe[2] );
				}
				else
					printf("NULL\n");
			}break;
			case RVEC: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = RVEC(%lf %lf %lf %lf)\n", t_v->name, fe[0], fe[1], fe[2], fe[3] );
				}
				else
					printf("NULL\n");
			}break;
			case LINE: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = LINE(%lf %lf %lf %lf)(%lf %lf %lf %lf) \n", t_v->name, fe[0], fe[1], fe[2], fe[3],
					fe[4], fe[5], fe[6], fe[7] );
				}
				else
					printf("NULL\n");
			}break;
			case ARC: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					void * rv = ((void **) (t_v->first_element))[0];
					void * rn = ((void **) (t_v->first_element))[1];
					double * rvfe = (double*) ((_var_t*) rv)->first_element;
					double * rnfe = (double*) ((_var_t*) rn)->first_element;
					printf("%s = ARC(%s = RVEC(%lf %lf %lf %lf), %s = RVEC(%lf %lf %lf %lf))\n", 
						t_v->name, ((_var_t*) rv)->name, rvfe[0], rvfe[1], rvfe[2], rvfe[3],
						((_var_t*) rn)->name, rnfe[0], rnfe[1], rnfe[2], rnfe[3]);
				}
				else
					printf("NULL\n");
			}break;
			case PATH: {
				if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					void ** fe = (void **) t_v->first_element;
					int ll;
					//printf("num_element = %d\n", t_v->num_element);
					printf("%s = PATH(", t_v->name);
					for (ll=0; ll<t_v->num_element; ll++){
						printf("%s", ((_var_t*)(fe[ll]))->name );
						if (ll < (t_v->num_element-1)) printf(",");
						else printf(")");
					}
					if (t_v->data[0] == 0) printf(" OPENED\n");
					else printf(" CLOSED\n");
				}
				else
					printf("NULL\n");
			}break;
			case MAT: {
					if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = MAT (%lf %lf %lf)\n\t (%lf %lf %lf)\n\t (%lf %lf %lf) \n", t_v->name, 
					fe[0], fe[1], fe[2], 
					fe[4], fe[5], fe[6],
					fe[8], fe[9], fe[10] );
				}
				else
					printf("NULL\n");
			}break;
			case MATX: {
					if (((_var_t *) e->S1[i])->first_element != NULL) {
					t_v = (_var_t *) e->S1[i];
					double * fe = (double*) t_v->first_element;
					printf("%s = MATX(%lf %lf %lf %lf)\n\t (%lf %lf %lf %lf)\n\t (%lf %lf %lf %lf) \n\t (%lf %lf %lf %lf) \n", 
					t_v->name, 
					fe[0], fe[1], fe[2], fe[3], 
					fe[4], fe[5], fe[6], fe[7],
					fe[8], fe[9], fe[10], fe[11],
					fe[12], fe[13], fe[14], fe[15]);
				}
				else
					printf("NULL\n");
			}break;
			case TYPE: {
				print_type(e, (long)e->S1[i], tp);
				printf("%s\n", tp );
			}break;
			case VAR_BYTE: {
				t_v = (_var_t *) e->S1[i];
				printf("%s = %d\n", t_v->name , t_v->data[0]);
			}break;
			case VAR_LETR:{
				t_v = (_var_t *) e->S1[i];
				printf("%s = '%c'\n", t_v->name , t_v->data[0]);
			}break;
			case VAR_CHAR:{
				t_v = (_var_t *) e->S1[i];
				printf("%s = %d\n", t_v->name , t_v->data[0]);
			}break;
			case VAR_INT:{
				t_v = (_var_t *) e->S1[i];
				printf("%s = %ld\n", t_v->name , *((long *)t_v->data));
			}break;
			case VAR_DUBL:
			{
				// Доделать потом
				// Значения этих переменных храняться в data[8]
				t_v = (_var_t *) e->S1[i];
				printf("%s = %lf\n", t_v->name, *((double *)t_v->data) );
			}break;

			default: {
				ntp = get_type_by_index(e, gt1(e));
				if ( ntp!= NULL ){
					t_v = (_var_t *) e->S1[i];
					printf("%s = %s\n", t_v->name, ntp->name );
				}
				else
					printf("UNDEFINED \n");
			}
		}
	}
	return 0;
}

/* Выводит содержимое простого типа, но не сбрасывает его с вершины стека*/
int wl0_PR(nicenv_t * e){
	if (e->help) {
		printf("\tPrint the top stack value and drop it from stack\n");
		return 0;
	}
	if (num_cells(e,1)){
		wl0_VIEW(e);
		drop1(e);
	}
	return 0;
}

int flip_cell(nicenv_t * e){
	if (e->sp1>0 && e->sp2 < MAX_STACK_SIZE){
		if (gt1(e) !=DUBL){
			ss_aux(e, gs1(e));
			st_aux(e, gt1(e));
			dsp1(e);
			isp2(e);//e->sp2++;
		}else{ //Double is keeping in flipped state in S2
			ss_aux(e, gs1(e));
			st_aux(e, gt1(e));
			dsp1(e);
			isp2(e);//e->sp2++;
			ss_aux(e, gs1(e));
			st_aux(e, gt1(e));
			dsp1(e);
			isp2(e);//e->sp2++;
		}
	}
	return 0;
}

int flop_cell(nicenv_t * e){
	if (e->sp2>0 && e->sp1 < MAX_STACK_SIZE){
		if (gt1_aux(e) != DUBL){
			ss(e, gs1_aux(e));
			st(e, gt1_aux(e));
			dsp2(e);
			isp1(e);
		}else{ //Double is keeping in flipped state in S2
			ss(e, gs1_aux(e));
			st(e, gt1_aux(e));
			dsp2(e);
			isp1(e);
			ss(e, gs1_aux(e));
			st(e, gt1_aux(e));
			dsp2(e);
			isp1(e);
		}
	}
	return 0;
}

int wl0_FLIP(nicenv_t * e){
	if (e->help) {
		printf("\tn FLIP -> Move n top elements from main stack into auxiliary stack in reverse order.\n");
		return 0;
	}
	int i,n;
	if (num_cells(e,2)){
		if(gt1(e) == INT){
			pop_int(e);
			n = e->lint;
			for(i=0; i<n; i++){
				flip_cell(e);
			}
		}	
	}
	return 0;
}

int wl0_FLOP(nicenv_t * e){
	if (e->help) {
		printf("\tn FLOP -> Move back n top elements from auxiliary stack into main stack in reverse order.\n");
		return 0;
	}
	int i,n;
	if (num_cells(e,1)){
		if(gt1(e) == INT){
			pop_int(e);
			n = e->lint;
			for(i=0; i<n; i++){
				flop_cell(e);
			}
		}	
	}
	return 0;
}

int wl0_PUSH(nicenv_t * e){
	if (e->help) {
		printf("\tPUSH -> Move 1 top element from main stack into auxiliary stack.\n");
		return 0;
	}
	int i,n;
	if (num_cells(e,1)){
		flip_cell(e);
	}
	return 0;
}

int wl0_POP(nicenv_t * e){
	if (e->help) {
		printf("\tPOP -> Move 1 top element from auxiliary stack into main stack.\n");
		return 0;
	}
	flop_cell(e);
	return 0;
}
/* Дуплицируем сложную переменную*/
int wl0_DUPV(nicenv_t * e){
	if (e->help) {
		printf("\tv DUPS -> v v\'\n");
		return 0;
	}
	printf("DUPV is not implemented\n");
	return 0;
}

/* Дуплицируем строку, иначе ничего не делаем*/
int wl0_DUPS(nicenv_t * e)
{
	if (e->help) {
		printf("\tstr DUPS -> str str\'\n");
		return 0;
	}
	_var_t * s, * d;
	char name[MAX_NAME_LENGTH];
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE){
		if (gt1(e) == STRI ){
			s = (_var_t * ) gs1(e);
			generate_unique_name (e, "c", name);
			d = new_var(e, name, gt1(e));
			fork_var(s,d); //source, destination
			push_var_ptr_to_stack(e, d);
		}
	}
	return 0;
}

int wl0_DUP(nicenv_t * e)
{
	if (e->help) {
		printf("\tx DUP -> x x\n");
		return 0;
	}
	//printf("Duplicate function template: %d\n", e->sp1);
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE){
		if (gt1(e) != DUBL){
			ss(e, gs1(e));
			st(e, gt1(e));
			isp1(e);
		}else {
			ss(e, gs2(e));
			st(e, gt2(e));
			isp1(e);
			ss(e, gs2(e));
			st(e, gt2(e));
			isp1(e);
		}
	}
	return 0;
}

int wl0_ADD(nicenv_t * e)
{
	long a, b;
	long dptr[2];
	double da, db;
	if (e->help) {
		printf("\ttext str ADD -> text\'\n");
		printf("\tstr1 str2 ADD -> str1\'\n");
		printf("\tn1 n2 ADD -> <n1+n2>\n");
		printf("\tpath1 path2 ADD -> path1'\n");
		return 0;
	}
	_var_t * tx, *st;
	//printf("ADD function template: %d\n", e->sp1);
	if (num_cells(e,2)){
		if ( gt2(e) == TEXT && gt1(e) == STRI){
			tx = (_var_t *) gs2(e);
			st = (_var_t *) gs1(e);

			add_string_to_text(e, tx, st, -1);
			drop1(e);
			return 0;
		}
		if ( gt1(e) == STRI && gt2(e) == STRI){
			tx = (_var_t *) gs2(e);
			st = (_var_t *) gs1(e);
			
			add_string_to_string(e, tx, st);
			drop1(e);
			return 0;
		}
		if ( gt1(e) == INT && gt2(e) == INT){
			a = (long) gs1(e);
			b = (long) gs2(e);
			drop1(e); drop1(e);
			push_int_number(e, a+b);
			return 0;
		}
		if ( gt1(e) == INT && gt2(e) == DUBL){
			a = (long) gs1(e);
			drop1(e);
			dptr[1] = (long) gs1(e);
			dsp1(e);
			dptr[0] = (long) gs1(e);
			dsp1(e);
			da = *((double*) dptr);
			da += a;
			e->dbl = da;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == DUBL && gt3(e) == INT){
			dptr[1] = (long) gs1(e);
			dsp1(e);
			dptr[0] = (long) gs1(e);
			dsp1(e);
			da = *((double*) dptr);
			a = (long) gs1(e);
			drop1(e);

			da += a;
			e->dbl = da;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == DUBL && gt3(e) == DUBL){
			dptr[1] = (long) gs1(e);
			dsp1(e);
			dptr[0] = (long) gs1(e);
			dsp1(e);
			da = *((double*) dptr);
			dptr[1] = (long) gs1(e);
			dsp1(e);
			dptr[0] = (long) gs1(e);
			dsp1(e);
			db = *((double*) dptr);
			
			da += db;
			e->dbl = da;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == PATH && gt2(e) == PATH){
			tx = (_var_t *) gs2(e);
			st = (_var_t *) gs1(e);
			add_path_to_path (e, tx, st);
			drop1(e);
		}

	}
	return 0;
}
int wl0_SUB(nicenv_t * e)
{
	if (e->help) {
		printf("\tn1 n2 SUB -> <n1-n2>\n");
		return 0;
	}
	long a, b;
	_var_t * tx, *st;
	if (num_cells(e,2) && e->sp1 < MAX_STACK_SIZE ){
		if ( gt1(e) == INT && gt2(e) == INT){
			a = (long) gs1(e);
			b = (long) gs2(e);
			drop1(e); drop1(e);
			push_int_number(e, b-a);
			return 0;
		}else {
			e->dbl = -1.0;
			push_dbl_to_stack(e);
			wl0_MUL(e);
			wl0_ADD(e);
		}
	}
	return 0;
}
int wl0_NEGA(nicenv_t * e){
	if (e->help) {
		printf("\tn NEGA -> -n\n");
		return 0;
	}
	long a;
	_var_t * tx, *st;
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE ){
		if ( gt1(e) == INT){
			a = (long) gs1(e);
			drop1(e);
			push_int_number(e, -a);
			return 0;
		}else {
			e->dbl = -1.0;
			push_dbl_to_stack(e);
			wl0_MUL(e);
		}
	}
	return 0;	
}
/*Multiply 2 numbers, if one number is double - it becomes double*/
int wl0_MUL(nicenv_t * e)
{
	if (e->help) {
		printf("\tn1 n2 MUL -> <n1*n2>\n");
		printf("\tcode n MUL -> loop code n times\n" );
		return 0;
	}
	long a, b, i;
	long dptr[2];
	double da, db;
	_var_t * tx, *st;
	_word_t * w;
	if (num_cells(e,2)){
		if ( gt1(e) == INT && gt2(e) == INT){
			pop_int(e);
			a = e->lint;
			pop_int(e);
			b = e->lint;
			push_int_number(e, a*b);
			return 0;
		}
		if ( gt1(e) == INT && gt2(e) == DUBL){
			pop_int(e);
			a = e->lint;
			pop_double(e);
			e->dbl *= a;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == DUBL && gt3(e) == INT){
			pop_double(e);
			da = e->dbl;
			pop_int(e);
			a = e->lint;
			da *= a;
			e->dbl = da;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == DUBL && gt3(e) == DUBL){
			pop_double(e);
			da = e->dbl;
			pop_double(e);
			e->dbl *= da;
			push_dbl_to_stack(e);
			return 0;
		}
		// code loop
		if ( gt1(e) == INT && gt2(e) == CODE){
			pop_int(e);
			a = abs(e->lint);
			w = (_word_t *) gs1(e);
			
			st1(e, 0);
			ss1(e, 0);
			dsp1(e);
			for (i=0; i<a; i++){
				(*(w->code))(e);
			}
			return 0;
		}

	}
	return 0;
}

/*Divide 2 numbers, if one number is double - it becomes double*/
int wl0_DIV(nicenv_t * e)
{
	if (e->help) {
		printf("\tn1 n2 DIV -> <n1/n2>\n");
		return 0;
	}
	long a, b;
	long dptr[2];
	double da, db;
	_var_t * tx, *st;
	if (num_cells(e,2)){
		if ( gt1(e) == INT && gt2(e) == INT){
			pop_int(e);
			a = e->lint;
			pop_int(e);
			b = e->lint;
			e->dbl = ((double) b) / a;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == INT && gt2(e) == DUBL){
			pop_int(e);
			a = e->lint;
			pop_double(e);
			e->dbl /= a;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == DUBL && gt3(e) == INT){
			pop_double(e);
			da = e->dbl;
			pop_int(e);
			a = e->lint;
			e->dbl = ((double) a)/da;
			push_dbl_to_stack(e);
			return 0;
		}
		if ( gt1(e) == DUBL && gt3(e) == DUBL){
			pop_double(e);
			da = e->dbl;
			pop_double(e);
			e->dbl /= da;
			push_dbl_to_stack(e);
			return 0;
		}

	}
	return 0;
}
int wl0_MOD(nicenv_t * e){
	if (e->help) {
		printf("\tn1 n2 MOD -> <n1 mod n2>\n");
		return 0;
	}
	long a, b;
	long dptr[2];
	double da, db;
	_var_t * tx, *st;
	if (num_cells(e,2)){
		if ( gt1(e) == INT && gt2(e) == INT){
			pop_int(e);
			a = e->lint;
			pop_int(e);
			b = e->lint;
			push_int_number(e, b%a);
			return 0;
		}
	}
	return 0;
}



int wl0_INC(nicenv_t * e){
	if (e->help) {
		printf("\tn INC -> n++\n");
		return 0;
	}
	if (num_cells(e,1)){
		if ( gt1(e) == INT ){
			long i = (long) gs1(e);
			i++;
			ss1(e, (void *) i);
		}
	}
	return 0;
}
int wl0_DEC(nicenv_t * e){
	if (e->help) {
		printf("\tn DEC -> n--\n");
		return 0;
	}
	if (num_cells(e,1)){
		if ( gt1(e) == INT ){
			long i = (long) gs1(e);
			i--;
			ss1(e, (void *) i);
		}
	}
	return 0;
}
/*
*	Извлекаем токен за токеном и переводим его в сикод.
*/
int translate_string(nicenv_t * e, _var_t * iline, _var_t * oline){
	// извлекаем токены

	char *tok = NULL;
	char str[MAX_STRING]; 
	_var_t * vn = NULL;
	char tline[MAX_STRING];
	char oname[MAX_NAME_LENGTH];
	_var_t * ns = NULL;
	int str_cnt = 0; // счетчик строк текста
	char * saveptr;

	// счетчик переменных будет находится в параметре e->var_counter;
	char tvarn[MAX_NAME_LENGTH];

	memset(str, 0, MAX_STRING);
	strcpy(str, (char*) iline->first_element);
	
   		
	printf("//String to translate: {%s}\n", str);
	//memset(tline,0, MAX_STRING);
	//sprintf(tline, "//%s %s", iline->name, str );
	//sprintf(tline, "%s\nint wl1_%s(nicenv_t * e )\n{", tline, iline->name );
	//printf("%s\n", tline);

	tok = strtok_r(str, " \t", &saveptr);
	//printf( "tok: [%s]\n", tok );

	while( tok != NULL ) 
   	{
   		// is_escape() - эскейп последовательность
   		// is_string()
   		// будем использовать флаг строки в среде, посколько у момент трансляции парсинга
   		// не происходит и этот флаг сброшен. Потом его надо сбросить в конце.
   		if (is_string(e, tok)) {
   			// Добавляем токен к строке если это строка
   			//printf("string detected, content: <%s>\n", e->string);
   			if (e->string_flag == 0){
   				// есть какая-то строка для добавления ее в стек.
   				// add_string()
   				memset(tline,0, MAX_STRING);
   				sprintf(tline, "\n\tstrcpy(e->string, \"%s\" ); ", e->string );
   				sprintf(tline, "%s\n\tadd_string(e);", tline);
   				// debug
   				//printf("\t//%s %s\n", tok, tline);
   				//sprintf(tline,"\t//%s %s\n", tok, tline);
   				printf("%s", tline);
 			}
   			goto next_tok;
   		}
   		if (is_number(e, tok) > 0){
   			//printf("number detected\n");
   			// есть какое-то число для добавления в стек в стек.
   			// is_number(e,"number")
			// add_string()
   			memset(tline,0, MAX_STRING);
   			sprintf(tline,"\n\tis_number(e, \"%s\");", tok);
			sprintf(tline,"%s\n\tadd_number(e);", tline);
			
			//printf("\t//%s %s\n", tok, tline);
			//sprintf(tline,"\t//%s %s\n", tok, tline);
   			printf("%s", tline);
			  			
   			goto next_tok;	
   		}
   		// is_type() - for complex type names, add later
   		if (is_word(e, tok) > 0){
   			//printf("word detected\n");
   			memset(tline,0, MAX_STRING);
   			// добавляем слово в строку
   			sprintf(tline,"\n\tis_word(e, \"%s\");", tok);
   			sprintf(tline,"%s\n\tprocess_word(e);", tline);
   			//printf("\t//%s %s\n", tok, tline);
   			//sprintf(tline,"\t//%s %s\n", tok, tline);
   			printf("%s", tline);
			
   			goto next_tok;	
   		}
   		// is_var()
      	vn = is_var_name_exists(e, tok);
      	if (vn != NULL) {
      		//грузим переменную с таким именем в стек
      		// _var_t * var_XXX = is_var_name_exists(e, "name");
      		// if (var_XXX != NULL) push_var_ptr_to_stack( e, var_XXX);
      		memset(tline, 0, MAX_STRING);
      		memset(tvarn, 0, MAX_NAME_LENGTH);
      		sprintf(tvarn,"var_%s_%d", tok, e->var_counter++);
      		sprintf(tline,"\n\t_var_t * %s = is_var_name_exists(e, \"%s\");", tvarn, tok);
      		sprintf(tline,"%s\n\tif (%s != NULL) push_var_ptr_to_stack( e, %s);", tline, tvarn, 
      			tvarn);
      		//printf("\t//%s %s\n", tok, tline);
      		//sprintf(tline,"\t//%s %s\n", tok, tline);
   			printf("%s", tline);

      	} else {
      		// создаем новую переменную с таким именем.
      		// _var_t * var_XXX = new_var(e, "name", UDEF);
      		memset(tline,0, MAX_STRING);
      		memset(tvarn, 0, MAX_NAME_LENGTH);
      		sprintf(tvarn,"var_%s_%d", tok, e->var_counter++);
      		sprintf(tline,"\n\t_var_t * %s = new_var(e, \"%s\", UDEF);", tvarn, tok);
      		sprintf(tline,"%s\n\tif (%s != NULL) push_var_ptr_to_stack( e, %s);", tline, tvarn, 
      			tvarn);
      		
      		//printf("\t//%s %s\n", tok, tline);
      		//sprintf(tline,"\t//%s %s\n", tok, tline);
   			printf("%s", tline);
      	}

next_tok:    
		// save the translation
		// Генерируем новое имя для строки, добавляем в текст
		memset(oname,0,MAX_NAME_LENGTH);
		str_cnt = oline->num_element;
		sprintf(oname, "%s_%d", oline->name, str_cnt);
   		ns = new_var(e, oname, STRI);
   		set_string(ns, tline);
   		add_string_to_text(e, oline, ns, POS_END);
   		
     	
     	//printf( "tok: [%s]\n", tok );
      	tok = strtok_r(NULL, " \t", &saveptr);
   	}
   	//printf("\treturn 0;\n}\n");
	return 0;
}

/*
* Генерируется случайное имя с заданным префиксом
* сохраняется имя в e->tmpName[MAX_NAME_LENGTH]
*/
char * random_name(nicenv_t * e, char * prefix){
	if (e->sp1 < MAX_STACK_SIZE)
	{
		wl0_RAND(e);
		pop_int(e);
		sprintf(e->tmpName, "%s_%X", prefix, (unsigned int) e->lint);
		return e->tmpName;
	}
	return NULL;
}

/*
*  Транслирует код nic в си. и сохраняет в другой переменной.
*/
int wl0_TRAN(nicenv_t * e)
{
	if (e->help) {
		printf("\tstr TRAN -> Translate nic into C lang.\n");
		return 0;
	}
	/*
		Пока создаем только переменные, грузим их в память
	*/
	_var_t * s = NULL;
	_var_t * out = NULL;
	_var_t * t = NULL;
	char oname[MAX_NAME_LENGTH];
	int i;
	if ( (e->sp1 > 0) && (gt1(e) == STRI) ){
		s = (_var_t *)  gs1(e);
		printf("//Translating the string: %s\n", s->name);
		// создаем новую строку переменной для парсинга
		// Создаем новую текстовую переменную, если oline еще пустое.
		wl0_RAND(e);
		pop_int(e);
		memset(oname,0,MAX_NAME_LENGTH);
		sprintf(oname, "%s_%X",s->name, (unsigned int) (e->lint) );
		out = new_var(e, oname, TEXT);
		
		translate_string(e, s, out);
		
		st1(e, 0);
		ss1(e, 0);
		dsp1(e);
		push_var_ptr_to_stack(e, out);
		return 0;
	}
	if ( (e->sp1 > 0) && (gt1(e) == TEXT) ){
		t = (_var_t *)  gs1(e);
		//printf("Translating the text: %s\n", t->name);
		wl0_RAND(e);
		pop_int(e);
		memset(oname,0,MAX_NAME_LENGTH);
		sprintf(oname, "%s_%X",t->name, (unsigned int) e->lint);
		out = new_var(e, oname, TEXT);

		for (i=0; i<t->num_element; i++){
			s = ((_var_t**) t->first_element)[i];
			translate_string(e, s, out);
			if (e->string_flag) {
				// открытая строка.
				sprintf(e->string, "%s\\\\", e->string);
			}
		}
		
		st1(e, 0);
		ss1(e, 0);
		dsp1(e);
		push_var_ptr_to_stack(e, out);
	}
	return 0;
}
/*
* Компилятор нового слова. Новое слово сохраняется в файл, компилируется и загружается
* новая динамическая библиотека.
*
*	Скомпилировать можно только переменную TEXT поскольку имя переменной будет являться
*	названием нового слова.
*/
int wl0_COMP(nicenv_t * e)
{
	if (e->help) {
		printf("\ttxt COMP -> Compile C code.\n");
		return 0;
	}
	// 1) Считаем, что в тексте содержится фрагмент сишного кода, который мы сохраняем в файл.
	// 3) компилируем библиотеку
	// 4) выгружаем старую если есть, загружаем новую
	// 5) подгружаем новое слово
	long t;
	_var_t * v, *fname, *h, *f;
	int i;
	char name_upper[MAX_NAME_LENGTH];
	char vn[MAX_NAME_LENGTH];
	char header[MAX_STRING], footer[MAX_STRING];
	
	memset(header, 0, MAX_STRING);
	memset(footer, 0, MAX_STRING);
	memset(name_upper, 0, MAX_NAME_LENGTH);

	if (num_cells(e,1)){
		t = gt1(e);
		v = (_var_t *)  gs1(e);
		if (t == TEXT){
			i=0;
			while (v->name[i]){
				name_upper[i] = toupper(v->name[i]);
				i++;
			}
			//printf("Compiling word wl1_%s\n", name_upper);
			//printf("int wl1_%s(nicenv_t * e){\n", name_upper);
			sprintf(header, "int wl1_%s(nicenv_t * e){\n", name_upper);
			h = new_var(e, random_name(e, v->name), STRI);
			set_string(h, header);

			add_string_to_text(e, v, h, 0);
			//wl0_PR(e);

			//printf("\treturn 0;\n}\n");
			sprintf(footer, "\n\treturn 0;\n}\n");
			f = new_var(e, random_name(e, v->name), STRI);
			set_string(f, footer);

			add_string_to_text(e, v, f, POS_END);
			/// Check if this name is already occupied and add the number to the function name.

			// Задаем имя файла для сохранения v->name
			memset(vn,0,MAX_NAME_LENGTH);
			sprintf(vn, "_tmp_%s.c", name_upper );
			fname = new_var(e, vn, STRI);
			set_string(fname, vn);
			push_var_ptr_to_stack(e, fname);
			
			//return 0;
			wl0_SAVE(e);
		}
	}

	return 0;
}

/*
* 	Compile using c compiler
* 	На входе должно быть имя для компиляции.
*/
int wl0_CC(nicenv_t * e){
	if (e->help) {
		printf("\t<fname> CC -> Compile C code from <fname>.\n");
		return 0;
	}
	long t;
	_var_t * v, *fname, *h, *f;
	int i;
	char command[MAX_STRING];
	memset(command, 0, MAX_STRING);

	if (num_cells(e,1)){
		t = gt1(e);
		v = (_var_t *)  gs1(e);
		if (t == STRI){
			// предполагаем, что это имя файла который надо скомпилировать.
			// ....
		}
	}

	return 0;
}

int wl0_DEL(nicenv_t * e){
	if (e->help) {
		printf("\tv DEL -> Delete variable completely.\n");
		return 0;
	}
	_var_t * v = NULL;
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE){
		v = (_var_t*) gs1(e);
		if ( is_var(e, v )) {
			if (del_var_by_name(e, v->name) == 0){
				drop1(e);
			}
		}
	}
	return 0;
}
/*Удаление всех автоматических переменных с флагом lock = -1 */
int wl0_ADEL(nicenv_t * e){
	if (e->help) {
		printf("\tADEL -> Remove all auto variables\n");
		return 0;
	}
	del_auto_vars(e);
	return 0;
}
/*
* Автоматическое создание переменных.
*/
int wl0_AUTO(nicenv_t * e){
	if (e->help) {
		printf("\t1 AUTO -> Enable the automatic variable creation\n");
		printf("\t0 AUTO -> Disable the automatic variable creation\n");
		return 0;
	}
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE && gt1(e) == INT){
		pop_int(e);
		if (0 == e->lint){e->avar = 0;}
		else e->avar = 1;
	}
	return 0;
}
/*Создает переменную, только делает переменную автоматической*/
int wl0_AVAR(nicenv_t * e){ 
	if (e->help) {
		printf("\tv AVAR -> v' /*Make variable automatic*/\n");
		return 0;
	}
	_var_t * v = NULL;
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE){
		v = (_var_t*) gs1(e);
		if ( is_var(e, v )) v->lock = -1; // переменная будет уделаена по команде ADEL
	}
	return 0;
}

/* Lock var from delete*/
int wl0_LOCK(nicenv_t * e){
	if (e->help) {
		printf("\tv LOCK -> Lock variable from deletion.\n");
		return 0;
	}
	_var_t * v = NULL;
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE){
		v = (_var_t*) gs1(e);
		if ( is_var(e, v )) v->lock = 1;
	}
	return 0;
}
/* UnLock var from delete*/
int wl0_UNLO(nicenv_t * e){
	if (e->help) {
		printf("\tv UNLO -> Unlock variable from deleteion.\n");
		return 0;
	}
	_var_t * v = NULL;
	if (num_cells(e,1) && e->sp1 < MAX_STACK_SIZE){
		v = (_var_t*) gs1(e);
		if ( is_var(e, v )) v->lock = 0;
	}
	return 0;
}
int file_save_app(nicenv_t * e, char * mode){
	FILE* l_fp ;
    char line[MAX_STRING + 1] ;
    char* token ;
    _var_t * s, * t, * s0;
    char str_name[MAX_NAME_LENGTH];
    int i = 0;

    if (num_cells(e,2) && gt2(e) == TEXT && gt1(e) == STRI)
    {
        s = (_var_t *) gs1(e);
	    l_fp = fopen( (char*) s->first_element , mode ) ;
	    if (l_fp == NULL) 
	    {
	    	perror("Open config failed");
	    	return -1;
	    }
	    drop1(e);
	    t = (_var_t *) gs1(e); // text
	    
	    for(i=0; i< t->num_element; i++)
	    {
	    	s0 = ((_var_t **) t->first_element)[i];
	    	fputs((char*) s0->first_element, l_fp);
	    	fputs("\n", l_fp); 
	    }
	    del_var_by_name(e, s->name);
	    drop1(e);
    	if (l_fp != NULL) fclose(l_fp);
	}
    return 0;
}
/*
*	Сохраняет файл с заданным именем.
*/
int wl0_SAVE(nicenv_t * e){
	if (e->help) {
		printf("\ttxt s SAVE -> Save file with name s with content txt.\n");
		return 0;
	}
	return file_save_app(e, "w");
}

/*
*	Дописывает в конец файла.
*/
int wl0_APEN(nicenv_t * e){
	if (e->help) {
		printf("\ttxt s APEN -> Append file with name s with content txt.\n");
		return 0;
	}
    return file_save_app(e, "a");
}


int wl0_LOAD(nicenv_t * e)
{
	if (e->help) {
		printf("\ts txt LOAD -> Load file with name s to undefined txt variable.\n");
		return 0;
	}
    FILE* l_fp ;
    char line[MAX_STRING + 1] ;
    char* token ;
    _var_t * s, * t, * s0;
    char str_name[MAX_NAME_LENGTH];
    int i = 0;
    make_same_name_udef_var(e);
    if (num_cells(e,2) && gt1(e) == UDEF && gt2(e) == STRI)
    {
        s = (_var_t *) gs2(e);
	    l_fp = fopen( (char*) s->first_element , "r" ) ;
	    if (l_fp == NULL) 
	    {
	    	perror("Open file failed");
	    	return -1;
	    }
	    wl0_SWAP(e);
	    drop1(e);
	    wl0_TEXT(e);
	    
	    wl0_VAR(e);
	    t = (_var_t *) gs1(e); // text
	    
	    while( fgets( line, MAX_COUNT, l_fp ) != NULL )
	    {
	    	//sprintf(str_name,"%s_%d",(char*) s->first_element, i++);
	    	generate_unique_name(e, "ss", str_name);
	    	s0 = new_var(e, str_name, STRI);
	    	if (s0 == NULL) {
	    		return -1;
	    	}

			//printf("Line[%d]:%s\n", i, line);	
			if (strlen(line) >0 ){        
				line[strlen(line)-1] = 0; // очищаем \n
				set_string(s0, line); 
				add_string_to_text(e, t, s0, POS_END);
				line[0] = 0;
			}
	    }
	    del_var_by_name(e, s->name);
	}
    fclose(l_fp);
    return 0;
}

/*
*	Заданние имени переменной.
*	a <name> NAME -> a
*/
int wl0_NAME(nicenv_t * e){
	if (e->help) {
		printf("\ta \"new_var_name\" NAME -> a\n");
		return 0;
	}
	_var_t * s, * t, * s0 = NULL;
    char str_name[MAX_NAME_LENGTH];
    int i = 0;

    if (num_cells(e,2) && is_var(e,(_var_t * )gs2(e)) && gt1(e) == STRI)
    {
        s = (_var_t *) gs1(e);
	    drop1(e);
	    t = (_var_t *) gs1(e); 
	    memset(t->name,0, MAX_NAME_LENGTH);
	    strcpy(t->name, (char*) s->first_element );
	    drop1(e);
	    del_var_by_name(e, s->name);
	}
    return 0;
}
/*  Сохраняем имя переменной как строку в стеке */
int wl0_NOM(nicenv_t * e){
	if (e->help) {
		printf("\ta NOM -> s /*Pun the name of variable in the stack as a string.*/\n");
		return 0;
	}
	char str_name[MAX_NAME_LENGTH];
    if (num_cells(e,1) && is_var(e,(_var_t * )gs1(e)))
    {
        _var_t * s = (_var_t *) gs1(e);
	    drop1(e);
	    strcpy(e->string, s->name);
	    add_string(e);
	}
    return 0;
}
/* Создает синонимы для слов или переменных. Переменные ссылаются на одну и ту же область
	а синоним слова ссылается на тот же код. Это было бы полезно использовать для передачи переменных в 
	подпрограммы.
*/
int wl0_LN(nicenv_t * e){
	if (e->help) {
		printf("\tw syn LN -> /* Create a symbolic link of variable or word */\n");
		return 0;
	}
	_var_t * v, * vln;
	_word_t * w;
    char str_name[MAX_NAME_LENGTH];
    int i = 0;
	// Нужно сделать символичесике линки, которые можно удалить потом не удаляя переменные
	// случай переменной
	make_same_name_udef_var(e);
    if (num_cells(e,2) && is_var(e,(_var_t * )gs2(e)) 
    	&& gt1(e) == UDEF && is_var(e,(_var_t * )gs1(e))){
    	v = (_var_t * )gs2(e);
    	vln = (_var_t * ) gs1(e);
    	vln->first_element = (char*) malloc(MAX_NAME_LENGTH);
    	strcpy(vln->first_element, v->name);
    	vln->type.type_index = v->type.type_index;
    	st1(e, (long)v->type.type_index);
    	vln->link = 1; 
    	drop1(e); drop1(e);
    }
	if (num_cells(e,2) && ( gt2(e) == CODE ) 
    	&& gt1(e) == UDEF && is_var(e,(_var_t * )gs1(e))){
    	// линк на код
    	w = (_word_t *)  gs2(e);
		//(*(w->code))(e);
    	vln = (_var_t * ) gs1(e);
    	vln->first_element = (char*) malloc(MAX_NAME_LENGTH);
    	strcpy(vln->first_element, w->name);
    	vln->type.type_index = CODE;
    	st1(e, CODE);
    	vln->link = 1; //
    	drop1(e); drop1(e);
    }
	return 0;
}

/* 
	Копирование одного объекта в другой
*/
int wl0_CP(nicenv_t * e){
	if (e->help) {
		printf("\torg copy CP -> /* Make a full copy of org object */\n");
		return 0;
	}
	printf("Not implemented yet\n");
	return 0;
}


/*
* Склеивает строки в текст или символы в строку, в зависимости от того,
* какие переменые лежат в стеке. выбирает весь стек до VOID, или до дна.
* строки добавляются в том порядке в котором они вводились в интерпретатор
* если текст был непустой, то строки дописываются в конец
*/
int wl0_GLUE(nicenv_t * e){
	if (e->help) {
		printf("\t[void] s1 s2 ... sn txt GLUE -> txt\'\n");
		return 0;
	}
	int i, j;
	_var_t * s, * t;
	if (num_cells(e,2)){
		if ( gt1(e) == UDEF && gt2(e) == STRI){
			// Определяем сначала переменную как текст
			// Потом переходим ко второму шагу
			if (e->sp1 < MAX_STACK_SIZE){
				wl0_TEXT(e);
				//wl0_SWAP(e);
				wl0_VAR(e);
			}
			st1(e, TEXT);	
		}
		if ( gt1(e) == TEXT && gt2(e) == STRI){
			//Добавляем строки к тексту.
			//Ищем первую строку
			j = e->sp1-2;
			for(i = e->sp1-2; i>=0; i--){
				if (e->T1[i] == STRI) 
					j = i;
				else
					break;
			}

			t = (_var_t *) gs1(e);
			for (i=j; i<=e->sp1-2; i++){
				s = (_var_t *) e->S1[i];
				e->S1[i] = NULL;
				e->T1[i] = 0;
				add_string_to_text(e, t, s, POS_END);
			}
			e->S1[j] = (void*) t;
			e->T1[j] = TEXT;
			ss1(e, NULL);
			st1(e, 0);
			e->sp1 = j+1;

			return 0;
		}
		if ( gt1(e) == STRI && gt2(e) == STRI){
			// Склеиваем строки
			// .... надо доделать
			return 0;
		}
		if ( gt1(e) == STRI && gt2(e) == LETR){
			// Склеиваем буквы в строку.
			// .... надо доделать
			return 0;
		}

	}
	return 0;
}

/*
* Склеивает строки в текст последовательно, в текстовую переменную, с тем чтобы образовать тело скрипта.
*/
int wl0_PACK(nicenv_t * e){
	if (e->help) {
		printf("\ttxt s1 s2 ... sn PACK -> txt\' /* Append strings sequentially to txt */\n");
		return 0;
	}
	int i, j;
	_var_t * s, * t;
	if (num_cells(e,2)){
		if ( gt1(e) == STRI ){
			int it = 0;
			while ( (e->sp1-1 >0) && (gt1(e) == STRI )  ){
				if (gt1(e) == STRI) {
					wl0_PUSH(e);
					it++;
				}
			}
			if (gt1(e) == TEXT ){
				// добавляем строки по одной
				for (int i=0; i<it; i++){
					wl0_POP(e);
					wl0_ADD(e);
				}
			}else
			{
				push_int_number(e, it);
				wl0_FLOP(e);
				return 0;
			}
		}
		/*
		if ( gt1(e) == STRI && gt2(e) == STRI){
			// Склеиваем строки
			// .... надо доделать
			return 0;
		}
		if ( gt1(e) == STRI && gt2(e) == LETR){
			// Склеиваем буквы в строку.
			// .... надо доделать
			return 0;
		}*/

	}
	return 0;
}

/*
* Вставляет строку в нужную позицию в тексте
*/
int wl0_INS(nicenv_t * e){
	if (e->help) {
		printf("\ttxt s n INS -> txt\' /*Insert string <s> in the <txt> at position <n>.*/\n");
		printf("\t If <n>==0 inserts in the beginning, <n>==-1 iserts at the end.\n");
		return 0;
	}
	if (num_cells(e,3)){
		if ( gt3(e) == TEXT && gt2(e) == STRI && gt1(e) == INT){
			pop_int(e);
			long line = e->lint; 
			_var_t * tx = (_var_t *) gs2(e);
			_var_t * st = (_var_t *) gs1(e);

			add_string_to_text(e, tx, st, line);
			drop1(e);
			return 0;
		}
	}
	return 0;
}

int set_long_to_var(nicenv_t * e, _var_t * v, long l){
	return 0;
}
// TODO: сделать присвоение всех встроенных типов
int wl0_EQ(nicenv_t * e){
	if (e->help) {
		printf("\tx v EQ -> vx\n");
		return 0;
	}
	long a, t, t0;
	double b;
	/* Переделать эту функцию на всевозможные типы*/
	_var_t * v = NULL;
	if (num_cells(e,2) && (gt1(e) == UDEF || gt1(e) == VAR_INT || gt1(e) == VAR_DUBL) )
	{
		t = gt2(e); // type of constant
		v = (_var_t *) gs1(e);
		switch(t){
			case INT:{
				wl0_SWAP(e);
				pop_int(e);
				a = e->lint;
				set_num_to_var(v, a);
				v->type.type_index = VAR_INT; 
				st1(e, VAR_INT); 
			}
			break;
			case DUBL: {
				wl0_SWAP(e);
				pop_double(e);
				b = e->dbl;
				v = (_var_t *) gs1(e);
				set_dbl_to_var(v, b);
				v->type.type_index = VAR_DUBL; st1(e, VAR_DUBL);
			}
			break;
		}
	}
	return 0;
}


/*Setting of value to variable*/
int wl0_QE(nicenv_t * e){
	if (e->help) {
		printf("\tv x QE -> vx\n");
		return 0;
	}
	wl0_SWAP(e);
	wl0_EQ(e);
	return 0;
}

/*Put the value of the variable to the stack*/
int wl0_VALU(nicenv_t * e){
	if (e->help) {
		printf("\tvx VALU -> x\n");
		return 0;
	}
	//Простые переменные типа VAR_INT, VAR_BYTE, VAR_LETR, VAR_CHAR
	long a, t1, t;
	double b;
	long idx;
	_var_t * v, * v1;
	if (num_cells(e,1)){
		t = gt1(e);
		v = (_var_t *) gs1(e);
		switch(t){
			case VAR_BYTE:
			case VAR_CHAR:
			case VAR_LETR:
			case VAR_INT: {
				a = get_num_from_var(v);
				if (t == VAR_BYTE) {ss1(e,(void*) a); st1(e, BYTE);}
				if (t == VAR_LETR) {ss1(e,(void*) a); st1(e, LETR);}
				if (t == VAR_CHAR) {ss1(e,(void*) a); st1(e, CHAR);}
				if (t == VAR_INT)  {ss1(e,(void*) a); st1(e, INT);}
			}
			break;
			case VAR_DUBL: {
				b = get_dbl_from_var(v);
				printf("DBL: {%lf}\n", b);
				dsp1(e);
				e->dbl = b;
				push_dbl_to_stack(e);
			}
			break;
			
			case IDX: {	// сложные типы, например вектора.
				pop_int(e);
				idx = e->lint; // сохраняем иднекс объекта, который нужно извлечь
				// тут могут быть типы DOT, PDOT, VEC, RVEC, MAT, STRI, TEXT, ....
				if (num_cells(e,1)){
					t1 = gt1(e);
					v1 = (_var_t *) gs1(e);
					drop1(e);
					if (t1 == DOT || t1 == VEC || t1 == PDOT || RVEC ){
						if (idx >= 0 && idx <=v1->num_element) {
							double * fe = (double*) v1->first_element;
							e->dbl = fe[idx];
							push_dbl_to_stack(e);
						}
					}
					if (t1 == TEXT || t1 == ARRA || t1 == MAT ){
						printf("not implemented\n"); 
					}
				}
			}
			break;
		}
	}
	return 0;
}

int wl0_EXP(nicenv_t * e){
	if (e->help) {
		printf("\tx EXP -> exp(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = exp((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = exp(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}

int wl0_LOG(nicenv_t * e){
	if (e->help) {
		printf("\tx LOG -> LOG(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = log((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = log(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}

int wl0_ABS(nicenv_t * e){
	if (e->help) {
		printf("\tx ABS -> ABS(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->lint = abs((long) e->lint);
			push_int_number(e, e->lint);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = abs(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}

int wl0_SIGN(nicenv_t * e){
	if (e->help) {
		printf("\tx SIGN -> SIGN(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			if (e->lint == 0) {
				push_int_number(e, 0);
				return 0;	
			}
			e->lint = e->lint / abs((long) e->lint);
			push_int_number(e, e->lint);
		}
		if (t == DUBL){
			pop_double(e);
			if (e->dbl == 0.0) {
				push_int_number(e, 0);
				return 0;	
			}
			e->lint = e->dbl / abs(e->dbl);
			push_int_number(e, e->lint);
		}
	}
	return 0;
}

int wl0_POW(nicenv_t * e){ //fix it
	if (e->help) {
		printf("\tx p POW -> (x)^p\n");
		return 0;
	}
	long t, t1;
	double a;
	double pwr,d;
	if (num_cells(e,2)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			pwr = (double) e->lint;
			t = gt1(e);
			if (t == BYTE || t == LETR || t == CHAR || t == INT){
				pop_int(e);
				d = (double) e->lint;
			} else
			if (t == DUBL){
				pop_double(e);
				d = e->dbl;
			}
			e->dbl = pow(d,pwr);
			push_dbl_to_stack(e);
			return 0;
		}
		if (t == DUBL){
			pop_double(e);
			pwr = e->dbl;
			t = gt1(e);
			if (t == BYTE || t == LETR || t == CHAR || t == INT){
				pop_int(e);
				d = (double) e->lint;
			} else
			if (t == DUBL){
				pop_double(e);
				d = e->dbl;
			}
			e->dbl = pow(d,pwr);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}

int wl0_SQRT(nicenv_t * e){
	if (e->help) {
		printf("\tx SQRT -> (x)^(1/2)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = sqrt((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = sqrt(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}

int wl0_COS(nicenv_t * e){
	if (e->help) {
		printf("\tx COS -> cos(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			a = e->lint;
			//printf("A = %lf\n", a);
			e->dbl = cos((double)a);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			a = e->dbl;
			//printf("A1 = %lf\n", a);
			e->dbl = cos(a);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
int wl0_SIN(nicenv_t * e){
	if (e->help) {
		printf("\tx SIN -> sin(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = sin((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = sin(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
int wl0_TAN(nicenv_t * e){
	if (e->help) {
		printf("\tx TAN -> tg(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = tan((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = tan(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
int wl0_ACOS(nicenv_t * e)
{
	if (e->help) {
		printf("\tx ACOS -> arccos(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = acos((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = acos(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
int wl0_ASIN(nicenv_t * e){
	if (e->help) {
		printf("\tx ASIN -> arcsin(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = asin((double) e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = asin(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
int wl0_ATAN(nicenv_t * e)
{
	if (e->help) {
		printf("\tx ATAN -> arctg(x)\n");
		return 0;
	}
	long t;
	double a;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == BYTE || t == LETR || t == CHAR || t == INT){
			pop_int(e);
			e->dbl = atan((double)e->lint);
			push_dbl_to_stack(e);
		}
		if (t == DUBL){
			pop_double(e);
			e->dbl = atan(e->dbl);
			push_dbl_to_stack(e);
		}
	}
	return 0;
}
// Генерирует случайное число.
int wl0_RAND(nicenv_t * e){
	if (e->help) {
		printf("\tRAND -> rnd_int \n");
		return 0;
	}
	if(e->sp1<MAX_STACK_SIZE){
		
		srand(e->prev_rand);
		e->lint = rand();
		srand(e->lint);
		e->lint = rand();
		e->prev_rand = e->lint;
		
		push_int_number(e, e->lint);		
	}	
	return 0;
}
// Добавляем в стек последнюю определенную переменную
int wl0_LAST(nicenv_t * e){
	if (e->help) {
		printf("\tlast -> v \n/* Place LAST created variable to stack*/\n");
		return 0;
	}
	push_int_number(e, 0);
	wl0_PREV(e);
	return 0;
}

int wl0_PREV(nicenv_t * e){
	if (e->help) {
		printf("\tn PREV -> v_n \n/* Place previously created variable of n count from the end of variable list*/\n");
		printf("\tvar PREV -> var' \n/*Get the previous screened variable with the same name. */\n");
		return 0;
	}
	if(e->sp1<MAX_STACK_SIZE && INT == gt1(e)){
		pop_int(e);
		_var_t * v = NULL;
		for (long i=0; i<(e->lint + 1); i++){
			v = prev_var(e, v);
		}
		push_var_ptr_to_stack(e, v);
	}else {
		_var_t * vn = (_var_t *) gs1(e);
		if(e->sp1<MAX_STACK_SIZE && is_var(e,vn)){ // ишем переменную с таким же именем
			_var_t * vp = vn;
			do{
				vp = prev_var(e,vp);
			}while (!(0 == strcmp(vp->name, vn->name)));
			// помещаем в стек новую переменную
			drop1(e);
			push_var_ptr_to_stack(e,vp);
		}
	}
	return 0;
}
// Проверяет, существует ли переменная и помещает 1 или 0 в стек
int wl0_XIST(nicenv_t * e){
	if (e->help) {
		printf("\tvx XIST -> 1|0 \n/* Check if variable exists */\n");
		return 0;
	}
	if(num_cells(e,1) && is_var(e, gs1(e))){
		if (UDEF != gt1(e)){
			drop1(e);
			push_int_number(e,1);
		} else {
			wl0_DEL(e);
			push_int_number(e,0);
		}
	}
	return 0;
}

/* Выполнить в шелле*/
int wl0_SH(nicenv_t * e)
{
	if (e->help) {
		printf("\ts sh -> execute command in shell\n");
		return 0;
	}	
	long t;
	_var_t * v;
	if (num_cells(e,1)){
		t = gt1(e);
		if (t == STRI){
			v = (_var_t *) gs1(e);
			system((const char*) v->first_element);
			drop1(e);
			//printf("Shell command executed\n");
		}
	}
	return 0;
}
/*
* Находит максимальное значение из массива чисел. Может быть INT может быть DUBL
* Если нету void, то слово проверяет все аргументы в стеке.
*/
int wl0_NMAX(nicenv_t * e)
{
	if (e->help) {
		printf("\tvoid .. a0 a1 a2 .. an NMAX -> maxn\n");
		return 0;
	}	
	long imax = LONG_MIN;
	double dmax = log(0);
	long t;
	int i;
	unsigned char dbl_flag = 0;
	if (num_cells(e,1)){
		// Проверяем все аргументы по порядку пока не встретим войд или дойдем до конца стека
		while ( (e->sp1-1) >=0){
			i = e->sp1-1;
			//for (i=e->sp1-1; i>= 0; i--){
			t = e->T1[i];
			if (t == INT){
				pop_int(e);
				if (e->lint > imax) imax = e->lint;
				//printf("int fould %ld\n", e->lint);

				if (dbl_flag){
					// был хотя бы один дабл, надо сравнить еще и даблом
					if ( dmax < imax) dmax = imax;
				}
			}else 
			if (t == DUBL){
				dbl_flag = 1;
				pop_double(e);
				//printf("double fould %lf\n", e->dbl);
				if (e->dbl > dmax) dmax = e->dbl;
			}else{
				break;
			}
		}
		//сохранить максимальное найденное число и выйти из цикла
			// эта функция не удаляет войд.
		//printf("max numbers %ld, %lf\n", imax, dmax);
		if (dbl_flag){
			// был хотя бы один дабл, надо сравнить еще и даблом
			if (imax>dmax) dmax = imax;
			e->dbl = dmax;
			push_dbl_to_stack(e);
		}else
		{
			e->lint = imax;
			push_int_number(e, e->lint);
		}
	}
	return 0;
}

/*
* Находит максимальное значение из массива чисел. Может быть INT может быть DUBL
* Если нету void, то слово проверяет все аргументы в стеке.
*/
int wl0_NMIN(nicenv_t * e)
{
	if (e->help) {
		printf("\tvoid .. a0 a1 a2 .. an NMIN -> minn\n");
		return 0;
	}	
	long imin = LONG_MAX;
	double dmin = -log(0);
	long t;
	int i;
	unsigned char dbl_flag = 0;
	if (num_cells(e,1)){
		// Проверяем все аргументы по порядку пока не встретим войд или дойдем до конца стека
		
		while ( (e->sp1-1) >=0){
		//for (i=e->sp1-1; i>= 0; i--){
			i = e->sp1-1;
			t = e->T1[i];
			if (t == INT){
				pop_int(e);
				if (e->lint < imin) imin = e->lint;
				//printf("int fould %ld\n", e->lint);

				if (dbl_flag){
					// был хотя бы один дабл, надо сравнить еще и даблом
					if ( dmin > imin) dmin = imin;
				}
			}else 
			if (t == DUBL){
				dbl_flag = 1;
				pop_double(e);
				//printf("double fould %lf\n", e->dbl);
				if (e->dbl < dmin) dmin = e->dbl;
			}else{
				break;
			}
		}
		//сохранить максимальное найденное число и выйти из цикла
			// эта функция не удаляет войд.
		//printf("min numbers %ld, %lf\n", imin, dmin);
		if (dbl_flag){
			// был хотя бы один дабл, надо сравнить еще и даблом
			if (imin < dmin) dmin = imin;
			e->dbl = dmin;
			push_dbl_to_stack(e);
		}else
		{
			e->lint = imin;
			push_int_number(e, e->lint);
		}
	}
	return 0;
}

/*
* Находит среднее значение из массива чисел. Может быть INT может быть DUBL
* Если нету void, то слово проверяет все аргументы в стеке.
*/
int wl0_NAVE(nicenv_t * e)
{
	if (e->help) {
		printf("\tvoid .. a0 a1 a2 .. an NAVE -> avg(n)\n");
		return 0;
	}	
	double dave = 0;
	long t;
	int i, n=0;
	unsigned char dbl_flag = 0;
	if (num_cells(e,1)){
		// Проверяем все аргументы по порядку пока не встретим войд или дойдем до конца стека
		
		while ( (e->sp1-1) >=0){
		//for (i=e->sp1-1; i>= 0; i--){
			i = e->sp1-1;
			t = e->T1[i];
			if (t == INT){
				pop_int(e);
				dave += e->lint;
				//printf("int fould %ld\n", e->lint);
				n++;
			}else 
			if (t == DUBL){
				pop_double(e);
				//printf("double fould %lf\n", e->dbl);
				dave += e->dbl;
				n++;
			}else{
				break;
			}
		}
		
		e->dbl = dave/n;
		push_dbl_to_stack(e);
	}
	return 0;
}
/*
* Возвращает размер типа в байтах.
*/
long size_of_type(nicenv_t * e, _newtype_t * t){
	long tsize = 0;
	if (t->type_index < MAX_TYPE_INDEX){
		// встроенный тип, можно поиграться с длиной, но пока пусть будет 8 байт
		//switch(t->type_index)
		tsize = sizeof(double);
	}else{
		// сложный тип
		tsize = sizeof(void*);
		if(t->type_size > 0) tsize = t->type_size;
	}
	return tsize;
}

/*
*	Выделяем массив. dim name type ARRA
*/
int wl0_ARRA(nicenv_t * e){
	if (e->help) {
		printf("\tdim1 .. dimn name type ARRA -> arr\n");
		return 0;
	}	
	int i, n=0;
	_var_t * narra = NULL;
	long t;
	long dims[8];
	char tname[MAX_NAME_LENGTH*8];
	long nelem;
	_newtype_t * nt = NULL;
	if (num_cells(e,3)){
		push_int_number(e,1);
		wl0_FLIP(e);
		make_same_name_udef_var(e);
		push_int_number(e,1);
		wl0_FLOP(e);
		if (gt1(e) == TYPE && gt2(e) == UDEF && gt3(e) == DIM){
			// Смотрим сколько у нас размерностей массива.
			narra = (_var_t*) gs2(e); // переменная
			narra->type.type_index = (long) gs1(e); // тип
			drop1(e); drop1(e);
			// Больше восьми резмерностей не буду делать.
			for (i=0; i<8; i++){
				if ( (e->sp1-1) >=0 && gt1(e) == DIM){
					dims[i] = (long) gs1(e);
					drop1(e);
					n++;
				}else
				{
					break;
				}
			}
			// имеем n размерностей.
			nelem = 1;
			//создаем новый тип переменной.
			memset(tname, 0, MAX_NAME_LENGTH*8);
			print_type(e, narra->type.type_index, narra->type.name);
			sprintf(tname,"ARRA_%s",narra->type.name);
			for (i=n-1; i>=0; i--){
				nelem *= dims[i] + 1;
				sprintf(tname,"%s[%ld]",tname,dims[i]);
			}
			
			// теперь надо найти размер элемента. Если это простой тип, то размер будет 8 байт.
			// если же сложный тип, то размер будет другим.
			nelem *= size_of_type(e, &(narra->type)); // размер выделяемой памяти в байтах.

			nt = new_type(e, tname, nelem);
			narra->type.type_index = nt->type_index;
			strcpy(narra->type.name, nt->name);
			narra->type.type_size = nelem;

			printf("Allocate memory %ld bytes\n", nelem );
			narra->first_element = (void *) malloc (nelem);
			if (narra->first_element == NULL){
				printf("Failed to allocate memory\n");
			}else
			{
				push_var_ptr_to_stack(e, narra);
			}
		}
	}
	return 0;
}

/*
*	Преобразуем объект к бинарной форм BARR
*/
int wl0_BARR(nicenv_t * e){
	if (e->help) {
		printf("\tNot implemented\n\n");
		printf("\t .. obj BARR -> objbin /*Converts any object into binary byte array*/ \n");
		return 0;
	}	
	return 0;
}

/* Помещение элемента в массив с инкрементом
* или присваивание значений координатам DOT,PDOT,VEC,RVEC 
*/
int wl0_PUT(nicenv_t * e){
	if (e->help) {
		printf("\tNot implemented yet!\n\n");
		printf("\tarr [n ORG] n1 PUT n2 PUT ... nm PUT -> arr\n");
		printf("\td vvar n1 IDX PUT -> /* Put d value int vvar[n1]*/\n");
		return 0;
	}	
	long t1, t2, t;
	long idx = 0;
	_var_t * v; 	
	if (num_cells(e,3)){
		t1 = gt1(e);
		t2 = gt2(e);
		if (t1 == IDX){
			if (t2 == DOT || t2 == VEC || t2 == PDOT || t2 == RVEC){
				v = (_var_t *) gs2(e); 
				double* fe = (double*) v->first_element;

				idx = (long) gs1(e);
				if (idx>=0 && idx <=v->num_element){
					drop1(e); drop1(e);
			
					t = gt1(e);
					if (t == DUBL) {
						pop_double(e);
						fe[idx] = e->dbl;
					} else if (t == INT){
						pop_int(e);
						fe[idx] = (double) e->lint;
					} else if (t == VAR_INT){
						wl0_VALU(e);
						pop_int(e);
						fe[idx] = (double) e->lint;
					} else if (t == VAR_DUBL){
						wl0_VALU(e);
						pop_double(e);
						fe[idx] = e->dbl;
					} else {
						return 0;
					}	
				}
			}
		}
	}
	return 0;
}
/*Помещение нескольких элементов в массив*/
int wl0_PUTN(nicenv_t * e){
	if (e->help) {
		printf("\tNot implemented yet!\n\n");
		printf("\tar [n ORG] n1 n2 n3 n4 n5 ... PUTN -> arr\n");
		return 0;
	}	
	return 0;
}


/* Создание сигнатуры типа */
int wl0_NEWT(nicenv_t * e){
	if (e->help) {
		printf("\tNot implemented yet!\n\n");
		printf("\t type1 type2 .. typen NEWT -> ntype\n");
		return 0;
	}	

	return 0;

}
/* Находит первое смещение подстроки в строке, или возвращает длину строки*/
int wl0_OFFS(nicenv_t * e){
	if (e->help) {
		printf("\t str pattern OFFS -> str n /* Find the first offset of the pattern in the str */\n");
		printf("\t /*If the pattern not found returns SLEN*/\n");
		return 0;
	}
	_var_t * str, * pattern; 	
	char * offs = NULL;
	if (num_cells(e,2) && gt1(e) == STRI && gt2(e) == STRI){
		str = (_var_t *) gs2(e);
		pattern = (_var_t *) gs1(e);
		offs = strstr((char *) (str->first_element), (char *) (pattern->first_element));
		drop1(e);
		if (offs == NULL) {
			wl0_SLEN(e); // Что лучше, возвращать длину строки или void или -1 - это вопрос риторический
			return 0;
		}
		e->lint = ((long)offs - (long) (str->first_element));
		push_int_number(e, e->lint);
	}
	return 0;
}
/* Отрезание начала строки, по заданному числу символов*/
int wl0_HEAD(nicenv_t * e){
	if (e->help) {
		printf("\t str n HEAD -> str\'\n");
		return 0;
	}	
	char oname[MAX_NAME_LENGTH];
	_var_t * is = NULL;
	_var_t * ns = NULL;
	char * inp_buff = NULL;
	long it = 0;
	if (num_cells(e,2) && gt1(e) == INT && gt2(e) == STRI)
	{
		is = (_var_t*) gs2(e);
		pop_int(e);
		it = e->lint;
		memset(oname,0,MAX_NAME_LENGTH);
		sprintf(oname, "%s_h%ld", is->name, it);
   		ns = new_var(e, oname, STRI);
   		if (it >= is->size){
   			//Просто копируем строку целиком.
   			set_string(ns, (char *) (is->first_element));
   		}
   		else if (it >=0){
   			inp_buff = (char *) malloc (it+1);
   			memset(inp_buff, 0, it+1);
   			strncpy(inp_buff, (char *) (is->first_element), it);
			set_string(ns, inp_buff);
   			free(inp_buff);
   		}
   		drop1(e);
   		push_var_ptr_to_stack(e, ns);
	}
	return 0;	
}

/* Отрезание конца строки, по заданному числу символов*/
int wl0_TAIL(nicenv_t * e){
	if (e->help) {
		printf("\t str n TAIL -> str\'\n");
		return 0;
	}	
	char oname[MAX_NAME_LENGTH];
	_var_t * is = NULL;
	_var_t * ns = NULL;
	char * inp_buff = NULL;
	long it = 0;
	if (num_cells(e,2) && gt1(e) == INT && gt2(e) == STRI)
	{
		is = (_var_t*) gs2(e);
		pop_int(e);
		it = e->lint;
		memset(oname,0,MAX_NAME_LENGTH);
		sprintf(oname, "%s_t%ld", is->name, it);
   		ns = new_var(e, oname, STRI);
   		if (it >= is->size){
   			//Возаращаем пустую строку.
   			//set_string(ns, NULL);
   		}
   		else if (it >=0){
   			inp_buff = (char *) malloc (it+1);
   			memset(inp_buff, 0, it+1);
   			strncpy(inp_buff, (char *) (is->first_element) + is->size - 1 - it, it);
			set_string(ns, inp_buff);
   			free(inp_buff);
   		}
   		drop1(e);
   		push_var_ptr_to_stack(e, ns);
	}
	return 0;	
}
/*возвращает длину строки в стеке. Строку из стека не удаляет. Длина считается вместе с дополнительным нулем
в конце строки*/
int wl0_SLEN(nicenv_t * e){
	if (e->help) {
		printf("\t str SLEN -> str n\n");
		return 0;
	}	
	_var_t * is = NULL;
	long it = 0;
	if (num_cells(e,1) && gt1(e) == STRI && (e->sp1)<MAX_STACK_SIZE)
	{
		is = (_var_t*) gs1(e);
		push_int_number(e, is->size);
	}
	return 0;
}

/*Вырезание фрагмента строки, может иметь 1 или 2 входных параметра*/
int wl0_CUT(nicenv_t * e){
	if (e->help) {
		printf("\t str n1 [len] CUT -> str\'\n");
		printf("\t n1 [len] CUT => SLEN n1 SUB TAIL [len HEAD]\n");
		return 0;
	}
	char oname[MAX_NAME_LENGTH];
	_var_t * is = NULL;
	_var_t * ns = NULL;
	char * inp_buff = NULL;
	long it = 0;
	long len = 0;
	if (num_cells(e,3) && gt1(e) == INT && gt2(e) == INT && gt3(e) == STRI)
	{
		is = (_var_t*) gs3(e);
		pop_int(e);
		len = e->lint;
		pop_int(e);
		it = e->lint;
		memset(oname,0,MAX_NAME_LENGTH);
		sprintf(oname, "%s_c%ld", is->name, len);
   		ns = new_var(e, oname, STRI);
   		if (it >= is->size){
   			//Возаращаем пустую строку.
   			//set_string(ns, NULL);
   		}
   		else if (it >=0){
   			if (len + it >= is->size) {
   				//Копируем до конца строки, 
   				len = is->size - it - 1;
   				inp_buff = (char *) malloc (len+1);
   				memset(inp_buff, 0, len+1);
	   			strncpy(inp_buff, (char *) (is->first_element) + it , len);
				set_string(ns, inp_buff);
   				free(inp_buff);
   			}else {
   				inp_buff = (char *) malloc (len+1);
   				memset(inp_buff, 0, len+1);
	   			strncpy(inp_buff, (char *) (is->first_element) + it , len);
				set_string(ns, inp_buff);
   				free(inp_buff);
   			}
   		}
   		drop1(e);
   		push_var_ptr_to_stack(e, ns);
	}
	return 0;	
}
/* Разбивает строку на слова. Преобразует stri в text, в котором каждая строка состоит из 1 слова*/
int wl0_SPLI(nicenv_t * e){
	if (e->help) {
		printf("\t str sf SPLI -> text /*Split the str using sf splitters into the text.*/ \n");
		return 0;
	}
    char line[MAX_STRING + 1] ;
    char* token ;
    _var_t * s, * f, * s0, * t; 
    char t_name[MAX_NAME_LENGTH];
    int i = 0, j, k, l = 0;
    int f_split = 0; 
    if (num_cells(e,2) && gt2(e) == STRI && gt1(e) == STRI)
    {
    	//Входная строка
    	s = (_var_t *) gs2(e);
    	f = (_var_t *) gs1(e); // Формат
    	drop1(e); drop1(e);
    	sprintf(t_name,"t_%s", s->name );
    	t = new_var(e, t_name, TEXT);
    	// Проходим строку и копируем в буфер
    	j=0;
    	for (i=0; i< (s->size); i++) // не учитываем последний 0
    	{
    		f_split = 0; // split flag
    		for (k=0; k<(f->size)-1; k++){
    			if ( ((char*)s->first_element)[i] == ((char*)f->first_element)[k] ||
    			 i == (s->size-1) ) f_split = 1;
    		}
    		if (!f_split) {
    			//Копируем символ
    			line[j] = ((char*)s->first_element)[i];
    			j++;
    		}
    		else{
    			//Обранужен сплит, сохраняем строку
    			if (j>0){ // Не сохраняем пустую строку.
	    			sprintf(t_name,"t_%s.%d", s->name, l++);
	    			s0 = new_var(e, t_name, STRI);
	    			line[j] = 0;
	    			set_string(s0, line);
	    			add_string_to_text(e, t, s0, POS_END);
	    			j=0;
	    		}
    		}
    	}
    	push_var_ptr_to_stack(e, t);
    }
	return 0;
}
/* Отрезает первую строку от текста*/
int wl0_CHOP(nicenv_t * e){
	if (e->help) {
		printf("\t str sd CHOP -> str' fword /*Return the heading fword, separated by sd splitters and cut it from initial string.*/ \n");
		printf("\t txt CHOP -> txt' fline /*Return the heading string, and delete the first line from initial txt.*/ \n");		
		printf("\t /* Returns VOID if the string or text is empty*/ \n");		
		return 0;
	}
    if (num_cells(e,2) && gt2(e) == STRI && gt1(e) == STRI)
    {
    	wl0_OFFS(e); //смещение по которому сечь строку
    	wl0_DUP(e);
    	push_int_number(e,2);
    	wl0_FLIP(e); //сохраняем значение офсета
    	wl0_DUPS(e); //дублируем строку.
    	push_int_number(e,1);
    	wl0_FLOP(e);
    	//wl0_DEC(e);
       	wl0_HEAD(e); //отрезали начало строки
    	wl0_SWAP(e); //начальная строка, надо отрезать у нее конец
    	wl0_SLEN(e);
    	wl0_DEC(e);
    	push_int_number(e,1);
    	wl0_FLOP(e);
    	wl0_SUB(e);
    	//wl0_INC(e);
    	wl0_TAIL(e);
    	wl0_SWAP(e);
    }
    return 0;
}

/* Возвращает количество тиков от стандартного начала времени */
int wl0_TICK(nicenv_t * e){
	if (e->help) {
		printf("\tTICK -> time /*The number of seconds since the standard time beginning*/ \n");
		return 0;
	}
	struct timeval start;
	if ( e->sp1 < MAX_STACK_SIZE ){
		gettimeofday(&start, NULL);
		ss(e, (void*) start.tv_sec);
		st(e, INT);
		isp1(e);
	}	
	return 0;
}
/* Возвращает количество тиков от стандартного начала времени */
int wl0_USEC(nicenv_t * e){
	if (e->help) {
		printf("\tUSEC -> usec /*The number of micro seconds since the standard time beginning*/ \n");
		return 0;
	}
	struct timeval start;
	if ( e->sp1 < MAX_STACK_SIZE ){
		gettimeofday(&start, NULL);
		ss(e, (void*) (start.tv_sec*1000000L + start.tv_usec));
		st(e, INT);
		isp1(e);
	}	
	return 0;
}
/* Преобразует целое чисто в строку*/
int wl0_NTOA(nicenv_t * e){
	if (e->help) {
		printf("\tn NTOA -> str /*Convert integer number into string*/ \n");
		return 0;
	}
	if ( num_cells(e, 1) && gt1(e) == INT && e->sp1 < MAX_STACK_SIZE ){
		pop_int(e);
		memset(e->string, 0, MAX_STRING);
		sprintf(e->string, "%ld", e->lint);
		add_string(e);
	}	
	return 0;
}
/* Задерка выполнения */
int wl0_WAIT(nicenv_t * e){
	if (e->help) {
		printf("\tn WAIT -> /* Wait for n ms*/ \n");
		return 0;
	}
	if ( num_cells(e, 1) && gt1(e) == INT ){
		pop_int(e);
		sleep(e->lint);
	}	
	return 0;
}

int wl0_INCH(nicenv_t * e){

	if (e->help) {
		printf("\tn INCH -> /* Converts size into inches*/ \n");
		return 0;
	}
	if ( num_cells(e, 1) && e->sp1 < MAX_STACK_SIZE ){
		e->dbl = 25.4;
		push_dbl_to_stack(e);
		wl0_DIV(e);
		wl0_DUP(e);
		wl0_PR(e);
	}	
	return 0;

}
int wl0_INMM(nicenv_t * e){	
	if (e->help) {
		printf("\tn INMM -> /* Converts size into mm*/ \n");
		return 0;
	}
	if ( num_cells(e, 1) && e->sp1 < MAX_STACK_SIZE){
		e->dbl = 25.4;
		push_dbl_to_stack(e);
		wl0_MUL(e);
		wl0_DUP(e);
		wl0_PR(e);
	}	
	return 0;
}
/*Создаем переменную в памяти со случайным именем*/
int wl0_UN(nicenv_t * e){
	if (e->help) {
		printf("\tUN -> unique_name /*Generates undefined variable with unique name*/ \n");
		return 0;
	}
	char un[MAX_NAME_LENGTH];
	_var_t * vn;
	if (e->sp1 < MAX_STACK_SIZE){
		generate_unique_name(e,"un", un);
		vn = new_var(e, un, UDEF);
		push_var_ptr_to_stack( e, vn);
	}
	return 0;
}

int wl0_STOK(nicenv_t * e){	
	if (e->help) {
		printf("\tnum_shares price delta STOK -> /*Calculates the upper and lower price to set*/ \n");
		return 0;
	}
	if ( num_cells(e, 3) && e->sp1 < MAX_STACK_SIZE){
		
		pop_double(e);
		double delta = e->dbl;
		pop_double(e);
		double price = e->dbl;
		
		pop_int(e);
		long total_share = e->lint;

		double up_price = (1.0 + delta/100) * price;
		double low_price = (1.0 - delta/200) * price;

		long tot_price = total_share * price;
		
		printf("\n\tPrice: %ld\n", tot_price);
		printf("\tUpper price: %lf\n", up_price);
		printf("\tLower price: %lf\n", low_price);

	}	
	return 0;
}

/*	
	Сохранить текущую текстовую переменную, если в ней есть строки, запустить внешний редактор, 
	сохранить буфер, который был и загрузить обратно ее в память.
*/
int wl0_EDIT(nicenv_t * e){
	if (e->help) {
		printf("\ttxt EDIT -> txt' /*Edit text in external editor*/ \n");
		return 0;
	}
	if ( num_cells(e, 1) && gt1(e) == TEXT &&  e->sp1 < MAX_STACK_SIZE){
		// создаем уникальное имя для файла в который будем сохранять текст.
		
		_var_t * t = (_var_t*) gs1(e);
		_var_t * s = NULL;
		char t_name[MAX_NAME_LENGTH];
		strcpy(t_name, t->name);
		char name[MAX_NAME_LENGTH];
		generate_unique_name(e,"_buff",name);
		memset(e->string,0,MAX_RAW_STRING);
		sprintf(e->string, "%s%s.nic", t->name, name);
		add_string(e);
		wl0_SAVE(e);
		memset(e->string,0,MAX_RAW_STRING);
		sprintf(e->string,"vim ./%s%s.nic", t->name, name);
		add_string(e);
		_var_t * str1 = (_var_t*) gs1(e); // сохраняем указательна строку
		wl0_SH(e);
		// удаляем содержимое буфера текста.
		int exec = t->exec; 
		t->exec = 0;// снимаем флаг
		push_var_ptr_to_stack(e,t);
		wl0_UNLO(e);
		wl0_DEL(e);
		wl0_VARL(e);
		// добавляем имя файла из которого будем грузить текст
		sprintf(e->string,"./%s%s.nic", t_name, name);
		add_string(e);
		_var_t * str2 = (_var_t*) gs1(e); // сохраняем указательна строку
		t = new_var(e, t_name, UDEF);
		push_var_ptr_to_stack(e,t);
		wl0_LOAD(e);
		t->exec = exec;
		// удадаляем буфер
		sprintf(e->string,"rm ./%s%s.nic", t_name, name);
		add_string(e);
		wl0_SH(e);
		// удаляем временные переменные
		push_var_ptr_to_stack(e,str1);
		push_var_ptr_to_stack(e,str2);
		wl0_DEL(e);
		wl0_DEL(e);

	}
	return 0;
}

int wl0_CIN(nicenv_t * e){
	if (e->help) {
		printf("\tCIN -> str /*Gets string from keyboard and place it into the stack*/ \n");
		printf("\t All words after 'CIN' till the end are executed before input from console starts\n");
		return 0;
	}
	
	memset(e->string,0,MAX_RAW_STRING);
	fgets(e->string, MAX_RAW_STRING-1, stdin);
	if (strlen(e->string) > 0 &&  e->sp1 < MAX_STACK_SIZE){
		e->string[strlen(e->string)-1] = 0; // удаляем перевод строки
		add_string(e);
	}

	return 0;
}

int wl0_STOP(nicenv_t * e){
	if (e->help) {
		printf("\tSTOP -> str /*Stop batch execution. Do not put any word after stop in the line. It will be lost after GO.*/\n");
		return 0;
	}
	e->stop = 1;
	return 0;
}
int wl0_GO(nicenv_t * e){
if (e->help) {
		printf("\tGO -> str /*Continue batch execution.*/\n");
		return 0;
	}
	e->stop = 0;
	return 0;
}
/*Вытягиваем кодовый сегмент слова и сохраняем его как строку в файл. */
int wl0_RIPC(nicenv_t * e){
	if (e->help) {
		printf("\tnum :word RIPC -> Displays num bytes of the binary in HEX\n");
		return 0;
	}
	_word_t * wptr = NULL;
	long nb = 0;
	if ( num_cells(e, 2) && ((gt1(e) == CODE) || (gt1(e) == INT)) && (gt2(e) == INT) ){
		wptr = (_word_t *)  gs1(e);
		unsigned char * w = (unsigned char *) (wptr->code);
		st1(e, 0);
		ss1(e, 0);
		dsp1(e);
		pop_int(e);
		nb = e->lint;
		// вытягиваем содержимое слова и печатаем его
		for (int i=0; i<nb; i++){
			if (!(i%16)) {
				printf("%p:\t", ((unsigned char*)w)+i);
			}
			printf("%02X ", ((unsigned char*)w)[i]);
			if ((i%16) == 15){
				printf("\t");
				for (int j=0; j<16; j++) {
					printf("%c", ((unsigned char*)w)[i-15+j]);
				}
				printf("\n"); 
			}
		}

	}
	printf("\n");
	return 0;
}

/*Сравнение строк*/
int wl0_CMPS(nicenv_t * e){
	if (e->help) {
		printf("\t s1 s2 CMPS-> -n|0|n /*String comparison similar to strcmp.*/\n");
		return 0;
	}
	if (num_cells(e,2) && STRI == gt1(e) && STRI == gt2(e)){
		_var_t * s2 = (_var_t*) gs1(e);		
		_var_t * s1 = (_var_t*) gs2(e);		
		drop1(e); drop1(e);
		e->lint = strcmp((char*)s1->first_element, (char*)s2->first_element);
		push_int_number(e,e->lint);
	}
	return 0;
}
int wl0_NOT(nicenv_t * e){
	if (e->help) {
		printf("\tn|0 NOT -> 0|1 /*Logical not.*/\n");
		return 0;
	}
	if (num_cells(e,1) ){
		if ( gt1(e) == INT){
			long a = (long) gs1(e);
			drop1(e);
			if (a!=0) push_int_number(e, 0);
			else push_int_number(e, 1);
			return 0;
		}
	}
	return 0;	
}

/*convert english word into number*/
int wl0_ETON(nicenv_t * e){
	if (e->help) {
		printf("\t s1 ETON -> n /*Convert word into number on the base of alphabet. Not case sensitive.*/\n");
		return 0;
	}
	if ( num_cells(e,1) && STRI == gt1(e) )
	{
		_var_t * s1 = (_var_t*) gs1(e);
		drop1(e);
		
		int i = 0;
		long long nn = 0;
		strncpy( e->string, (char*)s1->first_element, strlen((char*)s1->first_element) );
		e->lint = 0;
		while (e->string[i])
		{
		    nn *= 27; //база чисел
		    e->string[i] = tolower( e->string[i] );
		    //count the numbers
		    nn += (e->string[i]) - (int)'a' + 1;
		    printf("%lld\n", nn);
		    i++;
		}
		printf("[%s]\n", e->string);
				
		//push_int_number(e,e->lint);
	}
	return 0;
}

int wl0_BIN(nicenv_t * e){
	if (e->help) {
		printf("\t n BIN -> n /*Convert a number into binary string*/\n");
		return 0;
	}
	char bn[MAX_BITS_LINT]; // the number of bits in the long number
	int i;
	if ( num_cells(e,1) && INT == gt1(e) )
	{
		pop_int(e);
		for (i = MAX_BITS_LINT-1; i>=0; i--){
			bn[i] = '0';
			if (e->lint & 1) {
				bn[i] = '1';
			}
			e->lint >>= 1;
		}
		memset(e->string, 0, MAX_STRING);
		sprintf(e->string, "%s", bn);
		add_string(e);
	}
}

int wl0_HEX(nicenv_t * e){
	if (e->help) {
		printf("\t n HEX -> n /*Convert a number into hex string*/\n");
		return 0;
	}
	char bn[8+1]; // the number of hex digits in the long number
	int i;
	unsigned char j = 0;
	if ( num_cells(e,1) && INT == gt1(e) )
	{
		pop_int(e);
		for (i = 7; i>=0; i--){
			bn[i] = '0';
			j = (unsigned char) (e->lint & 0x0F);
			if ( j < 10 ){
				bn[i] += j;
			}
			else{
				bn[i] = 'A' + j - 10;
			}
			bn[8] = 0;
			e->lint >>= 4;
		}

		memset(e->string, 0, MAX_STRING);
		sprintf(e->string, "%s", bn);
		add_string(e);
	}
}
