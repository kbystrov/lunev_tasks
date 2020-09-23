/*
 * threads_create.c
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
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

const int num_argc = 3;
long var = 0;

struct var_struct {
	pthread_t thread_id;
	long iters;
	long * var_addr;
	void * retval;
};

static void * thread_start(void *arg){
	
	if(arg == NULL){
		printf("ERROR! null input in %s: PID: %d", __PRETTY_FUNCTION__, getpid());
		return 0;
	}
	
    long iters = ((struct var_struct *) arg)->iters;
    long * var_addr = ((struct var_struct *) arg)->var_addr;
           
    if(var_addr == NULL){
		printf("ERROR! arg->var_addr == NULL in %s: arg = %p; PID: %d", __PRETTY_FUNCTION__, arg, getpid());
		return 0;
	}
           
    for(int i = 0; i < iters; i++){
		(*var_addr)++;	   
	}
	
	pthread_exit(  ((struct var_struct *) arg)->retval );

    return 0;
 }


int main(int argc, char **argv){
	
	if(argc != num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return -1;
	}
	
	char * ptr;
	
	long n_threads = strtol(argv[1], &ptr, 10);
	
	if ( *ptr ) {
		printf("Input argv %s is not a number!\n", argv[1]);
		return -2;
	}
	
	if ( n_threads < 1 ) {
		printf("Thread number is %s but should be positive non-zero!\n", argv[1]);
		return -3;
	}

	if (n_threads >= INT_MAX || n_threads <= INT_MIN){
		printf("Thread number %s is not in int range!\n", argv[1]);
		return -4;
	}
	
	long num_iter = strtol(argv[2], &ptr, 10);
	
	if ( *ptr ) {
		printf("Iterations number %s is not a number!\n", argv[2]);
		return -5;
	}
	
	if ( num_iter < 1 ) {
		printf("Iterations number is %s but should be positive non-zero!\n", argv[2]);
		return -6;
	}
	
	if (num_iter >= INT_MAX || num_iter <= INT_MIN){
		printf("Thread number %s is not in int range!\n", argv[2]);
		return -7;
	}
	
	struct var_struct * arg_struct = calloc(n_threads, sizeof(struct var_struct));
	if(arg_struct == NULL){
		printf("Error allocating memory for %ld threads!\n", n_threads);
		return -8;
	}
	
	const char * rety = "Error!\n";
	void * ret_status = (void *) rety;
	

	for(int i = 0; i < n_threads; i++){
		
		arg_struct[i].iters = num_iter;
		arg_struct[i].var_addr = &var;
		arg_struct[i].retval = ret_status;
		
		int res = pthread_create(&arg_struct[i].thread_id, NULL, &thread_start, &arg_struct[i]);
		
		if(res != 0){
			perror("Error in pthread_create!\n");
			return res;
		}
		
	}
	
	for(int i = 0; i < n_threads; i++){
		/*
		int res_cancl = pthread_cancel(arg_struct[i].thread_id);
		if(res_cancl == 0){
			printf("Thread %ld was cancelled!\n", arg_struct[i].thread_id );
		}*/
		
		int res = pthread_join(arg_struct[i].thread_id, &ret_status);
        if (res != 0){
			perror("Error in pthread_create!\n");
			return res;
		}

        printf("Joined with thread %ld; returned value was %s\n", arg_struct[i].thread_id, (char *) ret_status);
        //free(ret_status);     
	}
	
	free(arg_struct);
	
	printf("THREADS: %ld; ITERS: %ld; FINAL RESULT: %ld\n", n_threads, num_iter, var);
	
	return 0;
}

