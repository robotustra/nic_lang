#include "common.h"

/*Создаем переменную*/
int generate_unique_name (nicenv_t * e, char * prename, char * name);
_var_t * new_var(nicenv_t * e, char * name, int type);
/*Новый тип*/
_newtype_t * new_type(nicenv_t * e, char * type_name, long type_size );
/*Тестируем кольцо переменных*/
int print_var(nicenv_t * e);
/*Проверка существования переменных*/
_var_t * is_var_name_exists(nicenv_t * e, char * name);
_var_t * prev_var(nicenv_t * e, _var_t * v_t);

/*Удаление переменных*/
int del_var_by_name(nicenv_t * e, char * name);
int del_var(nicenv_t * e, _var_t * v);
int del_auto_vars(nicenv_t * e);

/*Добавляем строку в текст в порядке следования*/
int add_string_to_string(nicenv_t * e, _var_t * str1, _var_t * str2);
int add_string_to_text(nicenv_t * e, _var_t * text, _var_t * str, long pos);
//int append_text_to_text (nicenv_t * e, _var_t * text1, _var_t * text2);
int append_text_to_text (nicenv_t * e, _var_t * text1, _var_t * text2, long pos);
int del_strings_from_text_after(nicenv_t * e, _var_t * text, long pos);
int trim_string_in_text_after(nicenv_t * e, _var_t * text, long line_pos, long col_pos);
int fix_string_quoting(_var_t * str);
int spool_code_to_inpl(nicenv_t * e, _var_t * text);
int check_condition(nicenv_t * e ); 
/*Присваиваем значеие буфера переменной*/
int set_string(_var_t * str, char * inp_buff);
/*Проверяет наличие слов в строке.*/
int last_input_line_contains(nicenv_t * e, char ** special_words);

int parse_line(nicenv_t * e);
int parse_interpret_line_preparse(nicenv_t * e, char * next_word);
int compile (int level, char * word_name);
int load_words(nicenv_t * e, int level);
int unload_words(nicenv_t * e, int level);
int add_words_to_list(nicenv_t * e, int level);
/*Находит на указатель последнего слова.*/
_word_t * find_word(nicenv_t * e);
/*Функции работы со стеком*/
int push_int_to_stack(nicenv_t * e);
int push_dbl_to_stack(nicenv_t * e);
int push_var_to_stack(nicenv_t * e);
int push_var_ptr_to_stack(nicenv_t * e, _var_t * v_n);
int push_code_to_stack(nicenv_t * e);
/*Добавляем строку со случайными именем в стек*/
unsigned long hash(unsigned char *str);
int add_string(nicenv_t * e);
int add_number(nicenv_t * e);
int process_word(nicenv_t * e);
int run_word(nicenv_t * e);
int is_string(nicenv_t * e, char * tok);
int scroll_text(nicenv_t * e, _var_t * t, int nline);
int scroll_text_remove(nicenv_t * e, _var_t * t, int nline);
int copy_var(_var_t * v1, _var_t * v2);
int fork_var(_var_t * v1, _var_t * v2);
int set_num_to_var(_var_t * v1, long a);
int set_dbl_to_var(_var_t * v1, double a);
int set_var_type(_var_t * v1, long t);
long get_num_from_var(_var_t * v1);
double get_dbl_from_var(_var_t * v1);
int set_dot_to_var(_var_t * dot, double x, double y, double z, double t);
/*Return the type by index*/
_newtype_t * get_type_by_index(nicenv_t * e, int t_idx);
_newtype_t * is_type_exist (nicenv_t * e, char * name);

int is_number(nicenv_t * e, char * next_word);
int is_word(nicenv_t * e, char * next_word);

int create_db(nicenv_t * e, _var_t * db);
int is_db_exist(nicenv_t * e, _var_t * db);
int add_var_to_db(nicenv_t * e, _var_t *db, _var_t * v);

int update_pos_symbol_category(nicenv_t * e, int pos, char * inp_buff, char* inp_cat);
int update_symbol_category(nicenv_t * e, int pos, char * inp_buff, char* inp_cat);
int get_next_category_string(nicenv_t * e, int pos, char * inp_buff, char * inp_cat, char * outline, int * cat_id);
int known_names_look_up(nicenv_t * e, char * inp_buff, char* inp_cat);
int is_known_word(nicenv_t * e, char * next_word);
int ins_at_pos( char * inp_buff, int pos, int key);

int set_line_to_var(_var_t * lin, double dl[8]);
int set_matx_to_var(_var_t * lin, double dl[16]);

int alloc_var(_var_t * var, int size);
int add_path_to_path(nicenv_t * e, _var_t * p1, _var_t * p2);



