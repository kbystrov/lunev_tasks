/*
 * fork_n_child.c
 * 
 * Copyright 2020 Student <student@client112>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

const int num_argc = 2;

extern char **environ;

int main(int argc, char **argv){

    printf("Environ is %s,", environ[0]);
    fflush(stdout);

    char * env_path_ptr = getenv("PATH");

    while(*env_path_ptr != '\0'){
        env_path_ptr++;
    }
    
    //printf("First arg is %s\n", argv[0]);
	
	if(argc < num_argc){
		printf("Wrong number of argc = %d! Should be >= %d!\n", argc, num_argc);
		return -1;
	}

    int count = 0;
    
    while(!strcmp(argv[0], argv[count+1])){
        count++;
    }

    char * args[] = {}; 

    for (int i = count; i < argc; i++){
        args[i] = argv[i+1];
    }

    int res = execvp(args[0], &args[0]);
    if(res < 0){
        perror("Error during exec() call\n");
    }
	
	return 0;
}

