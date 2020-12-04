/*
 * fifo_writer.c
 * 
 * Copyright 2020 Kirill Bystrov <kirill.bystrov@phystech.edu>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "common.h"

const int num_argc = 2;

int make_global_fifo(){
	//! Пытаемся создать глобальную FIFO для передачи PID процесса writer-а, если она еще не была создана
	(void) umask(0);
	errno = 0;
	if (mkfifo(global_fifo_name, (FIFO_MODE) ) != 0 ){
		if(errno == EEXIST){
			#ifdef DEBUG_PRINT_INFO
        	printf("Global FIFO was created by another process\n");
			#endif //! DEBUG_PRINT_INFO
		} else {
			fprintf(stderr, "ERROR while creating global FIFO");
			return ERR_WRITER_MAKE_GLOBAL_FIFO_MKFIFO;
		}
    } else {
		#ifdef DEBUG_PRINT_INFO
		printf("Global FIFO was created by PID = %d\n", getpid());
		#endif //! DEBUG_PRINT_INFO
	}
	errno = 0;
	//! Открываем ее для передачи и получаем ее FD, что бы потом передать туда свой PID reader-у 
	int global_fifo_id = open(global_fifo_name, O_WRONLY);
	CHECK_ERR(global_fifo_id, ERR_WRITER_MAKE_GLOBAL_FIFO_OPEN, "Can't open file in writer:", 1);

	return global_fifo_id;
}

int make_file(const char * argv){

	CHECK_ERR_NULL(argv, ERR_WRITER_MAKE_FILE_ARG);

	char * input_file_name = strdup(argv);
	CHECK_ERR_NULL(input_file_name, ERR_WRITER_MAKE_FILE_STRDUP);

	int file_fd = open(input_file_name, O_RDONLY);
	CHECK_ERR(file_fd, ERR_WRITER_MAKE_FILE_OPEN, "Error while opening input file", 1);

	free(input_file_name);

	return file_fd;
}

int make_uniq_fifo(char * fifo_name){

	CHECK_ERR_NULL(fifo_name, ERR_WRITER_MAKE_UNIQ_FIFO_ARG);

	(void) umask(0);
	snprintf(fifo_name , FIFO_NAME_MAX_SIZE, "fifo_%d", getpid());
	#ifdef DEBUG_PRINT_INFO
	printf("Writer creates unique FIFO name = %s\n", fifo_name);
	#endif //! DEBUG_PRINT_INFO

	int try_left = 5;
	repeat:
	errno = 0;
	if (mkfifo(fifo_name, (FIFO_MODE) ) == -1){
		if(errno == EEXIST){
			if(try_left > 0 ){
				remove(fifo_name);
				try_left--;
				goto repeat;
			} else {
				return ERR_WRITER_MAKE_UNIQ_FIFO_MKFIFO_RMV;
			}
		} else {
        	perror("ERROR while creating unique FIFO in writer");
        	return ERR_WRITER_MAKE_UNIQ_FIFO_MKFIFO_OTHER;
		}
    }
	errno = 0;

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s is made by writer pid = %d\n", fifo_name, getpid());
	#endif //! DEBUG_PRINT_INFO

	return 0;
};


int write_to_fifo(int file_fd, int fifo_id){

	long rd_res = 0;
	long wr_res = 0;
	char tmp_buf[PIPE_BUF] = {};

	while ( ( rd_res = read(file_fd, tmp_buf, PIPE_BUF) ) > 0 ){

		CHECK_ERR(rd_res, -15, "Error during reading from input file in writer", 1);

		wr_res = write(fifo_id, tmp_buf, rd_res);
		CHECK_ERR(wr_res, -14, "Error during writing to FIFO", 1);
		if (wr_res == 0) {
			break;
		}

	}

	return 0;
}

int send_pid(int fd){
	int pid = getpid();
	int res = write(fd, &pid, sizeof(int));
	if(res != sizeof(int)){
		return -15;
	}
	return res;
}

int main(int argc, char **argv){

	if(argc != num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return ERR_WRITER_ARGC;
	}
	//! Получаем указатель на входной файл
	int file_fd = make_file(argv[1]);
	if(file_fd < 0){
		fprintf(stderr, "Can't open input file writer with PID = %d", getpid());
		return file_fd;
	}
	//! Создаем глобальный FIFO для передачи своего PID reader-у
	int global_fifo_id = make_global_fifo();
	if(global_fifo_id < 1){
		fprintf(stderr, "Can't open global FIFO in writer with PID = %d", getpid());
		return global_fifo_id;
	}
	//! Создаем FIFO с уникальным именем в зависимости от PID writer-а для пары wrirer-reader
	char fifo_name[FIFO_NAME_MAX_SIZE] = {};
	int res = make_uniq_fifo(fifo_name);
	if(res < 0){
		fprintf(stderr, "Was error %d in writer\n", res);
		close(file_fd);
		return res;
	}
	//! Посылаем через глобальный FIFO свой PID reader-у, по которому он откроет уникальную FIFO для чтения
	res = send_pid(global_fifo_id);
	if(res < 0){
		fprintf(stderr, "Was error %d in writer\n", res);
		close(file_fd);
		return res;
	}
	//! Открываем уникальную FIFO на запись в неблокирующем режиме, что бы избежать дедлока
	int fifo_id = open(fifo_name, O_WRONLY);
	CHECK_ERR(fifo_id, ERR_WRITER_OPEN_UNIQ_FIFO_NBLK, "Can't open unique FIFO in non-blocking mode in writer:", 1);

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s with id = %d was opened in writer\n", fifo_name, fifo_id);
	#endif //! DEBUG_PRINT_INFO

	//! Пишем данные в уникальную FIFO
	res = write_to_fifo(file_fd, fifo_id);
	if (res < 0){
		fprintf(stderr, "Was error %d in writer\n", res);
		close(file_fd);
		return res;
	}

	if ( remove(fifo_name) == -1 ){
		fprintf(stderr, "FIFO %s with id = %d in writer\n", fifo_name, fifo_id);
		perror("Can't remove FIFO in writer");
        return -16;
	}

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s with id = %d was removed in writer\n", fifo_name, fifo_id);
	#endif //! DEBUG_PRINT_INFO

	if ( close(fifo_id) == -1){
		fprintf(stderr, "FIFO %s with id = %d in writer\n", fifo_name, fifo_id);
		perror("Can't close FIFO in writer");
        return -17;
	}
	
	return 0;
}

