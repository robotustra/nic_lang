/* Main file containing nic_shell()*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <ctype.h>
#include "common.h"
#include "sh_inner.h"/*В этом файле используются все переменные для функций внутри ф. шелла*/

nicenv_t env;

#define cursorforward(x) printf("\033[%dC", (x))
#define cursorbackward(x) printf("\033[%dD", (x))
char * prompt[5] = {
	"nic(i)>",
	"nic(e)>",
	"nic(r)>",
	"nic(w)>",
	"nic(d)>"
};

int getch_(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( 0, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( 0, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( 0, TCSANOW, &oldattr );
    return ch;
}
/* reads from keypress, echoes */
int getche_(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( 0, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( 0, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( 0, TCSANOW, &oldattr );
    return ch;
}

int is_line_finished(nicenv_t * e, int key, char * inp_buff, int * pos){
	if ( key == 10 && e->string_flag_pp == 0 && e->quote_flag == 0 ) {
		//printf("\n");
		return 1;
	} else 
	if ( key == 10 && (e->string_flag_pp == 1 || e->quote_flag == 1 ) )
	{
		*pos = ins_at_pos( inp_buff, *pos, '\\') ;
		*pos = ins_at_pos( inp_buff, *pos, 'n') ;
	}
	return 0;
}

int is_escape(int key){
	if (key == 27) return 1;
	return 0;
}
/*
* 	Удаляем сомвол из позиции Pos и смещаем весь буфер слева направо на 1 символ
*/
int del_at_pos(nicenv_t * e, char * inp_buff, int pos){
	if (inp_buff[pos-1] == '"') {
		if (e->string_flag_pp == 1) e->string_flag_pp =0;
		else e->string_flag_pp = 1;
	}
	if (inp_buff[pos-1] == '\'') {
		if (e->quote_flag == 1) e->quote_flag =0;
		else e->quote_flag = 1;
	}
	strcpy (&(inp_buff[pos-1]),&(inp_buff[pos]));
	return pos-1;
}
	
/*
* 	Обрабатываем эскейп последовательности. их не так много.
*/
int process_esc_sequence(nicenv_t * e, int pos, char * inp_buff){
	/*
	ESC 	27 27				-- сбрасываем буфер
	F1		27 79 80			-- пока ничего не делаем для F1-F4
	F2		27 79 81			
	F3		27 79 82
	F4		27 79 83
	F5		27 91 49 53 126
	F6		27 91 49 55 126
	F7		27 91 49 56 126
	F8		27 91 49 57 126
	F9		27 91 50 48 126
	F10		27 91 50 49 126
	F11		27 91 50 51 126
	F12		27 91 50 52 126
	INS 	27 91 50 126
	HOME	27 91 72			-- перемещаем курсор в начало, но буфер на сбрасываем
	PGUP	27 91 53 126
	PGDN	27 91 54 126
	END 	27 91 70			-- переходим в конец строки
	DEL 	27 91 51 126		-- удаляем следующий символ
	UP 		27 91 65
	LEFT 	27 91 68			-- смещаем курсор влево до начала
	DOWN	27 91 66
	RIGHT 	27 91 67			-- курсор вправо до конца введенной строки
	*/
	int key = 27, key2, key3;
	int ch;
	key2 = getch_();
	if (key2 == 27) {
		memset(inp_buff, 0, MAX_INPL_SIZE);
		pos = 0;
		return pos;
	}
   	key3 = getch_();
   	if (key2 == 79){
   		if (key3 >= 80 && key3 <= 83 ) ;;//printf("\n[F1-4]\n");
   	}else
   	if (key2 == 91){
   		if (key3 == 49) { 
   			key2 = getch_();
   			//while ((ch = getchar()) != '\n' && ch != EOF);
   			if (key2 >=53 && key2 <= 57) {key2 = getch_ (); /*printf("\n[F5-8]\n");*/ } // читаем еще раз чтобы очистить буфер
   		}
   		if (key3 == 50) { 
   			key2 = getch_();
   			//while ((ch = getchar()) != '\n' && ch != EOF);
   			if (key2 >=48 && key2 <= 52) {key2 = getch_ (); /*printf("\n[F9-12]\n");*/ } // читаем еще раз чтобы очистить буфер
   			else
   			if (key2 == 126) {/*ins*/ /*printf("\nINS\n");*/;;}
   		}
   		if (key3 == 51) {
   			key2 = getch_();
   			if (key2 == 126) {/*DEL*/ if (pos < strlen(inp_buff)) {pos++; pos = del_at_pos(e, inp_buff, pos); }; }
   		}
   		if (key3 == 68) {/*left*/ if (pos > 0) pos --; }
   		if (key3 == 67) {/*right*/ if (pos < strlen(inp_buff)) pos ++; }
   		if (key3 == 70) {/*end*/ pos = strlen(inp_buff); }
   		if (key3 == 72) {/*home*/ pos = 0; }
   	}/*else
   	printf(" ('%c', %d), ('%c', %d), ('%c', %d)\n", (key==27)? '^': (char)key, key, (char) key2, key2, (char) key3, key3 ); 
   	*/
   	return pos;
}
/*
* 	Эта функия печатает цветами все, что содержится в текущий момент.
*/


int print_one_category(int cat, char * line){
	switch (cat){
		case VARIABLE_CAT: printf( ANSI_COLOR_WHITE_HIGH "%s" ANSI_COLOR_RESET, line); break;
		case COMMENT_CAT: printf( ANSI_COLOR_BLUE_HIGH "%s" ANSI_COLOR_RESET, line); break;
		case NUMBER_POWER_CAT:
		case WORDS_CAT: printf( ANSI_COLOR_YELLOW_HIGH "%s" ANSI_COLOR_RESET, line); break;
		case STRING_CAT:
		case DQUOTE_CAT: printf( ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET, line); break; 
		case NUMBER_CAT: printf( ANSI_COLOR_GREEN_HIGH "%s" ANSI_COLOR_RESET, line); break; 
		case ERROR_CAT: printf( ANSI_COLOR_RED_HIGH "%s" ANSI_COLOR_RESET, line); break;
		case S_ESCAPE_CAT: printf( ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, line); break;
		case E_ESCAPE_CAT: printf( ANSI_COLOR_CYAN_HIGH "%s" ANSI_COLOR_RESET, line); break;
		case NUMBER_SIGN_CAT:
		case NUMBER_POWER_SIGN_CAT:
		case NUMBER_POWER_NUM_DIG_CAT: printf( ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET, line); break;

		default: printf( ANSI_COLOR_WHITE_HIGH "%s" ANSI_COLOR_RESET, line); break;
	}
	return 0;
}

void inplace_reverse(char * str)
{
  if (str)
  {
    char * end = str + strlen(str) - 1;

    // swap the values in the two given variables
    // XXX: fails when a and b refer to same memory location
#   define XOR_SWAP(a,b) do\
    {\
      a ^= b;\
      b ^= a;\
      a ^= b;\
    } while (0)

    // walk inwards from both ends of the string, 
    // swapping until we get to the middle
    while (str < end)
    {
      XOR_SWAP(*str, *end);
      str++;
      end--;
    }
#   undef XOR_SWAP
  }
}

/* 	
* 	Возвращает строку из символов одной категории, между разделителем и pos.
*/
int get_guess_category_string(nicenv_t * e, int pos, char * inp_buff, char * inp_cat, char * outline){
	int i, j = 0;
	memset (outline, 0, MAX_INPL_SIZE);
	if (pos > 0){ // выдаем подсказку если в строке есть хотя бы один символ, отличный от разделителя
		// категория символов, принадлежащих к строке поиска может быть VARIABLE_CAT,
		// WORDS_CAT, TYPE_CAT
		if ((pos == strlen(inp_buff) || (pos < strlen(inp_buff) && inp_cat[pos] == SPACER_CAT ))
			&& (inp_cat[pos-1] == VARIABLE_CAT || inp_cat[pos-1] == WORDS_CAT )){
			// выделяем символы одной карегории между курсором и разделителем
			j=0;
			for(i=pos-1; i>=0; i--) {
				//if (inp_cat[i] != SPACER_CAT || inp_buff[i] == ' ') { //// bug !!! возвращает почему-то больше слов????
				if (inp_cat[i] == VARIABLE_CAT || inp_cat[i] == WORDS_CAT) { //// bug !!! возвращает почему-то больше слов????
					outline[j++] = inp_buff[i]; outline[j] = 0;
					//printf("[%d,%d]", inp_buff[i],inp_cat[i] );
				}
				else
					break;
			}
			inplace_reverse(outline); // !! почему-то иногда копирует больше Одного слова
			//printf("reverse: [%s] \n", outline );
		}
		
	}
	return 0;
}

/*
*	Печатаем возможную посказку в outstr
*/
int guess_next_word(nicenv_t * e, char * word, char * outstr){
	int i;
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина слова не может быть больше 32
	int j = 0;
	memset(tstr, 0, MAX_NAME_LENGTH);
	memset (outstr, 0, MAX_STRING);

	for (i=0; i<strlen(word); i++)
	{
		tstr[i] = toupper(word[i]); // У нас только 4 символа во встроенных словах.
	}
	for (i=0; i<e->words_root.n_words; i++){
		//printf("Comparing [%s] --- [%s]\n", (e->words_root.word_names_index)[i], tstr);
		if (strlen(tstr) > 1){ // по одному символу
			if (strncmp((e->words_root.word_names_index)[i], tstr, strlen(tstr)) == 0) {
				if (j== 0) {sprintf(outstr, "#"); j = 1;}			
				if (strlen(outstr) < (MAX_STRING- strlen((e->words_root.word_names_index)[i])) )
				{
					sprintf(outstr, "%s[%s]", outstr, (e->words_root.word_names_index)[i] );
				}
			}
		}
	}

	return 0;
}

/*
* 	Возвращает outstr содержащую необходимые символы для дописывания слова.
*/
int guess_completion_string(nicenv_t * e, char * word, char * outstr, int tabcount){
	int i, j=0, m=0;
	char tstr[MAX_NAME_LENGTH]; // Максимальная длина слова не может быть больше 32
	// word содержит оригинальную версию строки, она нужна для поиска в списке переменных
	// поскольку они кейс сенситив
	memset(tstr, 0, MAX_NAME_LENGTH);
	for (i=0; i<strlen(word); i++)
	{
		tstr[i] = toupper(word[i]); // У нас только 4 символа во встроенных словах.
	}
	// находим число совпадений по всему списку
	j = 0;
	for (i=0; i<e->words_root.n_words; i++){
		if (strlen(tstr) > 1){
			if (strncmp((e->words_root.word_names_index)[i], tstr, strlen(tstr)) == 0) {
				j++;
			}
		}
	}
	// выбираем нужную строку
	for (i=0; i<e->words_root.n_words; i++){
		//printf("Comparing [%s] --- [%s]\n", (e->words_root.word_names_index)[i], tstr);
		if (strlen(tstr) > 1){
			if (strncmp((e->words_root.word_names_index)[i], tstr, strlen(tstr)) == 0) {
				m++;
				if (m == (tabcount%j+1) ) {
					sprintf(outstr, "%s", (e->words_root.word_names_index)[i] );
					return 1;
				}
			}
		}
	}
	return 0;
}


/*
*	Добиваем незаконченное слово, там где стоит курсор.
*/
int auto_complete_word(nicenv_t * e, int pos, char * inp_buff, char * inp_cat, int tcount, int * nc)
{
	char word[MAX_INPL_SIZE];
	char outstr[MAX_INPL_SIZE];

	get_guess_category_string(e, pos, inp_buff, inp_cat, word);// получаем фрагмент слова в word

	if (strlen(word) == 0) return pos; // если строка поиска пустая ничего не делаем

	if (tcount > 1) {
		//printf("\nlooking for the next word\n");
		if ( (*nc) > 0) {
			// производим поиск по первым двум символам найденого слова
			//printf("\nnc = %d, {{%s}}\n", *nc,  word);
			// корректируем строку поиска для второго раза
			pos -= strlen(word); // корректируем позицию в стоке ввода
			word[*nc] = 0; // сокращаем до первоначального числа символов
			pos += strlen(word);
		}
	}
	if( (*nc) == 0) *nc = strlen(word);
	
	int res = guess_completion_string(e, word, outstr, tcount); // получаем строку в outstr
	if (res) {
		//делаем подстановку
		pos = pos - strlen(word);
		
		strncpy (&(inp_buff[pos]), outstr, strlen(outstr));
		pos += strlen(outstr); 

		for (int i=pos; i<MAX_INPL_SIZE; i++ ) inp_buff[i] = 0; // просто перетираем все если было автодополнение
	}

	return pos;
}

/*
 	Печатаем буфер красиво.
 	inp_buff - содержимое вводимой строки.
 	pos - текущая позиция курсора
 	inp_cat - вспомогательный массив категорий символов в веденной строке, для подсветки категорий.
 	prev_len - длина предыдущей строки.
*/
int print_buffer_nice(nicenv_t * e, char* inp_buff, int pos, char * inp_cat, int * prev_len, int enter_flag ){
	
	char outline[MAX_INPL_SIZE];
	char prompt_str[MAX_STRING];
	int cat_id;
	int new_pos;
	int i;

	if (enter_flag) {
		pos = ins_at_pos( inp_buff, pos, ' ');
	}
	// каждый раз начинаем сначала потому, что мы можем исправлять строку в любой позиции
	update_symbol_category(e, 0, inp_buff, inp_cat);
	
	//printf("\n");
	known_names_look_up(e, inp_buff, inp_cat);

	get_guess_category_string(e, pos, inp_buff, inp_cat, outline);
	
	memcpy(e->preparse_line, inp_cat, strlen(inp_buff)); // копируем категории для того, чтобы их потом использовать в парсинге
	printf( ANSI_COLOR_WHITE "\r%s" ANSI_COLOR_RESET, prompt[e->shell_mode] );

	
	new_pos = 0;
	while( new_pos >= 0 ){
		new_pos = get_next_category_string( e, new_pos, inp_buff, inp_cat, outline, &cat_id);
		print_one_category(cat_id, outline);
	}

	
	// подготавливаем место для подсказки
	for ( i=0; i<=(*prev_len) + 7; i++) printf(" "); //очищаем подсказку
	cursorbackward( *prev_len + 1 + 7) ;

	if (enter_flag) { printf("\n"); return 0; }

	if (cat_id == VARIABLE_CAT || cat_id == WORDS_CAT){
		guess_next_word(e, outline, prompt_str);
		print_one_category(COMMENT_CAT, prompt_str);
	}
	/*if (strlen(prompt_str) > (*prev_len))*/ *prev_len = strlen(prompt_str) + 1;	
	printf("\r"); 
	cursorforward(7+pos); // 7 = длина prompt
	

	fflush(stdout);
	return 0;
}
/*
*  Вставляем символ в позицию курсора, если курсор не в конце строки, то смещаем всю строку на 1 символ
*/
int ins_at_pos( char * inp_buff, int pos, int key){
	if (pos < strlen(inp_buff)) {
		for (int i = strlen(inp_buff) - 1; i>=pos ; i--){
			inp_buff[i+1] = inp_buff[i];
		}
	}
	inp_buff[pos] = key;
	return pos+1;
}

/*
*	Функция читает посимвольно с входной строки и подсвечивает синтаксис.
* 	По окончании ввода она передает входной буфер для парсинга без подсветки.
* 	Буфер имеет ограничение по длине, просто потому, что я так хочу.
*/
int read_line_color(nicenv_t * e, char* inp_buff ){
	int key, key2, key3;
	int pos = 0; // позиция курсора
	char inp_cat[MAX_INPL_SIZE]; // категории подсветки символов
	int prev_len = 0;
	int tcount = 0; // счетчик табов
	int nc = 0; // число символов с замененной строке

	memset(inp_cat, 0, MAX_INPL_SIZE);

    while (1) {
        key = getch_(); key2 = 0; key3 = 0;
        if (is_line_finished(e, key, inp_buff, & pos)){
        	print_buffer_nice(e, inp_buff, pos, inp_cat, &prev_len, 1);
        	tcount = 0;
        	nc = 0; // сбрасываем счетчик символов
        	//Сохраняем буфер и передаем управление
         	break; // отправляем сообщение
        }
        if (key == 127) { // backspace
        	if (pos>0) pos = del_at_pos(e, inp_buff, pos); 
        	tcount = 0;
        	nc = 0; // сбрасываем счетчик символов
        }
        if (is_escape(key)){
        	// обрабатываем эскейп последовательности
        	pos = process_esc_sequence(e, pos, inp_buff); // изменяем содержимое буфера в засимости от нажатых клавиш
        	// стрелки могут вызывать историю или перемещаться по строке, пока курсор только в начале строки.
        	tcount = 0;
        	nc = 0; // сбрасываем счетчик символов
        }
        if (key == '\t') {
        	// будем дописывать строки по известным спискам
        	tcount ++;
        	pos = auto_complete_word(e, pos, inp_buff, inp_cat, tcount, &nc);
        }
        if ( (isprint(key) || isspace(key) ) && key != '\t' && key !=10 ) {
        	if (strlen(inp_buff) < MAX_INPL_SIZE-1){
        		// вставляем на месте курсора
        		pos = ins_at_pos( inp_buff, pos, key) ;
        	} else
        	{ 	// буфер закончился, отправляем строку на парсинг
        		break;
        	}
        	tcount = 0;
        	nc = 0; // сбрасываем счетчик символов
        }
        print_buffer_nice(e, inp_buff, pos, inp_cat, &prev_len, 0); // enter_flag = 0
    }
	return 0;
}

/*
*	Функция получает 1 строчку с терминала и помещает ее в строку ввода, переменную
*	env.input, которая является переменной типа TEXT
*/
int get_line(nicenv_t * e)
{
	unsigned long h;
	char inp_buff[MAX_INPL_SIZE];
	char str_name[MAX_NAME_LENGTH];
	_var_t * v_n = NULL, *t = NULL, *f_v = NULL;
	int i;
	e->string_flag_pp = 0;
	e->quote_flag = 0;
	memset(inp_buff, 0, MAX_INPL_SIZE);
	memset(e->preparse_line, 0, MAX_INPL_SIZE);
	if (e->stop) goto man_input;
	// проверяем, может у нас есть неинтерпретированные строки из самого inpl
repe:
	t = e->vars_root.first;
	f_v = is_var_name_exists(e,"file_inpl"); // означает, что у нас есть файл на входе.
	// сперва проверяем наличие слов в текстовом интерпретаторе.
	if(t!=NULL && t->num_element > 1){
#if (DEBUG_LEVEL == 5)		
		printf("Have somethig to interpret!\n");
		for (i=1; i< t->num_element; i++){
			printf("%s\n", (char*)((_var_t**)t->first_element)[i]->first_element );
		}
#endif		
		scroll_text(e, t, 1); // подвигаем текст на 1
#if (DEBUG_LEVEL == 5)		
		printf("New string:'%s'\n", (char*)(((_var_t **)t->first_element)[0])->first_element);
#endif		
		strcpy(inp_buff, (char*)(((_var_t **)t->first_element)[0])->first_element);
	}
	// если интерпретатор пустой, проверяем, нет ли у нас вложенных циклов
	else if ( (e->loop_cnt > 0) && (e->nest[e->nest_cnt-1] == 'L') ){
		// означает, что у нас есть вложенный цикл, который нужно выполнить
		if(e->test_cond == 0){
			// проверяем, означает, что сперва нужно выполнить условие.
			_var_t * cond = e->loop_cond[e->loop_cnt - 1]; // условие которое нужно проверить.
			if (cond == NULL || cond->first_element == NULL){
				e->loop_cnt --; // поврежденное условие, выходим из цикла
				e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt>0) e->nest_cnt--;
				goto repe;				
			}
			// надо заспулить все строки из этого условия в inpl
			spool_code_to_inpl(e, cond);
			e->test_cond = 1; // устанавливаем флаг проверки условия.
			goto repe;
		}else{
			e->test_cond = 0; // сбрасываем флаг проверки условия
			// была сделана проверка условия, необходимо посмотреть что на вершине стека, если там 0, то выходим из цикла,
			// если не 0, то запускаем новый цикл.
			if ( 0 == check_condition(e) ){
				// условие ложно, уменьшаем вложенность цикла на 1
				_var_t * end = e->loop_end[e->loop_cnt - 1]; // условие которое нужно проверить.
				if (end == NULL || end->first_element == NULL){
					e->loop_cnt --; // поврежденное заключение, выходим из цикла
					e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; 
					goto repe;				
				}
				spool_code_to_inpl(e, end); // выполнение заключения
				e->loop_cnt --;
				if (e->loop_cnt >= 0){
					e->loop_cond[e->loop_cnt] = NULL;
					e->loop_body[e->loop_cnt] = NULL;
					e->loop_end[e->loop_cnt] = NULL;
				}
				e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; 
			}else
			{
				// условие неложное, поэтому выполняем тело цикла
				_var_t * body = e->loop_body[e->loop_cnt - 1];
				if (body == NULL || body->first_element == NULL){
					e->loop_cnt --; // поврежденное заключение, выходим из цикла
					e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; // сбрасываем стек
					goto repe;				
				}
				spool_code_to_inpl(e, body);
			}
			goto repe;
		}
	}
	// если интерпретатор пустой, проверяем нет ли у нас в спулере run
	else if ( (e->run_cnt > 0) && (e->nest[e->nest_cnt-1] == 'R') ){
		_var_t * body = e->run_body[e->run_cnt - 1];
		if (body == NULL || body->first_element == NULL){
			e->run_cnt --; // поврежденное заключение, выходим из цикла
			e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; 
			goto repe;				
		}
		// body может быть или строкой или текстом, поэтому нужно скопировать следующую строку в inp_buff и если текст закончился, то 
		// уменьшить run_cnt на единицу.
		if (body->type.type_index == STRI) {
			// просто спулим строку в inp_buff
			strcpy(inp_buff, (char*)(body->first_element));
			e->run_cnt --;
			e->run_body[e->run_cnt] = NULL;
			e->run_line[e->run_cnt] = 0;
			e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; 
		}else 
		if (body->type.type_index == TEXT){
			// копируем следующую строку если текущий индекс строки меньше длины текста.
			int lcnt = e->run_line[e->run_cnt - 1];
			if ( lcnt < body->num_element){
				// спулим текущую строку в inp_buff
				strcpy(inp_buff, (char*)(((_var_t **)body->first_element)[ lcnt ])->first_element);
				//printf("inp =  [%s]\n", inp_buff);
				e->run_line[e->run_cnt - 1] ++;
				if (e->run_line[e->run_cnt - 1] == body->num_element){
					// проспулили все строки, нужно удалить ссылку на текст.
					e->run_cnt --;
					e->run_body[e->run_cnt] = NULL;
					e->run_line[e->run_cnt] = 0;		
					e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--;
				}
			}
		}
		inp_buff[strlen(inp_buff)] = '\n';
	}
	// if-else спулер
	else if ( (e->ifelse_cnt > 0) && (e->nest[e->nest_cnt-1] == 'I') ){
		// проверяем условие
		if (e->if_cond_test == 0){
			// проверяем, означает, что сперва нужно проверить условие.
			_var_t * cond = e->if_cond[e->ifelse_cnt - 1]; // условие которое нужно проверить.
			if (cond == NULL || cond->first_element == NULL){
				e->ifelse_cnt --; // поврежденное условие, выходим из цикла
				e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--;
				goto repe;				
			}
			// надо заспулить все строки из этого условия в inpl
			spool_code_to_inpl(e, cond);
			e->if_cond_test = 1; // устанавливаем флаг проверки условия.
			//goto repe;
		}else{
			e->if_cond_test = 0; // сбрасываем флаг проверки условия
			// если не 0, то запускаем новый цикл.
			_var_t * body = NULL;
			if ( 0 == check_condition(e) ){
				// условие ложно, выполняем условие else, если оно ненулевое
				body = e->else_body[e->ifelse_cnt - 1]; // условие которое нужно проверить.
			}else
			{
				// условие неложное, поэтому выполняем тело цикла
				body = e->if_body[e->ifelse_cnt - 1];
			}
			if (body == NULL || body->first_element == NULL){
				e->ifelse_cnt --; // поврежденное заключение, выходим из цикла
				e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; 
				goto repe;				
			}
			spool_code_to_inpl(e, body);
			e->ifelse_cnt --;
			if (e->ifelse_cnt >= 0){
				e->if_body[e->ifelse_cnt] = NULL;
				e->else_body[e->ifelse_cnt] = NULL;
				e->if_cond[e->ifelse_cnt] = NULL;
			}
			e->nest[e->nest_cnt-1] = 0; if (e->nest_cnt > 0) e->nest_cnt--; 
			//goto repe;
		}
		goto repe;
	}
	else if (f_v != NULL && f_v->num_element > 0)// проверяем не пустая ли переменная file_inpl?
	{
		// копируем первую строчку из этого файла в inpl
		strcpy(inp_buff, (char*)(((_var_t **)f_v->first_element)[0])->first_element);
		scroll_text_remove(e, f_v, 1); // скроллируем на 1 для следующего раза
	}
	else
	{
man_input:
		// Если нет текстового ввода, печатаем промпт
		printf("%s", prompt[e->shell_mode] ); fflush(stdout);
	}
	if (strlen(inp_buff)==0) 
	{	// Если строка пустая
		read_line_color(e, inp_buff); // Пока читаем только ввод с клавиатуры
	}
	// Очищаем от конца строки
	for (i=strlen(inp_buff); i>=0; i--){
		//if (inp_buff[i] != 10) inp_buff[i] = 0; // если есть мусор в конце то он мешает
		if (inp_buff[i] == 10 || inp_buff[i] == 13 ){ inp_buff[i] = 0; }
	}
	if (strlen(inp_buff) == 0) /*{inp_buff[0] = '\n';}*/return EMPTY_LINE;
	// Создаем только одну строку, и назовем ее line0
	if (e->input->num_element == 0) {
		v_n = is_var_name_exists(e, "line0");
		if (v_n == NULL){
			sprintf(str_name,"line0");
			v_n = new_var(e, str_name, STRI);
		}
		//3) Добавляется в список текста. Строки в структуре текста расположены в прямом порядке.
		add_string_to_text(e, e->input, v_n, POS_END);	
	}
	//printf("Adding text to string: [%s]\n", inp_buff );
	v_n = is_var_name_exists(e,"line0");
	if (v_n != NULL){
		set_string(v_n, inp_buff); // обновляем содержимое строки.
	}
	return 0;
}

/*
* 	Зополняем структуру оболочки.
*/
int shell_init(nicenv_t * e)
{
	int r;
#if (DEBUG_LEVEL >=4)	
	printf("Init shell structures...\n");
#endif
	e->sp1 = 0;
	e->osp1 = 0;	
	e->sp2 = 0;
	e->words_root.level = 0;
	e->words_root.first = NULL;
	e->words_root.ext_library = NULL;	// Никаких слов не загружено еще.
	e->vars_root.level = 0;
	e->vars_root.first = NULL;
	e->max_type_index = 0;
	e->types_root.level = 0;
	e->types_root.first = NULL;
	e->types_root.n_types = 0;
	e->shell_mode = INTERPRETER_MODE;
	e->max_type_index = MAX_TYPE_INDEX;
	e->next_word = NULL;
	e->line_pointer = 0;
	e->line_to_parse = 0;
	e->line_commited = 0;
	e->col_commited = 0;
	e->col_pointer = 0;
	e->string_flag = 0; // используется только при парсинге.
	e->string_flag_ready = 0; // ввод строки закончен
	e->escape_flag = 0;
	e->help = 0;
	e->var_counter = 0;
	e->avar = 0;	// автоматические переменные
	e->loop_cnt = 0; // счетчик вложенных циклов. Он увеличивается командой LOOP или REPE и уменьшается когда условие становится ложным.
	e->test_cond = 0; // флаг проверки условия. Означает, что последнее выполненное слово было проверка условия цикла.
	e->run_cnt = 0; // счетчик вложенных run, построчный спулер скриптов
	e->ifelse_cnt = 0;
	e->nest_cnt = 0;
	e->stop = 0; // приостановка выполенения кода. переход в построковый режим. Все что осталось в переменной inpl не выполняется.

	//_var_t input - инициализировать переменную типа текст.
	e->input = new_var(e, "inpl", TEXT);
	// line0 create
	new_var(e, "line0", STRI);
	//printf("Input variable created.\n");
	// Слова 0 уровня.
	if (e->recompile_flag) compile(0, "wordl0");
	
	r = load_words(e, 0); // указатель на библиотеку.
	if (r == 0) add_words_to_list(e, 0);
	else {
		perror("Can't load level 0 words, exiting...");
		exit(1);
	}
	return 0;
}
int get_lines_from_file(nicenv_t * env, char* file_input){
#if (DEBUG_LEVEL == 4)	
	printf("Executing file [%s]\n", file_input);
#endif	
	/*
		Проверяем существование текстовой переменной inpl 
		Добавляем в нее текст программы по строкам.
	*/
	FILE* l_fp ;
    char line[MAX_STRING + 1] ;
    char* token ;
    _var_t * t, * s0;
    char str_name[MAX_NAME_LENGTH];
    int i = 0;
    /* inpl уже существует, нужно его найти */
    //t = (_var_t *) is_var_name_exists(env, "inpl");
    t = (_var_t *) new_var(env, "file_inpl", TEXT);

    l_fp = fopen( file_input , "r" ) ;
    if (l_fp == NULL) 
    {
    	perror("Open file failed");
    	exit (1);
    }
	//printf("File %s is opened\n",  file_input);   
	// Добавляем line0 в текст impl 
	//sprintf(str_name,"file_line0");
	//s0 = new_var(env, str_name, STRI);
	//add_string_to_text(env, t, s0, POS_END); 
    while( fgets( line, MAX_COUNT, l_fp ) != NULL )
    {
    	sprintf(str_name,"file_line%d", ++i);
    	//printf("add var [%s]\n", str_name);
    	s0 = new_var(env, str_name, STRI);
    	if (s0 == NULL) {
    		return -1;
    	}
		//printf("Line[%d]:%s\n", i, line);	
		if (strlen(line) >0 ){        
			//line[strlen(line)-1] = 0;
			set_string(s0, line); 
			add_string_to_text(env, t, s0, POS_END);
			line[0] = 0;
		}
    }
    //printf("Line[%d]:%s\n", i, line);
    //printf("Lines loaded: %d\n", i);
    fclose(l_fp);
	return 0;
}
int clear_input(nicenv_t * e){
	int i=0;
	long idx = e->line_to_parse;
	_var_t * err_line = ((_var_t**)(e->input->first_element))[idx];
	printf("Parsing error at the line %d\n", (int) e->line_to_parse );
	printf("In the line: \n%s\n", (char*) err_line->first_element);
	for (i=0; i<e->col_pointer-1; i++) putchar('_');
	putchar('^');

	printf("\nError at col %d.\n", (int)e->col_pointer);

	printf("Clearing input\n");
	// Тут надо удалить последнюю строку и удалить переменную из кольца переменных.
	
	// Удаляем все строки в массиве ввода после строки, которую надо пасить.
	printf("line_to_parse :%d\n", (int)e->line_to_parse );
	printf("commit line: %d\n", (int)e->line_commited);
	printf("commted col: %d\n", (int)e->col_commited);
	// Если строка была хоть как-то частично распаршена, и какое-то из выражений
	// было вополнено, то сохраняем эту часть строки.
	if (e->col_commited > 0) {
		// Удаляем только коней строки, а не всю строку
		trim_string_in_text_after(e, e->input, e->line_commited, e->col_commited);
	}else {
		// Удаляем всю строку.
		del_strings_from_text_after(e, e->input, e->line_to_parse);
	}
	e->line_pointer = e->line_commited;
	e->col_pointer = e->col_commited;
	e->line_pointer = e->line_to_parse;
	printf("LP: %d\n", (int)e->line_pointer);

	//e->col_pointer = 0;

	e->escape_flag = 0; // Сбрасываем флаг 
	return 0;
}
/*
*	Новый символ не является пока ничем, значение сохраняется в строке,
*	и создается переменная с этим именем.
*/
/*
int add_new_symbol(nicenv_t * e)
{
	_var_t * v_n = NULL;
	printf("Adding new symbol: ");
	if ( strlen(e->string) > 0 ){
		printf("[%s]\n", e->string);
		v_n = new_var(e, e->string, UDEF);
		push_var_ptr_to_stack(e, v_n);
		memset(e->string, 0, MAX_RAW_STRING);
	}
	e->escape_flag = 0; // Сбрасываем флаг 
	return 0;
}	

int process_var(nicenv_t * e){
	_var_t * v_t = NULL;
	printf("Process var\n");
	// Добавляем указатель на переменную в стек.

	v_t = e->var; // Должна содержать имя переменной
	if (v_t != NULL){
		push_var_ptr_to_stack(e, v_t);
	} else
	{
		printf("Error: can't get variable by name!\n");
	}

	e->escape_flag = 0; // Сбрасываем флаг и игнорируем его
	return 0;
}
*/
/*int process_ctrl_word(nicenv_t * env){
	printf("Process control word\n");
	return 0;
}*/
int code_run_mode(nicenv_t * env){
	printf("Run mode\n");
	return 0;
}
int execute_code(nicenv_t * env){
	printf("Execute code\n");
	return 0;
}
int load_word(nicenv_t * env){
	printf("Load word\n");
	return 0;
}
/*
*	Оболочка.
*/
int nic_shell(char * file_input, int recompile_flag)
{
	int parse_res = END_OF_PARSE;
	int retcode = 0;
	int r = END_OF_PARSE;

	nicenv_t * e = &env;

	e->recompile_flag = recompile_flag;
	//memset (e->ext_path, 0, MAX_STRING);
	//strncpy(e->ext_path, path, strlen(path)-3); // remove "nic" in the end of line
	//printf("LIB_PATH=%s\n", e->ext_path);
	shell_init(e);

	if (NULL != file_input){
		get_lines_from_file(e, file_input);
	}

input:
	r = get_line(e);

	if(EMPTY_LINE == r) goto input;

parse:

	parse_res = parse_line(e);

	if (EXIT == parse_res)					goto bye;
	else if (END_OF_PARSE == parse_res)		goto input;
	
	goto parse;

bye: 
	unload_words(e, 0);

	return 0; 
}