/*
 * print_num.c
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
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>

long get_input_num(const char * argv){
	long n = 0;
	char * ptr;
	errno = 0;
	
	n = strtol(argv, &ptr, 10);
	
	if ( *ptr ) {
		fprintf(stderr, "Input argv %s is not a number!\n", argv);
		return -2;
	}
	
	if ( n < 1 ) {
		fprintf(stderr, "Input number is %s but should be positive non-zero!\n", argv);
		return -3;
	}
	
	if (errno == ERANGE && (n == LONG_MIN || n == LONG_MAX) ){
		fprintf(stderr, "Input number %s is not in long range!\n", argv);
		return -4;
	}

	return n;
}

const int num_argc = 2;

int main(int argc, char **argv){
	
	if(argc != num_argc){
		fprintf(stderr, "Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return -1;
	}

	long chld_num = get_input_num(argv[1]);
	if(chld_num < 1){
		fprintf(stderr, "ERROR %d while get input argument\n", chld_num);
		return chld_num;
	}
	errno = 0;

	long order_id = 1;
	int in_child = 0;

	for(order_id = 1; order_id <= chld_num; order_id++){

		pid_t pid = fork();

		if(pid < 0){
			perror("ERROR while fork child");
			printf("Error! Failure on creating child in %ld process\n", order_id);
			return -5;
		} else if (pid == 0) {
			in_child = 1;
			break;
		}

	}
	
	
	return 0;
}

