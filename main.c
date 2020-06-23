#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "common.h"

int nic_shell( char * file_input, int recompile_flag);

static void usage(FILE *fp, int argc, char **argv)
{
    fprintf(fp,
    	"Usage: %s [options] [script_file]\n\n"
        "Options:\n"
        "-c 				   Recompile words library\n"
        "",
        argv[0]);
}

int main(int argc, char* argv[]){
	//Parse input
	//printf("argv[0] = %s\n", argv[0]);
	if (argc == 1){
		nic_shell( NULL, 0);
	}
	else 
	if (argc == 2 ){
		//printf("argv1= [%s]", argv[1]);
		if (argv[1][0] == '-' && argv[1][1] == 'c' ) { // "-c" option to rebuild library
			nic_shell(NULL, 1);
		}
		else{
			nic_shell( argv[1], 0);
		}
	}
	else if (argc == 3 ){
		if (argv[1][0] == '-' && argv[1][1] == 'c' ) { // "-c" option to rebuild library
			nic_shell( argv[2], 1);
		}
	}else{
		usage(stdout, argc, argv);
	}
	// Other variants not implemented yet.

}

