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

const int num_argc = 2;

int main(int argc, char **argv){
	
	printf("Start of program!\n");
	
	if(argc != num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return -1;
	}
	
	long n = 0;
	char * ptr;
	errno = 0;
	
	n = strtol(argv[1], &ptr, 10);
	
	if ( *ptr ) {
		printf("Input argv %s is not a number!\n", argv[1]);
		return -3;
	}
	
	if ( n < 1 ) {
		printf("Input number is %s but should be positive non-zero!\n", argv[1]);
		return -4;
	}
	
	if (errno == ERANGE && (n == LONG_MIN || n == LONG_MAX) ){
		printf("Input number %s is not in long range!\n", argv[1]);
		return -5;
	}
	
	for(long i = 1; i <= n; i++){
		printf("%ld\n", i);
	}
	
	return 0;
}

