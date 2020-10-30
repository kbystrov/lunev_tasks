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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

const int num_argc = 2;
#define MSQ_MODE 0666


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

//! On succes return craetion order id for child (<= 1), 0 - for parent, and -1 in case of error.
long fork_n_childs(long chld_num){

	if(chld_num < 1){
		fprintf(stderr, "Error! Negative input number at %s\n", __PRETTY_FUNCTION__);
		return -1;
	}

	for(long order_id = 1; order_id <= chld_num; order_id++){

		pid_t pid = fork();

		if(pid < 0){
			perror("ERROR while fork child");
			fprintf(stderr, "Error! Failure on creating child in %ld process\n", order_id);
			return -1;
		} else if (pid == 0) { //< Child
			#ifdef DEBUG_PRINTS
			printf("CHILD: have created child num = %ld with PID = %d\n", order_id, getpid() );
			#endif //! DEBUG_PRINTS
			return order_id;
		} else {				//< Parent
			#ifdef DEBUG_PRINTS
			printf("PARENT: have created child num = %ld with PID = %d\n", order_id, pid);
			#endif //! DEBUG_PRINTS
		}

	}

	return 0;
}

int delete_msgq(key_t msqid){
	int res = msgctl(msqid, IPC_RMID, NULL);
	if(res == -1){
		perror("ERROR during deleting message queue");
	}
	return res;
}

int main(int argc, char **argv){

	int res = 0;
	
	if(argc != num_argc){
		fprintf(stderr, "Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return -1;
	}
	//! Reads input argument
	long chld_num = get_input_num(argv[1]);
	if(chld_num < 1){
		fprintf(stderr, "ERROR %ld while get input argument\n", chld_num);
		return chld_num;
	}
	errno = 0;
	//! Parent creates unique message queue before forking children
	key_t msqid = msgget(IPC_PRIVATE, MSQ_MODE);
	if(msqid == -1){
		perror("ERROR while creating message queue");
		return -1;
	}
	//! Trying forking n childs. If it fails - parent kills message queue
	long order_id = fork_n_childs(chld_num);
	if (order_id == -1){
		fprintf(stderr, "ERROR while forking children\n");
		res = delete_msgq(msqid);
		return res;
	}

	//! Parent kills message queue at the end
	if(order_id == 0){
		res = delete_msgq(msqid);
	}
	
	return res;
}

