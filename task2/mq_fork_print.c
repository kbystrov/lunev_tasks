/*
 * mq_fork_print.c
 * 
 * Copyright 2020 Kirill Bystrov <kirill.bystrov@gmail.com>
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

typedef struct msgbuf {
    long mtype;
    long order_id; //< Is send from child to parent to notify the last that child has made printf
} mybuf;



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

	if (n == LONG_MAX){
		fprintf(stderr, "LONG_MAX children is not allowed!\n");
		return -5;
	}

	return n;
}

//! On succes return creation order id for child (>= 1), 0 - for parent, and -1 in case of error.
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
	#ifdef DEBUG_PRINTS
	printf("Msgq was deleted by %d!\n", getpid());
	fflush(stdout);
	#endif //! DEBUG_PRINTS
	return res;
}

/** Send to appropriate child message to make printf, waits for it answer and repeats it again for all other children. 
  * At the end - deletes message queue. 
*/
int parent_processing(key_t msqid, long chld_num){

	mybuf msg;
	long msg_to_par_type = chld_num + 1;
	size_t maxlen = sizeof(msg.order_id);

	int res = 0;

	for(long i = 1; i <= chld_num; i++){

			msg.mtype = i;

			#ifdef DEBUG_PRINTS
			printf("msqid = %d; msg.mtype = %ld;\n", msqid, msg.mtype);
			fflush(stdout);
			#endif //! DEBUG_PRINTS

			res = msgsnd(msqid, (void *) &msg, 0, 0); //< Parent notifies i-th child to make printf
			if(res == -1){
				perror("ERROR while parent sends messages to childs");
				break;
			}
			
			maxlen = msgrcv(msqid, (void *) &msg, maxlen, msg_to_par_type, 0); //< Parent waits notification form child that it has made printf
			if(msg.order_id != i){
				fprintf(stderr, "ERROR: Wrong message from child! Should be %ld, but it is %ld\n", i, msg.order_id);
				break;
			}

			#ifdef DEBUG_PRINTS
			printf("After PARENT get from child: msqid = %d; msg.mtype = %ld; msg.order_id = %ld\n", msqid, msg.mtype, msg.order_id);
			fflush(stdout);
			#endif //! DEBUG_PRINTS

	}
	//! Parent kills message queue at the end
	res = delete_msgq(msqid);
	return res;
}
//! Waits notification form parent to start printfing, prints its order id and PID and notifies parent that it has made its printf.
int child_processing(key_t msqid, long total_chld_num, long order_id){

	mybuf msg;
	long msg_to_par_type = total_chld_num + 1;
	size_t maxlen = sizeof(msg.order_id);

	#ifdef DEBUG_PRINTS
	printf("order_id = %ld, msqid = %d\n", order_id, msqid);
	fflush(stdout);
	#endif //! DEBUG_PRINTS

	int res = msgrcv(msqid, (void *) &msg, 0, order_id, 0); //< Child waits notification form parent to start printfing
	if(res == -1){
		perror("ERROR in child while getting message from parent");
		fflush(stderr);
		return -1;
	}

	printf("[%ld] child PID = %d\n", order_id, getpid());

	#ifdef DEBUG_PRINTS
	printf("msqid = %d; msg.mtype = %ld;\n", msqid, msg.mtype);
	fflush(stdout);
	#endif //! DEBUG_PRINTS
	
	msg.mtype = msg_to_par_type;
	msg.order_id = order_id;
	res = msgsnd(msqid, (void *) &msg, maxlen, 0); //< i-th child notifies parent that it has made printf

	if(res == -1){
		perror("ERROR while child sends messages to parent");
		return -1;
	}

	#ifdef DEBUG_PRINTS
	printf("After CHILD send to PARENT: msqid = %d; msg.mtype = %ld; msg.order_id = %ld\n", msqid, msg.mtype, msg.order_id);
	fflush(stdout);
	#endif //! DEBUG_PRINTS

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

	if(order_id == 0){ 	//! Parent processing
		res = parent_processing(msqid, chld_num);
	} else {			//! Child processing
		res = child_processing(msqid, chld_num, order_id);
	}

	return res;
}

