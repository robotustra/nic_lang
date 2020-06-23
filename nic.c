/*This file is obsolete and is not used for current project.*/

/*Nick's interpreter and compiler*/

/*
	This file contains the interpreter of my language, which uses reverse
	polish syntax, like in stack, but can do objects as well.
	It's Forth like language which can compile the code and use compiled words in 
	the interpreter.
*/

/*

	The main object storage is a stack, which contains all the objects.
	Objects could be a simple numbers or a pointers to the defined objects.

	When compiler starts it loads the structures defined in the symbols as an objects. 
	Or it can keep objects only for the context.

*/

/*

	Interpreter has base operation (words) which are in the base of language.

	Interpreters knows the type of objects which are placed in the stack. And do all checks of
	object types to call operators. Because when we define new word, the types of arguments are
	also defined.


	The list of base words.

	[num] [UP] ROT[ATE] : cyclic rotation of arguments in the stack. This rotation can use 
		the number of arguments to rotate in stack.
		[num] by default is taken as 

	[num] DUP[LICATE] : Duplicates the [num]ber of parameters placed on the top of the stack
		by default [num] = 1. If we need duplicate 5 paremeters it will be placed on the top 
		of the stack.


	[num1] [num2] PICK : picks the [num2] arguments skipping the [num1] params from the top.

	[num] DROP : removes the [num]ber of argument from the top of stack.

	value VALUE : creates the noname constant or variable, which could not be saved in the
		interpreter, but could be calculated. This word could be used to define the constant
		of variable. This word can be used in complex expressions for better readability

		Like:

		3 VALUE const1 CONST  ------ 3 const1 CONST
		3.5 VALUE var1 VAR    ------ 3.5 var1 VAR

	[value] name VAR : creates the named storage for variable and set the initial value
		which could be modified during runtime. Value is being put at the storage.
		This operation is applicable to the base types only. All variables no matter
		of what type occupy the same amount of bytes, defined by architecture of CPU.

	[value] name CONST : create the constant which could be initialized only once. And not 
		necessary at declaration time.

	name ADDR[ESS] : puts the adress of defined variables into the stack.

		Every parameter in the stack could be interpreted differently.

	name VALUE : extracting the value of the variable and putting it in the stack.

	address [type] GET: Pick up the value from the address and put it instead on the top 
		of the stack. [type] defines the size of variable being get from memory.

	Operation of increment of address is

	value [type] [addr|name] SET: Saving of the value at some address
		[type] means the type of value being picked from the stack, it can

	
	[begin] [end] SHOW : Show the content of the stack.
	
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INPUT_LEN	128
char string_start_flag = 0;
/*
-------	Helper functions -------
*/
int m_strcmp(char * str1, char * str2)
{
	while ((*str1 != 0) && (*str2 != 0))
	{
		if(*str1 != *str2){
			return -1;
		}
		++str1;
		++str2;
	}
	if(*str1 == *str2)
		return 0;

	return -1;
}

unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}


/* Types for allocation new data for interpreter*/
#define MAX_NAME_LENGTH 	32
typedef struct _VARIABLE
{
	char name[MAX_NAME_LENGTH]; // имя переменной
	int type;		// тип переменной, который может быть простым типом, или новым.
	void* first_element; // указатель на первый элемент, который определяется типом.
	int num_element; // число элементов, определяет размер в длинах указателей
	int size; // размер переменных в байтах.
	char data[8]; //base type data
} VARIABLE_STRUCT;
typedef	struct _TYPE
{
	char name[MAX_NAME_LENGTH];
	int index; // определяет новый индекс, для использования в стеке.
	int size; // размер блока памяти в байтах нужный для выделения нового элемента в стеке.
} TYPES_STRUCT;
typedef struct _TEXT
{
	char name[MAX_NAME_LENGTH];
	long line;
	void * prev;
	void * next;
	void * element;
	int type;			// the type of element which it in the text.
} TEXT_STRUCT;
typedef struct _WORD
{
	char name[MAX_NAME_LENGTH];
	void * prev;
	void * next;
	void * func;
	void * text; // link to text object with the source of word.	
} WORD_STRUCT;

#define MAX_STACK_SIZE 	64

///////////////////////////
// Stack for the variables with the size less or equal to the length of int pointer.
int old_stack_pointer;
int stack_pointer = 0;
void * _STACK [MAX_STACK_SIZE];
long   _STACK_ELEMENT_TYPE[MAX_STACK_SIZE]; //This is a type of element which was parsed.

int stack_pointer2 = 0;
void * _STACK2 [MAX_STACK_SIZE];
long   _STACK_ELEMENT_TYPE2 [MAX_STACK_SIZE]; //This is a type of element which was parsed.


//////////////////////////
// Double is allocated dynamically now

//////////////////////////
// The structure which will use the number of parameters
#define MAX_NEW_VARS	64
#define MAX_NEW_TYPES	16
#define MAX_NEW_WORDS	64
VARIABLE_STRUCT new_vars[MAX_NEW_VARS];
TYPES_STRUCT	new_types[MAX_NEW_TYPES];
int new_vars_counter = 0;
int new_type_counter = 0;


//This is a ring structure to keep all newly defined words.
WORD_STRUCT	words;
//This structure will contain temporary objects and some other structures.
TEXT_STRUCT root;

/*
	Objects could be allocated dynamically if they occupy more space than void*.
*/

enum MODES
{
	INTERPRETER_MODE,
	WORD_BODY_INPUT_MODE,
	COMPILER_MODE
};

enum TOKTYPE
{
	NOTHING,
	BYTE,
	INT_NUMBER,
	LONG,
	DOUBLE_NUMBER,
	NAME,
	RAW_STRING,
	STRING,
	OPERATOR,
	BAD_SYMBOL,
	UNKNOWN_SYMBOL,
	END_OF_PARSE,
	VAR_TYPE,
	VARIABLE_POINTER,
	FUNCTION_POINTER,
	TEXT_TYPE,
	DIM_TYPE,
	IDX_TYPE,
	ARRAY_TYPE,
	OBJECT_TYPE
};


/*
	Keywords list
*/

//Indexes of keywords are used for reference in the cicle below.
char keywords[50][5] = /* fixed number of words*/
{
	"ADDR",  //0 // Address of variable
	"SIZE",  //1
	"SHOW",  //2 // Show the stack content
	"DROP",  //3 // Remove one or more element from the top of stack
	"DUP",   //4 // Duplication of the last element
	"ADD",   //5 // Addition of two latest elements, the result is placed in the stack
	"SUB",   //6 // Subtraction of two integer elements A B SUB -> A-B
	"LIST",  //7 // list of keywords
	"SWAP",  //8 //SWAP 2 top elements 
	"ROT",   //9 //Rotate 3 elements in stack
	"TYPE",  //10 // place the type of element
	"PRI",   //11 //print top element of stack and remove it.
	"PRIN",  //12 //print top element of stack with new line and remove it.
	"BYTE",  //13 // place size of byte in the memory
	"INT",   //14 // integer (long)
	"DUBL",  //15 // double
	"NEGA",  //16 // negation of the number
	"VALU",  //17 // gets value of variable to the stack
	"EQUL",  //18 // assign value to the variable.
	"VARL",  //19 // list the defined variables 
	"VAR",   //20 // Creates the variable from the name and type.
	"SPEL",  //21 // Spell the variable without removing it
	"STRI",  //22 // Creation of string variable.
	"DEL",	 //23 // Delete variable from memory and from variable stack.
	"WORD",  //24 // Creation of new word.
	"IS",    //25 // Word, meaning the end of input of new object, either new word or struct
	"FLIP",  //26 // Move some words from 1st stack to 2nd stack and revert them.
	"FLOP",	 //27 // Return back n elements from 2nd stack to 1st stack in opposit order. 
	"DEEP",  //28 // Stack deep
	"MUL",	 //29 // Multiply 2 numbers.
	"COMP",  //30 // Compiling
	"TRAN",  //31 // translating of nic to c code.
	"DIM",	 //32 // creation of array index
	"IDX",	 //33 // getting index of array
	"MAP",   //34 // Map one area of memory to another
	"UMAP",  //35 // Unmap two objects.
	"ARRA",  //36 // Creation of Array
	"WLIS",  //37 // List of words dynamically allocated
	"/END",  //38 // Finish the input of the text
	"EXIT",  // exit from interpreter
	""
};


/*
	Determin what is the type of tocken used
*/
int toktype(char * pch, int mode)
{
	int i;
	int dot_count = 0;
	// All tockens are separated with spaces.
	if (pch == NULL || (pch != NULL && pch[0] == 0)) return END_OF_PARSE;
	
	// 0) word input mode - should parce string maunally.
	if (mode == WORD_BODY_INPUT_MODE) return RAW_STRING;

	// 1) An int number
	if ( isdigit(pch[0]) || (pch[0] == '-' ) || (pch[0] == '+' ) || (pch[0] == '.') ){
		if (pch[0] == '.') { dot_count ++; }
		for ( i=1; i<strlen(pch); i++ )
		{	
			//Look til the end of the word.
			//--printf("[%c]", pch[i]);
			if ((pch[i] == '.'))
			{
				dot_count ++;
				if (dot_count > 1) return BAD_SYMBOL;
				continue;
			}
			if ( !isdigit(pch[i]) )
			{
				return BAD_SYMBOL;
			}
		}
		if (dot_count) return DOUBLE_NUMBER;
		return INT_NUMBER;
	}
	// 2) Variable name or keyword
	if ( isalpha(pch[0]) || (pch[0] == '_') )
	{
		for ( i=1; i<strlen(pch); i++ )
		{	
			//--printf("{%c}", pch[i]);
			if ( !(isalpha(pch[i]) || isdigit(pch[i]) || (pch[i] == '_')) )
			{
				return BAD_SYMBOL;
			}
		}
		return NAME;
	}
	// 3) String (starts from ")
	if ( pch[0] == '"' )
	{
		string_start_flag ++;
		string_start_flag = string_start_flag%2;
		//start of string, it means that all subsequent symbols will be
		// interpreted as string content until closing parences
		return STRING;
	}

	// 4) Word_input_mode

	// 5) String
	
	return UNKNOWN_SYMBOL;
}
/*
	Helper function to zero variable;		
*/
int zero_var_strict(int idx)
{
	memset( ((char*) & new_vars[idx]), 0, sizeof(VARIABLE_STRUCT) );
	return 0;
}
/*
	Helper function to copy one variable to another
*/
int copy_var_struct(int to_idx, int from_idx)
{	
	int i;
	VARIABLE_STRUCT * var_to;
	VARIABLE_STRUCT * var_from;
	if ( (to_idx < new_vars_counter) && (from_idx < new_vars_counter) )
	{
		//copy all fields of the structure.
		var_to = & new_vars[to_idx];
		var_from = & new_vars[from_idx];

		strcpy(var_to->name, var_from->name);
		var_to->type = var_from->type;
		var_to->first_element = var_from->first_element;
		var_to->num_element = var_from->num_element;
		var_to->size = var_from->size;
		for (i = 0; i < 8; i++)
		{
			var_to->data[i] = var_from->data[i];
		}
	}
	else
	{
		printf("Warning: One or more index are out of range.\n");
	}
	return 0;
}

/* 
SWAP
*/
int swap_two_elements()
{
	void * tmpptr;
	int tmpint;
	double dtmp;
	if (stack_pointer<2)
	{
		printf("Not enough elements to SWAP\n");
		return -1;
	}

	tmpptr = _STACK[stack_pointer-2];
	tmpint = _STACK_ELEMENT_TYPE[stack_pointer-2];

	_STACK[stack_pointer-2] = _STACK[stack_pointer-1];
	_STACK_ELEMENT_TYPE[stack_pointer-2] = _STACK_ELEMENT_TYPE[stack_pointer-1];

	_STACK[stack_pointer-1] = tmpptr;
	_STACK_ELEMENT_TYPE[stack_pointer-1] = tmpint;
	
	return 0;
}

/* 
ROT
*/
int rot_three_elements()
{
	void * tmpptr;
	int tmpint;
	int t1, t2, t3;
	if (stack_pointer<3)
	{
		printf("Not enough elements to ROT\n");
		return -1;
	}
	tmpptr = _STACK[stack_pointer-3];
	tmpint = _STACK_ELEMENT_TYPE[stack_pointer-3];

	_STACK[stack_pointer-3] = _STACK[stack_pointer-2];
	_STACK_ELEMENT_TYPE[stack_pointer-3] = _STACK_ELEMENT_TYPE[stack_pointer-2];

	_STACK[stack_pointer-2] = _STACK[stack_pointer-1];
	_STACK_ELEMENT_TYPE[stack_pointer-2] = _STACK_ELEMENT_TYPE[stack_pointer-1];

	_STACK[stack_pointer-1] = tmpptr;
	_STACK_ELEMENT_TYPE[stack_pointer-1] = tmpint;

	return 0;
}


/*
	This function save long int place the number to the stack
	and return 0 in case of success. Otherwise it return -1
*/
int push_num_to_stack(long num)
{
	if (stack_pointer < MAX_STACK_SIZE-1)
	{
		_STACK[stack_pointer] = (void*) num;
		_STACK_ELEMENT_TYPE[stack_pointer] = INT_NUMBER;
		stack_pointer++;
		return 0;
	}
	return -1; // Stack is full;
}

int put_type(int type)
{
	if (stack_pointer < MAX_STACK_SIZE-1)
	{
		_STACK[stack_pointer] = (void*) ((long) type);
		_STACK_ELEMENT_TYPE[stack_pointer] = VAR_TYPE;
		stack_pointer++;
		return 0;
	}
	return -1; // Stack is full;
}


/*
	This function save double place the number to the stack
	and return 0 in case of success. Otherwise it return -1
*/
int push_dbl_num_to_stack(double dnum)
{
	double * dbl_ptr;
	if (stack_pointer < MAX_STACK_SIZE-1)
	{
		dbl_ptr = (double*) malloc(sizeof(double));
		*dbl_ptr = dnum;

		_STACK[stack_pointer] = (void*) dbl_ptr;
		_STACK_ELEMENT_TYPE[stack_pointer] = DOUBLE_NUMBER;
		stack_pointer++;

		return 0;
	}
	return -1; // Stack is full;
}

/*
	Drop elements
*/
int drop_stack_top()
{
	int i =0;
	int bot;
	int nelem = 1;
	//pick up the number of elements to be dropped
	if (stack_pointer == 0) {
		printf("Error: 1 argument is missing: <INT_NUM> DROP\n");
		return -1;
	}
	stack_pointer--;
	if (_STACK_ELEMENT_TYPE[stack_pointer] != INT_NUMBER){
		printf("Error: argument for DROP should be integer.\n");
		stack_pointer ++;
		return -1;
	}
	nelem = (long) _STACK[stack_pointer];
	if (nelem <= 0 ){
		printf("Warning: argument for DROP is negative, ignoring\n");
		_STACK[stack_pointer] = (void *) 0;
		_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
		return 0;
	}
	bot = stack_pointer-nelem;
	if (bot < 0) 
	{
		printf("Warning: try to remove more elements than there are in the stack\n");
		bot = 0;
	}
	for (i=stack_pointer-1; i>=bot ; i--)
	{
		if (_STACK_ELEMENT_TYPE[i] == DOUBLE_NUMBER){
			//dbl_stack_pointer--;
			if (_STACK[i] != NULL){
				free((double*)_STACK[i]);
			}
			//_DBL_STACK[dbl_stack_pointer] = 0;
		}
		_STACK[i] = (void *) 0;
		_STACK_ELEMENT_TYPE[i] = NOTHING;
	}
	stack_pointer = bot; 
	printf("New SP = %d\n", stack_pointer);
	return 0;
}

/*
	Deep of the stack.
*/
int deep_stack()
{
	if(stack_pointer<MAX_STACK_SIZE)
	{
		
		_STACK[stack_pointer] = (void*) (long) stack_pointer;
		_STACK_ELEMENT_TYPE[stack_pointer] = INT_NUMBER;
		stack_pointer++;
	}else{
		printf("Stack is full.\n");
		return -1;
	}
	return 0;
}

//Helper function to zero element
int drop_element(){
	if (stack_pointer > 0)
	{
		stack_pointer--;
		_STACK[stack_pointer] = (void*) 0;
		_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
		return 0;
	}
	return -1; // can't remove
}
//Helper function to zero element in the stack2
int drop_element_stack2(){
	if (stack_pointer2 > 0)
	{
		stack_pointer2--;
		_STACK2[stack_pointer2] = (void*) 0;
		_STACK_ELEMENT_TYPE2[stack_pointer2] = NOTHING;
		return 0;
	}
	return -1; // can't remove
}

/*
	Helper function to delete top variable
*/
int drop_top_variable()
{
	if (new_vars_counter>0){
		zero_var_strict(new_vars_counter-1);
		new_vars_counter--;
	}
	return 0;
}

/*
	Diplicate one element on the top of stack
*/
int dup_stack_top()
{
	int i =0;
	int bot;
	int nelem = 1;
	double * dbl_ptr;
	//pick up the number of elements to be dropped
	if (stack_pointer == 0) {
		printf("Warning: Stack is empty, nothing to duplicate\n");
		return -1;
	}
	if (stack_pointer < MAX_STACK_SIZE-1 )
	{
		_STACK[stack_pointer] = _STACK[stack_pointer-1];
		_STACK_ELEMENT_TYPE[stack_pointer] = _STACK_ELEMENT_TYPE[stack_pointer-1];
		stack_pointer++;
		if (_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER)
		{
			//make a copy of element
			dbl_ptr = (double*) malloc(sizeof(double));
			*dbl_ptr = * ((double*) _STACK[stack_pointer-2]);
			_STACK[stack_pointer-1] = (void *) dbl_ptr;
		}

	}else
	{
		printf("Stack is full, ignore DUP\n");
		return -1;
	}
	printf("New SP = %d\n", stack_pointer);
	return 0;	
}

// Adds 2 integer numbers in stack
int add_two_ints()
{
	int i =0;
	int bot;
	int nelem = 1;
	//pick up the number of elements to be dropped
	if (stack_pointer < 2) {
		printf("Warning: Need at least 2 arguments in the stack to ADD\n");
		return -1;
	}

	stack_pointer --;
	_STACK[stack_pointer-1] += (long) _STACK[stack_pointer];
	_STACK[stack_pointer] = (void *) 0; // clear the location over SP
	_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
	printf("New SP = %d\n", stack_pointer);
	return 0;
}

int mul_minus_one()
{

	if(stack_pointer < 1)
	{
		printf("Warning: Need at least 1 argument in the stack to NEG\n");
		return -1;
	}
	if(_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER)
	{
		_STACK[stack_pointer-1] =  (void *) (-1 *((long) _STACK[stack_pointer-1]) );
	}
	else
	if (_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER)
	{
		* ((double*) _STACK[stack_pointer-1]) *= -1.0;
	}
	else
	{
		printf("Multiplication is not defined for this type\n");
		return -1;
	}
	return 0;
}

int add_two_numbers()
{
	int i =0;
	int bot;
	int nelem = 1;
	//pick up the number of elements to be dropped
	if (stack_pointer < 2) {
		printf("Warning: Need at least 2 arguments in the stack to ADD\n");
		return -1;
	}

	stack_pointer --;
	// D + D
	if (_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER &&
		_STACK_ELEMENT_TYPE[stack_pointer] == DOUBLE_NUMBER)
	{
		printf("D+D\n");
		* ((double *)_STACK[stack_pointer-1]) += *((double *)_STACK[stack_pointer]); 

		if(_STACK[stack_pointer] != NULL) free(_STACK[stack_pointer]); 
		_STACK[stack_pointer] = (void *) 0; // clear the location over SP
		_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
	}
	else
	// D + I
	if (_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER &&
		_STACK_ELEMENT_TYPE[stack_pointer] == INT_NUMBER)
	{
		printf("D+I\n");
		* ((double *)_STACK[stack_pointer-1]) += (long)(_STACK[stack_pointer]) ;
						
		_STACK[stack_pointer] = (void *) 0; // clear the location over SP
		_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
	} 
	else
	// I + D
	if (_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER &&
		_STACK_ELEMENT_TYPE[stack_pointer] == DOUBLE_NUMBER)
	{
		printf("I+D\n");
		stack_pointer++;
		swap_two_elements();
		stack_pointer--;
		//printf("afrer swap %d\n", stack_pointer);
		* ((double *)_STACK[stack_pointer-1]) += (long)(_STACK[stack_pointer]) ;
		//printf("++++ %d\n", stack_pointer);				
		_STACK[stack_pointer] = (void *) 0; // clear the location over SP
		_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
		//printf("afrer zero %d\n", stack_pointer);
	}
	else
	// I + I
	if (_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER &&
		_STACK_ELEMENT_TYPE[stack_pointer] == INT_NUMBER)
	{
		//printf("I+I\n");
		stack_pointer++;
		add_two_ints();
	}
	else
	{
		printf("Error: Addition could not be applied for these types of arguments\n");
		stack_pointer++;
		return -1;
	}
	//printf("New SP = %d\n", stack_pointer);
	return 0;
}

/*
	Multiply two numbers
	1) Check if we have 2 numbers
*/
int mul_two_numbers()
{
	double prod;
	long iprod;
	if( (stack_pointer>1) &&
		(_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER || 
		(_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER) &&
		(_STACK_ELEMENT_TYPE[stack_pointer-2] == INT_NUMBER || 
		(_STACK_ELEMENT_TYPE[stack_pointer-2] == DOUBLE_NUMBER) ) ) )
	{
		//multiply D * D
		if ( (_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER) &&
			(_STACK_ELEMENT_TYPE[stack_pointer-2] == DOUBLE_NUMBER) ) 
		{
			//result is double
			//printf("D*D\n");
			prod = *(double*)(_STACK[stack_pointer-1]);
			//printf(":::%lf\n", prod);
			prod *= *(double*)(_STACK[stack_pointer-2]);
			//printf(":::%lf\n", prod);
			*(double*)(_STACK[stack_pointer-2]) = prod;
			free((double*)_STACK[stack_pointer-1]);
		}else
		if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == DOUBLE_NUMBER) &&
			(_STACK_ELEMENT_TYPE[stack_pointer-2] == INT_NUMBER))
		{	
			// I * D
			//printf("I*D\n");
			prod = *(double*)(_STACK[stack_pointer-1]);
			//printf(":::%lf\n", prod);
			prod *= (long)(_STACK[stack_pointer-2]);
			//printf(":::%lf\n", prod);
			*(double*)(_STACK[stack_pointer-1]) = prod;
			//printf("SP1 = %d", stack_pointer);
			swap_two_elements();
			//printf("After swap_two_elements\n");
		}else
		if ((_STACK_ELEMENT_TYPE[stack_pointer-2] == DOUBLE_NUMBER) &&
			(_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER))
		{
			// D * I
			//printf("D*I\n");
			swap_two_elements();
			prod = *(double*)(_STACK[stack_pointer-1]);
			//printf(":::%lf\n", prod);
			prod *= (long)(_STACK[stack_pointer-2]);
			//printf(":::%lf\n", prod);
			*(double*)(_STACK[stack_pointer-1]) = prod;
			swap_two_elements();
		}
		else
		{
			//INT * INT
			//printf("I*I\n");
			iprod = (long)(_STACK[stack_pointer-1]);
			//printf(":::%d\n", iprod);
			iprod *= (long)(_STACK[stack_pointer-2]);
			//printf(":::%d\n", iprod);
			_STACK[stack_pointer-1] = (void*) iprod;
			swap_two_elements();
		}
		//printf("SP2 = %d", stack_pointer);
		drop_element();
		//printf("SP3 = %d", stack_pointer);
	}
	else
	{
		printf("Warning: MUL is not defined for this types.\n");
	}
	return 0;
}

int sub_two_ints()
{
	int i =0;
	int bot;
	int nelem = 1;
	//pick up the number of elements to be dropped
	if (stack_pointer < 2) {
		printf("Warning: Need at least 2 arguments in the stack to SUB\n");
		return -1;
	}

	stack_pointer --;
	_STACK[stack_pointer-1] -= (long) _STACK[stack_pointer];
	_STACK[stack_pointer] = (void *) 0; // clear the location over SP
	_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
	//printf("New SP = %d\n", stack_pointer);
	return 0;
}

int sub_two_numbers()
{
	if (0 == mul_minus_one())
	{
		if (0 == add_two_numbers())
		{
			return 0;
		}
		return -1;
	}
	return -1;
}

int print_type(int tp)
{
	if (tp == NOTHING)				printf("NOTHING");
	else if (tp == BYTE) 			printf("BYTE");
	else if (tp == INT_NUMBER)   	printf("INT");
	else if (tp == DOUBLE_NUMBER)	printf("DOUBLE");
	else if (tp == STRING)			printf("STRING");
	else if (tp == TEXT_TYPE)		printf("TEXT");
	// here I have to look for the type in the list of newly defined types after.
	else printf("UNKNOWN TYPE");
	return 0;
}

int print_stack()
{
	int i =0;
	printf("---stack top---\n");
	for (i=stack_pointer-1; i>=0; i--)
	{
		switch(_STACK_ELEMENT_TYPE[i])
		{
			case NOTHING:
			{
				//Nothing left in the stack if the variable were removed from the stack.
				//this situation is ok, nothing coild be droped from the stack.
				printf("%d : ", (long)_STACK[i]);
				printf("NOTHING\n");
			}break;
			case INT_NUMBER:
			{
				printf("%d : ", (long)_STACK[i]);
				printf("INT\n");
			}break;
			case DOUBLE_NUMBER:
			{
				printf("%lf : ", *((double*)_STACK[i]) );
				printf("DOUBLE\n");
			}break;
			case VAR_TYPE:
			{
				printf("%d : ", (long)_STACK[i] );
				printf("TYPE INDEX\n");
			}break;
			case VARIABLE_POINTER:
			{
				printf("%s@%p : ", (void*)((VARIABLE_STRUCT *) (_STACK[i]))->name , _STACK[i] );
				printf("VARIABLE POINTER TO ");
				print_type( ((VARIABLE_STRUCT *) (_STACK[i]))->type );
				printf("\n");
			}break;
			case STRING:
			{
				printf("%s@%p : ", (void *)((VARIABLE_STRUCT *) (_STACK[i]))->name , _STACK[i] );
				printf("STRING\n");
			}break;
			default:
			{
				printf("UNDEFINED TYPE\n");
			}
		}
	}
	printf("---stack bottom---\n");
	return 0;
}

int print_stack2()
{
	int i =0;
	printf("---stack2 top---\n");
	for (i=stack_pointer2-1; i>=0; i--)
	{
		switch(_STACK_ELEMENT_TYPE2[i])
		{
			case NOTHING:
			{
				//Nothing left in the stack if the variable were removed from the stack.
				//this situation is ok, nothing coild be droped from the stack.
				printf("%d : ", (long)_STACK2[i]);
				printf("NOTHING\n");
			}break;
			case INT_NUMBER:
			{
				printf("%d : ", (long)_STACK2[i]);
				printf("INT\n");
			}break;
			case DOUBLE_NUMBER:
			{
				printf("%lf : ", *((double*)_STACK2[i]) );
				printf("DOUBLE\n");
			}break;
			case VAR_TYPE:
			{
				printf("%d : ", (long)_STACK2[i] );
				printf("TYPE INDEX\n");
			}break;
			case VARIABLE_POINTER:
			{
				printf("%s@%p : ", (void*)((VARIABLE_STRUCT *) (_STACK2[i]))->name , _STACK2[i] );
				printf("VARIABLE POINTER TO ");
				print_type( ((VARIABLE_STRUCT *) (_STACK2[i]))->type );
				printf("\n");
			}break;
			case STRING:
			{
				printf("%s@%p : ", (void *)((VARIABLE_STRUCT *) (_STACK2[i]))->name , _STACK2[i] );
				printf("STRING\n");
			}break;
			default:
			{
				printf("UNDEFINED TYPE\n");
			}
		}
	}
	printf("---stack2 bottom---\n");
	return 0;
}


int is_keyword(char * chr)
{
	int i;
	char ch_upper[5];
	for(i=0; i<strlen(chr); i++)
	{
		ch_upper[i] = toupper(chr[i]);
	}
	i=0;
	while (strlen(keywords[i]))
	{
		if (!strcmp(keywords[i], ch_upper))
		{
			//--printf("Found keyword: %s\n", keywords[i]);
			//printf("Index: %d\n", i );
			return i;
		}
		//printf("%s\n", keywords[i]);
		i++;
	}
	return -1; // not a keyword
}

int list_keywords()
{
	int i=0;
	while (strlen(keywords[i]))
	{
		printf("%s\t", keywords[i]);
		i++;
		if(!(i%12) ) printf("\n");
	}
	printf("\n");
	return 0;
}

/* TYPE
	Place the type of variable to the stack
*/
put_type_to_stack()
{
	//pick up the number of elements to be dropped
	if (stack_pointer == 0){
		_STACK[stack_pointer] = (void*) NOTHING; // the type
		_STACK_ELEMENT_TYPE[stack_pointer] = VAR_TYPE;
		stack_pointer++;
		return 0;
	}
	if (stack_pointer < MAX_STACK_SIZE-1)
	{
		_STACK[stack_pointer] = (void*) _STACK_ELEMENT_TYPE[stack_pointer-1]; // the type
		_STACK_ELEMENT_TYPE[stack_pointer] = VAR_TYPE;
		stack_pointer++;
		return 0;	
	}else
	{
		printf("Error: Can't place TYPE of variable because the stack is full\n");
	}
	return -1;
}

/*
* 	Look for the name in the list of variables	
*/
int is_new_name(char * pch)
{
	int i;
	for (i = 0; i < MAX_NEW_VARS; i++)
	{
		if(!m_strcmp(new_vars[i].name,pch) )
		{
			return 0; // not new name
		}
	}
	return 1; //not found in the list
}

/*
	Add byte to stack
	char name[32]; // имя переменной
	int type;		// тип переменной, который может быть простым типом, или новым.
	void* first_element; // указатель на первый элемент, который определяется типом.
	int num_element; // число элементов, определяет размер в длинах указателей
	int size; // размер переменных в байтах.
*/
int add_new_name(char * pch)
{
	int vtype;
	//get type of variable from stack

	// I have to decide how to treat type of variables
	//--------------------------------------------------
	/*
	if (stack_pointer > 0 && _STACK_ELEMENT_TYPE[stack_pointer-1] == VAR_TYPE)
	{
		vtype = (int) _STACK[stack_pointer-1];
		drop_element(); 
	}
	else
	{
		printf("Can't add variable of this type.\n");
		return -1;
	}*/
	//1) Allocate variable, 2) copy the address of variable to the stack.
	if (new_vars_counter < MAX_NEW_VARS-1)
	{
		// have space to add
		if (strlen(pch) < MAX_NAME_LENGTH ) 
			strcpy(new_vars[new_vars_counter].name, pch);
		else
			strncpy(new_vars[new_vars_counter].name, pch, MAX_NAME_LENGTH);

		new_vars[new_vars_counter].type = NOTHING; // the type of the name is undefined
		new_vars[new_vars_counter].first_element = (void*) new_vars[new_vars_counter].data; //NULL;
		new_vars[new_vars_counter].num_element = 1;
		new_vars[new_vars_counter].size = 1;
		new_vars_counter ++;

	} else
	{
		printf("Not enough of space to add variable.\n");
		return -1;
	}
	return (new_vars_counter-1); // the index of variable
}

/*
*	Put variable pointer to stack. Newly defined names without types are also placed in the list of variables.
*/
int put_var_to_stack(int idx)
{
	if(stack_pointer < MAX_STACK_SIZE-1){
		_STACK[stack_pointer] = (void *) &(new_vars[idx]);
		_STACK_ELEMENT_TYPE[stack_pointer] = VARIABLE_POINTER;
		stack_pointer++;
	}
	else
	{
		printf("Not enough stack to put variable\n");
		return -1;
	}
	return 0;
}

/* PRI
*	Print the top element and remove it from stack
*/
int print_top_stack_element()
{
	int el_type;
	if (stack_pointer == 0)
	{
		printf("Warning: Nothing to print\n");
		return -1;
	}
	else
	{
		//printf("SP: %d\n", stack_pointer);
		stack_pointer--;
		el_type = _STACK_ELEMENT_TYPE[stack_pointer];
		//printf("EL T: %d", el_type);
		switch(el_type)
		{
			case INT_NUMBER:
			{
				printf("%d\n", (long)_STACK[stack_pointer]);
			}break;
			case DOUBLE_NUMBER:
			{
				printf("%lf\n", *((double*)_STACK[stack_pointer]) );
				if (_STACK[stack_pointer] != NULL) free(_STACK[stack_pointer]);
			}break;
			case VAR_TYPE:
			{
				printf("%d\n", (long)_STACK[stack_pointer] );
			}break;
			case STRING:
			{
				printf("%s\n", (char*)((VARIABLE_STRUCT *) (_STACK[stack_pointer]))->first_element);
			}break;
			default:
			{
				printf("UNDEFINED PRINT FORMAT\n");
			}
		}
		_STACK[stack_pointer] = (void*) 0;
		_STACK_ELEMENT_TYPE[stack_pointer] = NOTHING;
	}
	//printf("New SP:%d\n", stack_pointer);
	return 0;
}

print_n_stack_element()
{
	int i;
	int n_el;
	int el_type;
	if (stack_pointer < 1)
	{
		printf("Warning: Need one integer parameter for PRIN\n");
		return -1;
	}
	n_el = (long) _STACK[stack_pointer-1];
	if (_STACK_ELEMENT_TYPE[stack_pointer-1] != INT_NUMBER || n_el <0){
		printf("Warning: Parameter for PRIN shold be positive integer\n");
		return -1;	
	}
		
	drop_element();

	while ( (n_el > 0) && ((stack_pointer-1)>0) )
	{
		//printf("SP: %d\n", stack_pointer);
		el_type = _STACK_ELEMENT_TYPE[stack_pointer-1];
		switch(el_type)
		{
			case INT_NUMBER:
			{
				printf("%d ", (long)_STACK[stack_pointer-1]);
			}break;
			case DOUBLE_NUMBER:
			{
				printf("%lf ", *((double*)_STACK[stack_pointer-1]) );
				if (_STACK[stack_pointer-1] != NULL) free ((double*)_STACK[stack_pointer-1]);
			}break;
			case VAR_TYPE:
			{
				printf("%d ", (long)_STACK[stack_pointer-1] );
			}break;
			case STRING:
			{
				printf("%s\n", (char*)((VARIABLE_STRUCT *) (_STACK[stack_pointer-1]))->first_element);
			}break;
			default:
			{
				printf("UNDEFINED PRINT FORMAT\n");
			}
		}
		drop_element();
		//stack_pointer--;
		n_el--;
	}
	printf("\n");
	return 0;
}

/*
* Check if this name already in the list of variables.
*/
int is_variable(char* pch)
{
	int i;
	int r = 0;
	// try to look variable name if it's defined.
	for (i=0; i<new_vars_counter; i++){
		if ( 0 == strcmp(new_vars[i].name, pch) ){
			// name is found, return the index of variable in the stack.
			return i;
		}
	}
	// if we are here the variable was not found just return error
	return -1; //not a variable
}

/*
* 	1) Check if the type of name == NOTHING
*	2) Check it the previous argument is type index
*	3) Set the type of variable and remove it from the stack.
*	4) Remove also the variable from main stack.
*/
int create_variable_from_name()
{
	VARIABLE_STRUCT * var_ptr = NULL;

	if (stack_pointer < 2) {
		printf("Warning: Two arguments are needed for VAR\n");
		return -1;
	}
	if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == VARIABLE_POINTER) &&
		(_STACK[stack_pointer-1] != NULL))	
	{
		var_ptr = _STACK[stack_pointer-1];
		if ((var_ptr->type == NOTHING) &&
			(_STACK_ELEMENT_TYPE[stack_pointer-2] == VAR_TYPE))
		{
			//ok, have to check the size of variablle's type and allocate space if needed.
			//if the types are BYTE, INT, LONG we will not allocate memory
			// May be it's ok not to alocate built in types. Use allocation for new types?
			if( (_STACK[stack_pointer-2] == ((void*) INT_NUMBER)) ||
				(_STACK[stack_pointer-2] == ((void*) BYTE)) ||
				(_STACK[stack_pointer-2] == ((void*)DOUBLE_NUMBER)) )
			{
				var_ptr->type = (long) _STACK[stack_pointer-2];
				var_ptr->first_element = (void*) var_ptr->data;
			}
			else
			{
				printf("Non generic type, looking for type to allocate.\n");
				// If the type is defined, allocate the number of elements.
			}
			swap_two_elements();
			drop_element();
		}else
		{
			printf("Cannot create variable because the type is not defined.\n");
			//drop_element(); //Do not remove variable from stack
		}
	}
	return 0;
}


/*
	Display all variables in the variable stack
*/
int var_list()
{
	int i;
	printf("---- Variables Stack Top ----\n");
	for (i=new_vars_counter-1; i>=0; i--)
	{
		printf("%s@%x : ", new_vars[i].name, new_vars[i].first_element);
		print_type(new_vars[i].type);
		printf("\n");
	}
	printf("---- Variables Stack Bot ----\n");
	return 0;
}

/*
	EQUL
	1) takes the value and save it in the variable
	2) remove it form stack on success
*/
int assign_value_to_var()
{
	char * chpar;
	int * ipar;
	double * dpar;
	int type;
	VARIABLE_STRUCT * var_ptr = NULL;

	if (stack_pointer < 2)
	{
		printf("Warning: Not enough parameters for EQUL\n");
		return -1;
	}
	var_ptr = (VARIABLE_STRUCT *) _STACK[stack_pointer-1]; 
	if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == VARIABLE_POINTER)/* &&
		(var_ptr->type == _STACK_ELEMENT_TYPE[stack_pointer-2]) */) 
	{
		//!!!! We can overwrite the current type of variable!!!
		//!!!! All responsibility is on programmer!!!

		//Can make an assignment of value
		var_ptr->type = _STACK_ELEMENT_TYPE[stack_pointer-2]; // CAREFUL!!!
		type = var_ptr->type;
		if (type == BYTE) {
			chpar = (char*) ((VARIABLE_STRUCT *)_STACK[stack_pointer-1])->first_element;
			*chpar = ((long) _STACK[stack_pointer-2]) & 0xFF;
			swap_two_elements();
			drop_element();
		}else
		if (type == INT_NUMBER) {
			ipar = (int*) ((VARIABLE_STRUCT*)_STACK[stack_pointer-1])->first_element;
			*ipar = (long) _STACK[stack_pointer-2];
			swap_two_elements();
			drop_element();
		}else
		if (type == DOUBLE_NUMBER) {
			dpar = (double*) ((VARIABLE_STRUCT*)_STACK[stack_pointer-1])->first_element;
			*dpar = * ((double*) _STACK[stack_pointer-2]);
			swap_two_elements();
			drop_element();
		}else
		{
			printf("EQUL is not implemented for this type yet.\n");
		}
		
	}

	return 0;
}

/*	VALU
	Gets value of variable and put it to stack
*/
int get_variable_value()
{
	VARIABLE_STRUCT * var_ptr;
	double * dbl_ptr;

	if (stack_pointer<MAX_STACK_SIZE)
	{
		if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == VARIABLE_POINTER) &&
			(_STACK[stack_pointer-1] != NULL))
		{
			var_ptr = (VARIABLE_STRUCT*) _STACK[stack_pointer-1];
			if ( var_ptr->type == BYTE ){
				_STACK[stack_pointer] = (void*) (long) *((char*)var_ptr->first_element);
				_STACK_ELEMENT_TYPE[stack_pointer] = BYTE;
				stack_pointer++;
				swap_two_elements();
				drop_element();
			}else
			if ( var_ptr->type == INT_NUMBER ){
				_STACK[stack_pointer] = (void*) *((long*)var_ptr->first_element);
				_STACK_ELEMENT_TYPE[stack_pointer] = INT_NUMBER;
				stack_pointer++;
				swap_two_elements();
				drop_element();
			}else
			if (var_ptr->type == DOUBLE_NUMBER) {
				//if (dbl_stack_pointer < MAX_STACK_SIZE)	{
				dbl_ptr = (double*) malloc(sizeof(double));
				*dbl_ptr = *((double*)var_ptr->first_element);
				//_DBL_STACK[dbl_stack_pointer] = *((double*)var_ptr->first_element);
				_STACK[stack_pointer] = (void*) dbl_ptr;
				_STACK_ELEMENT_TYPE[stack_pointer] = DOUBLE_NUMBER;
				//dbl_stack_pointer++;
				stack_pointer++;
				swap_two_elements();
				drop_element();
				//}
			}else{
				printf("VALU is not implemented for this type\n");
			}
		}
	}
	else
	{
		printf("Stack is full.\n");
		return -1;
	}
	return 0;
}

/*
*	Add string to the stack
*	1) Allocate variable for string
*/
int add_string(char * tmpstr)
{
	unsigned long strhash;
	VARIABLE_STRUCT * var_ptr;

	if (stack_pointer < MAX_STACK_SIZE)
	{
		var_ptr = (VARIABLE_STRUCT *) malloc(sizeof(VARIABLE_STRUCT));

		printf("Allocated Var struct\n");	
		var_ptr->size = strlen(tmpstr);
		
		strhash = hash((unsigned char*) tmpstr);
		//if(strhash < 0) strhash*=-1;
		printf("Hash: %lu\n", strhash);
		sprintf(var_ptr->name, "string%lu", strhash );
		var_ptr->first_element = (void *) malloc(strlen(tmpstr));
		strcpy(var_ptr->first_element, tmpstr);
		var_ptr->type = STRING;
		// put in the stack
		_STACK_ELEMENT_TYPE[stack_pointer] = STRING;
		_STACK[stack_pointer] = (void *) var_ptr;
		stack_pointer++;
	}
	else
	{
		printf("Not enough space to allocate string.\n");
	}

	return 0;
}

/*
	Spell object and do not remove it form the stack.
*/
int spell_val()
{
	int el_type;
	if (stack_pointer == 0)
	{
		printf("Warning: Nothing to print\n");
		return -1;
	}
	else
	{
		//printf("SP: %d\n", stack_pointer);
		stack_pointer--;
		el_type = _STACK_ELEMENT_TYPE[stack_pointer];
		//printf("EL T: %d", el_type);
		switch(el_type)
		{
			case INT_NUMBER:
			{
				printf("%d\n", (long)_STACK[stack_pointer]);
			}break;
			case DOUBLE_NUMBER:
			{
				printf("%lf\n", *((double*)_STACK[stack_pointer]) );
			}break;
			case VAR_TYPE:
			{
				// this should be generalized after. to display the value content.
				printf("%d\n", (long)_STACK[stack_pointer] );
			}break;
			case STRING:
			{
				printf("%s\n", (char*) ((VARIABLE_STRUCT*)_STACK[stack_pointer])->first_element );
			}break;
			default:
			{
				printf("UNDEFINED PRINT FORMAT\n");
			}
		}
		stack_pointer++;
	}
	return 0;
}

/*
	Init root structure
*/
int init_root()
{
	strcpy( root.name, "root");
	root.line = 0;
	root.prev = NULL;
	root.next = NULL;
	root.element = NULL;
	root.type = NOTHING;
	return 0; 
}

/**
*	If there is no name defined for the string it's added to the root structure.
*/
int add_obj_to_root(void * obj)
{
	void * next_el = root.next;
	void * new_el;
	while(next_el != NULL)
	{
		next_el = ((TEXT_STRUCT*) next_el) -> next;
		printf("Element: %x\n", next_el);
	}
	// allocate new element
	new_el = (void*) malloc(sizeof(TEXT_STRUCT));
	printf("New element is allocated\n");

	((TEXT_STRUCT*)next_el)->next = new_el;
	((TEXT_STRUCT*)new_el)->prev =  next_el;
	((TEXT_STRUCT*)new_el)->next = NULL;
	((TEXT_STRUCT*)new_el)->element = obj;	// do not check 
	((TEXT_STRUCT*)new_el)->type = OBJECT_TYPE;
	return 0;
}
/**
*	Remove object from root list.
*/
int del_object_from_root(void * obj)
{
	//................
	printf("del_object_from_root()\n");
	return 0;
}


/*	"string" <NAME> STRI
*	Creates the string from variable allocated in the stack and move it to the named variable.
*
*/
int create_string_variable()
{
	VARIABLE_STRUCT * var_ptr = NULL;
	VARIABLE_STRUCT * var_string = NULL;
	if (stack_pointer < 2) {
		printf("Warning: Two arguments are needed for STRI\n");
		return -1;
	}
	if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == VARIABLE_POINTER) &&
		(_STACK[stack_pointer-1] != NULL))	
	{
		var_ptr = _STACK[stack_pointer-1];
		if ((var_ptr->type == NOTHING) &&
			(_STACK_ELEMENT_TYPE[stack_pointer-2] == STRING))
		{
			//copy string parameter
			var_string = (VARIABLE_STRUCT *)_STACK[stack_pointer-2];
			
			var_ptr->type = STRING;
			var_ptr->first_element = var_string->first_element;
			var_ptr->size = var_string->size;
			//after copy tmp string deallocate variable structure.

			//_STACK_ELEMENT_TYPE[stack_pointer-1] = STRING; // It's by default the VARIABLE POINTER
			if (var_string != NULL) free(var_string);

			swap_two_elements();
			drop_element();
		}else
		{
			printf("Cannot create variable because the type is not defined.\n");
			//drop_element(); //Do not remove variable from stack
		}
	}
	return 0;
}
/**	DEL
*	Delete variable from all stacks. 
*/
int delete_var()
{
	int i, j;
	VARIABLE_STRUCT * var_ptr = NULL;
	if (stack_pointer<1){
		printf("Warning: One argument is needed for DEL\n");
		return -1;
	}
	if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == VARIABLE_POINTER) &&
		(_STACK[stack_pointer-1] != NULL))	
	{
		// 1) find the place in the variable stack.
		for (i = new_vars_counter-1; i>=0; i--){
			if (_STACK[stack_pointer-1] == ((void *) &(new_vars[i])) )
			{
				printf("Variable index is %d\n", i);
				break;
			}
		}
		//remove all dynamicly allocated data from variable.
		var_ptr = (VARIABLE_STRUCT*) _STACK[stack_pointer-1];
		
		if (var_ptr->type == STRING){ // no other types should be freed
			free(var_ptr->first_element);
		}
		// move top element to fill the gap
		if (i < new_vars_counter-1){
			copy_var_struct(i, new_vars_counter-1);
		}
		//replace all references to this variable from STACK if any.
		for(j=stack_pointer-1; j>=0; j--)
		{
			if (_STACK[j] == ((void*)var_ptr) ){
				_STACK_ELEMENT_TYPE[j] = NOTHING;
				_STACK[j] = 0;
			}
			// replace the pointer to the structure
			if (_STACK[j] == ((void*) & new_vars[new_vars_counter-1]) &&
				(i < new_vars_counter-1) ){
				_STACK[j] = (void*) & new_vars[i];
			}
		}
		drop_top_variable();
		drop_element();
	}
	return 0;
}

/** WORD
*	This function sets the attribute TEXT to variable placed in the 
*	top of stack.
*   
*/
int set_word_attribute_to_var()
{
	TEXT_STRUCT * txt_ptr = NULL;
	VARIABLE_STRUCT * var_ptr = NULL;

	if (stack_pointer < 1) {
		printf("Warning: One name is needed for WORD\n");
		return -1;
	}
	if ((_STACK_ELEMENT_TYPE[stack_pointer-1] == VARIABLE_POINTER) &&
		(_STACK[stack_pointer-1] != NULL))	
	{
		var_ptr = _STACK[stack_pointer-1];
		if (var_ptr->type == NOTHING)
		{
			var_ptr->type = TEXT_TYPE;
			var_ptr->first_element = malloc(sizeof(TEXT_STRUCT));
			var_ptr->size = sizeof(TEXT_STRUCT);
			// init text structure
			txt_ptr = (TEXT_STRUCT*) var_ptr->first_element;
			strcpy( txt_ptr->name, var_ptr->name);
			txt_ptr->line = 0;	//the index of the line
			txt_ptr->prev = NULL;
			txt_ptr->next = NULL;
			txt_ptr->element = NULL; //malloc(sizeof(VARIABLE_STRUCT));
			txt_ptr->type = TEXT_TYPE;
		}else
		{
			// This is a protection against erasing of existing variable
			// But it can be allowed to do in the future.
			printf("Cannot create variable because the variable is not empty.\n");
			//drop_element(); //Do not remove variable from stack
		}
	}
	return 0;
}



/*
	Init words structure
*/
int init_words()
{
	strcpy( words.name, "WORDS_ROOT");
	words.prev = NULL;
	words.next = NULL;
	words.func = NULL;
	words.text = NULL;
	return 0; 
}

/*
*	Helper function which finds the end of the list.
*/
void * get_last_word_addr()
{
	if(words.prev == NULL){ 
		return (void*) &words;
	}
	return words.prev; // this is an last added word in the list.
}

/** WORD
*	This is move newly created name from new_vars array to the new word
*	structure. 
*	1) Copy the name
*	2) Remove variable with such name
*	3) Allocate structure for text.
*	//4) Allocate space for 1st string.
*/
int new_word_creation()
{
	TEXT_STRUCT * txt_ptr;
	VARIABLE_STRUCT * str_ptr;
	//find the end of the words list
	WORD_STRUCT * last_def_word = (WORD_STRUCT *) get_last_word_addr();
	WORD_STRUCT * new_def_word = NULL;
	//1)
	// 1.1) Allocating new word.
	new_def_word = (WORD_STRUCT *) malloc (sizeof(WORD_STRUCT));
	// 1.2) Make link to the existing ring list.
	last_def_word->next = (void*) new_def_word;
	new_def_word->prev = (void*) last_def_word;
	new_def_word->next = NULL;
	words.prev = (void*) new_def_word;

	strcpy(new_def_word->name, new_vars[new_vars_counter-1].name);
	//2)
	delete_var();
	//3)
	new_def_word->text = malloc(sizeof(TEXT_STRUCT));
	txt_ptr = (TEXT_STRUCT*) new_def_word->text;
	strcpy( txt_ptr->name, "word");
	txt_ptr->line = 0;	//the index of the line
	txt_ptr->prev = NULL;
	txt_ptr->next = NULL;
	txt_ptr->element = NULL; //malloc(sizeof(VARIABLE_STRUCT));
	txt_ptr->type = TEXT_TYPE;
	// Do not allocate space for string because the body of word could be empty
	return 0;
}

/*
* 	This function adds the string to the body of new word.
*	1) String is extracted after word "word" and till the word "is"
*   
*/
int add_string_to_word()
{
	TEXT_STRUCT * txt_ptr;
	VARIABLE_STRUCT * str_ptr;
	//find the end of the words list
	WORD_STRUCT * last_def_word = (WORD_STRUCT *) get_last_word_addr();
	// Make sure we got the right word address
	printf("Last word: %s\n",last_def_word->name);

	/*//4)
	str_ptr = (VARIABLE_STRUCT*) txt_ptr->element;
	// init variable structure
	str_ptr->type = STRING;
	str_ptr->size = 0;
	str_ptr->first_element = NULL;
	*/
	return 0;
}

/*	IS
*	This keyword defines the whole text of the new word.
* 	According to the Forth practice there could be different
*/
int new_word_defined()
{
	return 0;
}

/*
* 	WLIS == Word list
*	List all words, created from interpreter.
*/
int new_words_list()
{
	WORD_STRUCT * pword = &words;
	while (pword != NULL){
		printf("Name word '%s'\n", pword->name );
		pword = pword->next;
	}

	return 0;
}

/* FLIP
	Take n elements and flip them from one stack to another.
*/
int flip_n_elements()
{
	int i;
	int top;
	if ( (_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER) && 
		((long)_STACK[stack_pointer-1]) <= (stack_pointer - 1) )
	{
		//can move elements to _STACK2
		top = (long)_STACK[stack_pointer-1];
		drop_element(); // remove counter from stack
		for(i=0; i<top; i++)
		{
			//move element from one top to another
			_STACK2[stack_pointer2] = _STACK[stack_pointer-1];
			_STACK_ELEMENT_TYPE2[stack_pointer2] = _STACK_ELEMENT_TYPE[stack_pointer-1];
			stack_pointer2++;
			drop_element();		
		}
	}else
	{
		printf("Warning: Too much elements to FLIP.\n");
		return -1;
	}
	return 0;
}
/* FLOP
	Take n elements and flop them from the second stack to another.
*/
int flop_n_elements()
{
	int i;
	int top;
	if ( (_STACK_ELEMENT_TYPE[stack_pointer-1] == INT_NUMBER) && 
		((long)_STACK[stack_pointer-1]) <= stack_pointer2)
	{
		//can move elements to _STACK2
		top = (long)_STACK[stack_pointer-1];
		drop_element(); // remove counter from stack
		for(i=0; i<top; i++)
		{
			//move element from one top to another
			if (stack_pointer<MAX_STACK_SIZE)
			{
				_STACK[stack_pointer] = _STACK2[stack_pointer2-1];
				_STACK_ELEMENT_TYPE[stack_pointer] = _STACK_ELEMENT_TYPE2[stack_pointer2-1];
				stack_pointer++;
				drop_element_stack2();			
			}
		}
	}else
	{
		printf("Warning: Too much elements to FLOP.\n");
		return -1;
	}
	return 0;
}

/*
	This parser reads expression from terminal and understand what should be done with
	this expression. 
	1) If it's a number it puts the value into the stack.
	2) if operation 
*/
int terminal ()
{
	int i = 0;
	int j = 0;
	int k;
	char * pch;
	char input[MAX_INPUT_LEN];
	char tmpstr[MAX_INPUT_LEN];
	int rv = 0;
	long itmp = 0;
	int tidx = 0;
	int tres = 0; 
	//int p_counter = 0; //counter of parameters being input in the line.
	//int p_param_type = -1;
	char * s_end;
	double dtmp = 0;
	char skip_token = 0;
	//char fix_zero = 0;
	int input_len = 0;
	int mode = INTERPRETER_MODE;

	init_root();
	init_words();

	while ( 1 )
	{
		//p_counter = 0;
		printf("nic(i)> ");
		gets (input);
		input_len = strlen(input);
		memset(input+input_len, 0, MAX_INPUT_LEN-input_len);
		if ( !strcmp(input, "exit") ) break;
		old_stack_pointer = stack_pointer;
	  	//printf ("Splitting string \"%s\" into tokens:\n", input);
	  	pch = strtok (input," ");
	  	while (pch != NULL)
	  	{
	    	//--printf ("%s\n",pch);
	    	skip_token = 0;
	    	//fix_zero = 0;
	    	//p_counter++;
	    	//p_param_type = -1;
			rv = toktype (pch, mode);

			switch (rv)
			{
				case INT_NUMBER:
				{
					//printf("INT_NUMBER: ");
					//push the number in stack
					itmp = atoi(pch);
					//printf("Integer number [%d]\n", itmp);
					tres = push_num_to_stack(itmp);
					if (tres < 0){
						printf("Stack full, skipping\n");
						// should drop the rest of the input also
						memset(input, 0, MAX_INPUT_LEN); //clear input line
					}
					break;
				}
				case DOUBLE_NUMBER:
				{
					//printf("DOUBLE_NUMBER\n");
					// add to stack
					dtmp = strtod(pch, &s_end);
					//printf("Parsed value: %lf\n", dtmp );
					tres = push_dbl_num_to_stack(dtmp);
					if (tres < 0){
						printf("Stack full, skipping\n");
						// should drop the rest of the input also
						memset(input, 0, MAX_INPUT_LEN); //clear input line
					}
					break;
				}
				case STRING:
				{
					// the string is in the input line extract it and place
					// in stack.
					memset(tmpstr, 0, MAX_INPUT_LEN);
					//printf("String: >%s< \n", pch);

					// define the position of index in the input
					// it's a beggining of the string.
					itmp = ((long) pch) - ((long) input);
					//printf("pch = %x, inp = %x, %d \n", pch, input, itmp);

					// fix zero which is left because of strtok function.
					//printf ("zero: %d, next: %d\n", input[itmp+strlen(pch)],input[itmp+strlen(pch)+1] );
					if ( input[itmp+strlen(pch)+1] !=0 ) input[itmp+strlen(pch)] = ' ';
					
					//now looking the end of the string looking for "
					//for (j=0; j<30; j++) printf("[%d,%d]",j, input[j] );
					//printf("strlen: %d\n", input_len);
					for(j=itmp+1; j<input_len; j++)
					{
						//printf("j=%d\n", j);
						if(input[j] == '"') break;
						tmpstr[j-itmp-1] = input[j];
					}
					//printf("j = %d\n", j );
					printf("String param: '%s'\n", tmpstr);
					add_string(tmpstr);
					// now renew the input because the rest of line can
					// contain other words
					pch = strtok (input+j+1," ");
					skip_token = 1;

					break;
				}
				case RAW_STRING:
				{	
					// this case is happend when we are in
					// WORD_BODY_INPUT_MODE
					memset(tmpstr, 0, MAX_INPUT_LEN);
					// it's a beggining of the string.
					itmp = ((long) pch) - ((long) input);
					printf("pch = %x, inp = %x, %d, [%c] \n", pch, input, itmp, input[itmp]);
					// fix zero which is left because of strtok function.
					printf ("zero: %d, next: %d\n", input[itmp+strlen(pch)],input[itmp+strlen(pch)+1] );
					if ( input[itmp+strlen(pch)+1] !=0 ) input[itmp+strlen(pch)] = ' ';
					// looking for the ending of the string.	
					for(j=itmp; j<input_len; j++)
					{
						if( ((input[j] == ' ') &&
							(input[j+1] == 'I' || input[j+1] == 'i') &&
						 	(input[j+2] == 'S' || input[j+2] == 's')) )
						{
					 		printf("'IS' found#1\n");
							mode = INTERPRETER_MODE;
							tmpstr[j-itmp] = input[j];
							tmpstr[j-itmp+1] = input[j+1];
							tmpstr[j-itmp+2] = input[j+2];
							j++; // need to increment this because last s is the interpreted as a new string
					 		break;
						}
						if (((j == 0)&&
							(input[j] == 'I' || input[j] == 'i') &&
						 	(input[j+1] == 'S' || input[j+1] == 's')) )
						{
							printf("'IS' found#2\n");
							mode = INTERPRETER_MODE;
							tmpstr[j-itmp] = input[j];
							tmpstr[j-itmp+1] = input[j+1];
						 	break;
						}
						tmpstr[j-itmp] = input[j];
					}

					// add this string if it's not the word "is"
					printf("String param: '%s'\n", tmpstr);
					add_string(tmpstr);

					// testing: adding string to word.
					add_string_to_word(); 
					
					// now renew the input because the rest of line can
					// contain other words (skip 'is')
					pch = strtok (input+j+2," ");
					skip_token = 1;


					/*
					for(k = 0; pch[k]; k++){
  						tmpstr[k] = tolower(pch[k]);
					}

					if (0 == strcmp(tmpstr, "is")){
						printf("Switch back to INTERPRETER_MODE\n");
						mode = INTERPRETER_MODE;
					}else
					{
						printf("%s \n", tmpstr);
					}*/

					break;
				}
				case NAME:
				{
					//--printf("NAME\n");
					// define the behaviour of the
					// it can be either name of varable or operator,
					// depending on context.
					tidx = is_keyword(pch);
					if (tidx == 2){ /*show stack*/
						print_stack(); print_stack2();
					}
					if (tidx == 3){ /*DROP N elements*/
						drop_stack_top(); //take the number of elements to be dropped from stack
					}
					if (tidx == 4){ /*DUP licate last element*/
						dup_stack_top();
					}
					if (tidx == 5){ /* ADD interpret as integer add*/
						//add_two_ints();
						add_two_numbers();
					}
					if (tidx == 6){ /* SUBtraction of integers */
						//sub_two_ints();
						sub_two_numbers();
					}
					if (tidx == 7){ /*LIST all keywords*/
						list_keywords();
					}
					if (tidx == 8){ /*SWAP*/
						swap_two_elements();
					}
					if (tidx == 9){ /*ROT*/
						rot_three_elements();
					} 
					if (tidx == 10){ /*TYPE */
						put_type_to_stack();
					}
					if (tidx == 11){ /*PRI*/
						print_top_stack_element();
					}
					if (tidx == 12){ /*PRINt N elements from stack and do CR*/
						print_n_stack_element();
					}
					if (tidx == 13){ //BYTE
						put_type(BYTE);
					}
					if (tidx == 14){ //INT
						put_type(INT_NUMBER);
					}
					if (tidx == 15){ //DOUBLE
						put_type(DOUBLE_NUMBER);
					}
					if (tidx == 16){ // Negation
						mul_minus_one();
					}
					if (tidx == 17){ //"VALU",  //17 // gets value of variable to the stack
						printf("VALU keyword\n");
						get_variable_value();
					}
					if (tidx == 18){ //"EQUL",  //18 // assign value to the variable.
						printf("EQUL keyword\n");
						assign_value_to_var();
					}
					if (tidx == 19){ //"VARL",  Display the list of variables
						var_list();
					}
					if (tidx == 20){ //VAR - creation of variable from the name and type
						create_variable_from_name();
					}
					if (tidx == 21){ //SPEL - prints the content of object and not remove it from list.
						spell_val();
					}
					if (tidx == 22){ //STRI - create string
						create_string_variable();
					}
					if (tidx == 23){ //DEL - Delete variable from memory and from variable stack.
						delete_var();
					}
					if (tidx == 24){ //"WORD",  //24 // Creation of new word.
						printf("WORD keyword\n");
						//new_word_creation();
						// Start the definition of new word 
						// which should be saved as text.
						// switch into the usual line input mode 
						// until "is" word is inputed.

						// The variable in the stack should change
						// the type on WORD or TEXT, and this text could be
						// modified and saved when we get 'IS' keyword.
						set_word_attribute_to_var();

						printf("WORD_BODY_INPUT_MODE\n");
						mode = WORD_BODY_INPUT_MODE;
					}
					if (tidx == 25){ //"IS",    //25 // Word, meaning the end of input of new object, either new word or struct
						printf("IS keyword found, skipping for now, must report error after.\n");	
						printf("INTERPRETER_MODE\n");
						mode = INTERPRETER_MODE;					
					}
					if (tidx == 26){ //"FLIP",  //26 // Move some words from 1st stack to 2nd stack and revert them.
						printf("FLIP keyword\n");
						flip_n_elements();
					}
					if (tidx == 27){ //"FLOP",	 //27 // Return back n elements from 2nd stack to 1st stack in opposit order. 
						//printf("FLOP keyword\n");
						flop_n_elements();
					}
					if (tidx == 28){ //DEEP //28 - put the deep of the stack to the top;
						deep_stack();
					}
					if (tidx == 29){ //MUL //29 - multiply 2 numbers
						mul_two_numbers();
					}
					if (tidx == 30){ //COMP //30 - compile the word. Compiling is done over C code only.
						printf("COMP keyword\n");
					}
					if (tidx == 31){ //TRAN //31 - translate nic code to c code.
						printf("TRAN keyword\n");
					}
					if (tidx == 32){//	"DIM",	 //32 // creation of array index
						printf("DIM keyword\n");
					}
					if (tidx == 33){//	"IDX",	 //33 // getting index of array
						printf("IDX keyword\n");
					}
					if (tidx == 34){// MAP 
						printf("MAP keyword\n");
					}
					if (tidx == 35){//UMAP
						printf("UMAP keyword\n");
					}
					if (tidx == 36){// ARRA - creation of array
						printf("ARRA keyword\n");
					}
					if (tidx == 37){//"WLIS",  //37 // List of words dynamically allocated
						printf("WLIS keyword\n");
						new_words_list();
					}
					if (tidx >= 0) break;
				}
				/*
					If we didn't find key words till this point, it means that we have
					new name, or the name already defined before.
					If the name is new, the next operator is supposed to create new
					variable, but if the name is already defined it can be used as usual.
				*/
				case OPERATOR: /*Or new name*/
				{
					tidx = is_new_name(pch);
					if(tidx)
					{
						//Create new variable to the list. The first time I just add it to the list of vars
						// And put variable address to the stack.
						// Depending on the next keyword it could be either variable of function.
						tidx = add_new_name(pch);
						put_var_to_stack(tidx);
						break;
					}
					//printf("OPERATOR\n");
					tidx = is_variable(pch);
					if (tidx >= 0)
					{
						//adding new name
						put_var_to_stack(tidx);
						break;
					}
					//Execute operator whith parameters in the stack.
					//break;
				}
				case BAD_SYMBOL:
				{
					if (tidx < 0)
					{
						printf("Word is nither keyword nor variable\n");
					}
					printf("BAD_SYMBOL\n");
					//revert to the previous stack state.
					break;
				}
				case UNKNOWN_SYMBOL:
				{
					printf("UNKNOWN_SYMBOL\n");
					// creation and allocation of the parameter in the memory
					break;

				}
				case END_OF_PARSE:
				{
					printf("END_OF_PARSE\n");
					break;
				}
				default: break;
			}
			if (!skip_token){
				pch = strtok (NULL, " ");
    			i++;
    		}
	  	}
	}

  	return i;
}


