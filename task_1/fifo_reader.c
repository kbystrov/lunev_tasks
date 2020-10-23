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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>

const char * global_fifo_name = "global_fifo";

#define BUF_SIZE 4096

#define FIFO_MODE 0666
#define MAX_NAME_SIZE 100

int make_uniq_fifo_name(char * fifo_name, int wr_pid){

	if(fifo_name == NULL){
		fprintf(stderr, "NULL input buffer in make_uniq_fifo()\n");
		return -3;
	}

	(void) umask(0);

	snprintf(fifo_name , MAX_NAME_SIZE, "fifo_%d", wr_pid);
	fprintf(stderr, "Reader creates unique FIFO name = %s\n", fifo_name);

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO %s is made by reader pid = %d\n", fifo_name, getpid());
	#endif //! DEBUG_PRINT_INFO

	return 0;
};

int main(){

	(void) umask(0);
	//! Открываем глобальную FIFO для получения PID writer-а
	int global_fifo_id = open(global_fifo_name, O_RDONLY);
	if(global_fifo_id < 1){
		fprintf(stderr, "Can't open global FIFO in reader");
		return -1;
	}
	//! Считываем PID очередного writer-а
	int wr_pid = 0;
	int res = read(global_fifo_id, &wr_pid, sizeof(int));
	if(res < 0){
		perror("Error while opening global FIFO in reader");
		return -2;
	}
	//! Получаем имя уникального FIFO по PID-у writer-а
	char fifo_name[100] = {};
	res = make_uniq_fifo_name(fifo_name, wr_pid);
	if(res < 0){
		fprintf(stderr, "Error while making unique FIFO name in reader");
		return res;
	}
	

	(void) umask(0);

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO reader pid = %d\n", getpid());
	#endif //! DEBUG_PRINT_INFO

	int fifo_id = open(fifo_name, O_RDONLY);
	if(fifo_id < 0){
		perror("ERROR while opening unique FIFO in reader");
		return -4;
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO %d was opened in reader: %s\n", fifo_id, fifo_name);
	#endif //! DEBUG_PRINT_INFO

	if ( remove(fifo_name) == -1 ){
		fprintf(stderr, "FIFO %s with id = %d in reader\n", fifo_name, fifo_id);
		perror("Can't remove FIFO in writer");
        return -2;
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO %d was removed in reader\n", fifo_id);
	#endif //! DEBUG_PRINT_INFO

	long rd_res = 0;
	long read_len = 0;
	char buf[BUF_SIZE] = {};

	while ( (rd_res = read(fifo_id, buf, BUF_SIZE)) > 0 ) {
		fwrite(buf, 1, rd_res, stdout);
		read_len += rd_res;
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "Reader has read %ld bytes\n", read_len);
	#endif //! DEBUG_PRINT_INFO

	close(fifo_id);
	if (res < 0){
		fprintf(stderr, "Error result %d while reading in parent process\n", res);
		perror("Errno message:");
		return -13;
	}
	
	return 0;
}

