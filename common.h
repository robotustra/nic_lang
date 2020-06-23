/*
* 	common data structured for nic
*/
#ifndef _COMMON_H_
#define _COMMON_H_

// Раскраска
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RED_HIGH     "\x1b[31;1m"
#define ANSI_COLOR_GREEN_HIGH   "\x1b[32;1m"
#define ANSI_COLOR_YELLOW_HIGH  "\x1b[33;1m"
#define ANSI_COLOR_BLUE_HIGH    "\x1b[34;1m"
#define ANSI_COLOR_MAGENTA_HIGH "\x1b[35;1m"
#define ANSI_COLOR_CYAN_HIGH    "\x1b[36;1m"
#define ANSI_COLOR_WHITE_HIGH   "\x1b[37;1m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define VARIABLE_CAT	0 	/* категория некой последовательности символов, которая может оказаться переменной, словом или типом*/
#define COMMENT_CAT		1
#define STRING_CAT		2
#define LITERAL_CAT 	3
#define WORDS_CAT		4
#define NUMBER_CAT		5
#define TYPE_CAT		6
#define ERROR_CAT		7
#define DQUOTE_CAT		8	/* Двойная кавычка для строки*/
#define S_ESCAPE_CAT	9	/* String escape char*/
#define ESCAPE_CAT 		10  /* : */
#define QUOTE_CAT		11  /* Одинарная кавычка для литерала*/
#define SPACER_CAT		12
#define E_ESCAPE_CAT	13	/* String escape char*/
#define NUMBER_SIGN_CAT	14  /* +- */
#define NUMBER_DOT_CAT		15 
#define NUMBER_POWER_CAT	16  /* E, e*/
#define NUMBER_POWER_SIGN_CAT 17 /*+- in the power*/
#define NUMBER_POWER_NUM_DIG_CAT  18 /*digits power*/
#define KNOWN_VAR_CAT	19 	/* известная переменная используется только для подсветки синтаксиса */




#define MAX_NAME_LENGTH		64 /*32*/
#define MAX_STACK_SIZE		256/*64*/
#define MAX_INPL_SIZE		256
#define MAX_STRING			1024 /*256*/ //Максимально возможная строка для парсинга ввода.
#define VAR_ARRAY_INCREMENT	32	/*Количество элементов массива на которые идет увеличение*/
#define POS_END				-1 	// Вставка строки в конец файла.
#define POS_BEGIN			0   // Вставка в начало строки.
#define MAX_COUNT			128 // Максимальная длина строки в файле.
#define MAX_RAW_STRING		4096 // Максимальная длина строки. 
#define MAX_PATHNAME_LEN	260
#define MAX_LOOPS 			64 	// Максимальная вложенность циклов. Оно может быть любым, но не будем сходить сума.
#define MAX_RUNS 			64  // Максимальное число вложенных RUN в скриптах. RUN подобно циклу, но выполняется 1 раз.
#define MAX_IF_ELSE 		64  // Максимальное число вложенных if-else в скриптах. RUN подобно циклу, но выполняется 1 раз.
#define MAX_NEST			196 // максимальная вложенность ключевых слов в интерпретаторе
#define MAX_BITS_LINT		32  //количество бит в целом числе

// Специальные слова, которые влияют на моду интерпретатора.
#define SPECIAL_WORDS_NUM	13
#define BUILTIN_WORDS_NUM	5

//#define DEBUG_LEVEL			5 	/*prints parsing info*/
#define DEBUG_LEVEL			0

struct nicenv;

// Структура хранения слова
typedef struct _word {
	char name[MAX_NAME_LENGTH];
	struct _word * next;
	struct _word * prev;
	int level;
	int (*code) (struct nicenv *);
	struct _word * depend;  // кольцевой список зависимостей для данного слова.
} _word_t;

//Структура хранения уровней слов
typedef struct _words_ring {
	struct _words_ring * next;
	struct _words_ring * prev;
	int level;
	char ** word_names_index;	// массив указателей слов, для выполнения операции поиска слов
	_word_t * first;			// первое слово в кольце.
	int n_words;				// количество слов в кольце. Просто для подстраховки.
	void *ext_library; 			// библиотека слов, динамически загруженная.
} _words_ring_t;

//Структура для хранения новых типов
typedef struct _newtype{
	char name[MAX_NAME_LENGTH]; 	// Имя типа.
	struct _newtype * next;
	struct _newtype * prev;
	int type_index;
	long type_size;					// размер типа определен, если это сложный тип
	int level;		
}_newtype_t;

typedef struct _newtype_ring{
	int 			level;			// не используется
	_newtype_t * 	first;
	int 			n_types;				// Число новых типов в системе.
	char ** type_names_index;		// массив указателей для поиска типов
}_newtype_ring_t;

//Структура переменных
typedef struct _var
{
	char name[MAX_NAME_LENGTH]; // имя переменной
	struct _var * next;
	struct _var * prev;
	_newtype_t 	type;		// тип переменной, который может быть простым типом, или новым.
	void* first_element; // указатель на первый элемент данных, который определяется типом.
	int num_element; // число элементов, определяет размер в длинах указателей (void*)
	int size; // размер переменных в байтах (необязательная величина).
	char data[8]; //base type data
	char exec; 	// флаг выполнимости переменной. имеет смысл только для текста или для строки.
	char lock;
	char link; // флаг, показывающий является ли данная переменная символическим линком на другую переменную или переменную. 
	//В этом случае переменная содержит имя того объекта на который ссылается, а тип объекта совпадает с тем на который оно ссылается.
} _var_t;

// Вход в кольцо переменных
typedef struct _vars_ring
{	
	int 		level;			// Введено для общности, но не используется пока
	_var_t * 	first;
	int 		n_vars;			// Число переменных в системе.
	long		m_all;			// Количество байт выделенное в памяти.
	char ** 	var_names_index;// Пока не используется, но надо использовать для ускорения поиска	
}_vars_ring_t;

typedef struct nicenv {
	_words_ring_t 	words_root;
	_vars_ring_t	vars_root;
	int 			max_type_index;				// индекс типа, зарегистрированного последним
	_newtype_ring_t types_root;
	int 			shell_mode;					// текущая мода выполнения.
	_var_t *		input;						// все введенные строки
	int 			osp1;
	int 			sp1;						// указатель на вершину стека
	void *	 		S1 [MAX_STACK_SIZE];		// stack 1
	long  			T1 [MAX_STACK_SIZE];		// тип элементов в стеке
	int 			sp2;						// указатель на вершину стека
	void *	 		S2 [MAX_STACK_SIZE];		// stack 2
	long  			T2 [MAX_STACK_SIZE];		// тип элементов в стеке
	char *			next_word;					// строка для парсинга,
	_word_t *		word;						// последнее слово, которое найдено при парсинге.
	_var_t *		var;						// последняя переменная при парсенге.
	double 			dbl;						// последнее введенное число дабл
	long			lint;						// последнее введенное целое число
	long			prev_rand;						// последнее введенное целое число
	char *			string;						// вводимая строка.
	long  			line_pointer;				// указатель на строку для парсинга
	long			line_to_parse;				// указатель 
	long 			col_pointer;				// смещение от начала строки.
	long 			line_commited;				// часть ввода, которая была интерпретирована
	long 			col_commited;				// позиция курсора, которая была интерпретирована
	char 			string_flag;				// указывает, что была откыта строка
	char 			string_flag_ready;			// указывает, что строка закрыта и готова к добавлению в стек.
	char 			escape_flag;				// эскейп последовательность :
	char 			quote_flag;					// указывает, что была откыт символ
	char 			comment_flag;				// указывает на то, что открыт комментарий
	char 			string_flag_pp;				// флаг строки для препарсинга
	char 			help;
	char 			avar;						// флаг о создании автоматических переменных
	char 			recompile_flag;				// если == 1 то либа со словами должна быть перекомпилирована. По умолчанию она не компилится
	char 			preparse_line[MAX_INPL_SIZE]; // распаршеная входная строка.
	int 			parse_param0;				// указывает индекс уровня слова
	int 			parse_param1;				// указывает индекс слова
	int 			var_counter;				// счетчик переменных
	char 			tmpName[MAX_NAME_LENGTH];
	char 			ext_path[MAX_STRING];		// путь к библиотеке слов, если nic вызывается из другой директории
	_var_t *		loop_body[MAX_LOOPS];		// адрес переменной, текста или строки, которая выполняется в цикле
	_var_t *		loop_cond[MAX_LOOPS]; 		// адрес переменной-условия если оно истино, то цикл выполняется, если нет, то выходим из цикла.
	_var_t *		loop_end[MAX_LOOPS]; 		// завершение цикла, выполняется если условие ложное
	char  			loop_cnt; 					// счетчик вложенных циклов.
	char 			test_cond; 					// проверка условия
	_var_t *		run_body[MAX_RUNS];
	int 			run_line[MAX_RUNS];			// счетчик строк, скопированных в inpl
	char 			run_cnt;					// счетчик вложенных run спулеров.
	_var_t *		if_body[MAX_IF_ELSE];
	_var_t * 		else_body[MAX_IF_ELSE];		// счетчик строк, скопированных в inpl
	_var_t * 		if_cond[MAX_IF_ELSE];
	char 			if_cond_test; 				// if условие проверено
	char 			ifelse_cnt;					// счетчик вложенных run спулеров.
	char 			nest[MAX_NEST];				// вложенность слов
	char 			nest_cnt;					// счетчик вложенности
	char 			stop;						// флаг остановки интерпретации в автоматическом режиме.

} nicenv_t;

nicenv_t 	env; // структура данных интерпретатора, содержит в себе все.

// Возможные моды выполнения которые хранятся в env.shell_mode
enum MODES
{
	INTERPRETER_MODE,		
	EDITOR_MODE,
	RUN_MODE,
	WAIT_MODE,
	DEBUG_MODE
};

/*
*	Фкункция будет возвращать только один из токенов приведенных ниже.
*/
enum TOKTYPE
{
	END_OF_PARSE,	// конец ввода	
	NUMBER,			// числовая константа
	STRING,			// "что-то"
	LITERAL,		// 'a'
	//TYPE,			// тип найден в списке
	VAR,			// переменная, найденная в списке
	WORD,			// найденное слово
	CTRL_WORD,    	// может быть имеет смысл разделять контрольные слова и загружаемые?
	NEW_SYMBOL,		// новое слово, которое может стать либо новым типом, переменной или словом
	BAD_SYMBOL,		// не удовлетворяет ни одному из валидных символов.
	ESCAPE,			// эскейп символ, выходим из парсинга.
	EDIT,
	EXIT,
	EMPTY_LINE
};
/*
*	Базовые типы в интерпретаторе.
*/
enum BASE_TYPES
{
	VOID,	//  - разделитель в стеке
	UDEF,	//	- неопределенный тип, одычно неинициализированная переменная
	BYTE, 	// 	- 8 бит
	LETR,	// 	- 8 битный символ
	CHAR, 	// 	- 16 битный символ
	INT, 	// 	- метка типа
	DUBL,	// 	- метка типа
	CODE,   //  - указатель на выполнимый код слова.
	IDX,	//	- модификатор типа на индекс массива
	DIM, 	// 	- модификатор типа на размерность
	ARRA,	// 	- массив
	ARRB, 	//  - бинарный массив, или бинарный файл, загружаемый с диска или создаваемый
	STRI, 	//	- строка
	FORM, 	// 	- модификатор типа на формат строки вывода
	TEXT, 	//	- метка текста
	STRU, 	//	- создание структуры
	TIME, 	// 	- метка типа время
	DOT,	//	- трехмерная точка (пока размерность 3, но может быть 4 мерная)
	VEC,	//	- вектор (модификатор типа)
	PDOT,	// 	- физическая точка, задается 4 координатами.
	RVEC,	//  - относительный вектор, задается единичным вектором и длиной
	LINE,	//  - задается как 2 трехмерные точки (2x4) выделяется 4 координаты
	ARC,	// 	- дуга, задается как направляющий RVEC и аксиальный RVEC который задает поворот. 
			// 		Длина направляюшего вектора есть радиус дуги, Аксиальный вектор показывает в какую сторону нужно совершить
			//		поворот, а его модуль это величина поворота. Поврот делается по правилу правой руки.
	MAT,	//	- матрица 3x3
	MATX,	//	- матрица 4x4
	TYPE,   //  - индекс типа
	VAR_BYTE, //  - указатель переменной типа байт
	VAR_LETR, // 	- 8 битный символ
	VAR_CHAR, // 	- 16 битный символ
	VAR_INT,  //  - переменная типа инт
	VAR_DUBL, // - переменная типа дабл
	//VAR_PTR, // - указатель на переменную, нужно для создания массива переменных. А надо ли?
	DBASE, 	// - тип, база данных, которая является хранилищем объектов на диске, аналог файлов
	PATH,	// 	- сложный объект, который задает контур в 3д пространстве, как последовательность объектов
	AXIS,	// 	- сложный объект, начало координат и единичный вектор
	MAX_TYPE_INDEX
};

#endif /*_COMMON_H_*/