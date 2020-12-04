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

int make_file(const char * argv){

	CHECK_ERR_NULL(argv, ERR_WRITER_MAKE_FILE_ARG );

	char * input_file_name = strdup(argv);
	CHECK_ERR_NULL(input_file_name, ERR_WRITER_MAKE_FILE_STRDUP);

	int file_fd = open(input_file_name, O_RDONLY);
	CHECK_ERR(file_fd, ERR_WRITER_MAKE_FILE_OPEN, "Error while opening input file", 1);

	free(input_file_name);

	return file_fd;
}

int get_rd_pid(){
	(void) umask(0);
	//! Открываем глобальную FIFO для получения PID reader-а
	int global_fifo_id = open(global_fifo_name, O_RDONLY);
	CHECK_ERR(global_fifo_id, ERR_WRITER_GET_RD_PID_OPEN, "ERROR in writer while opening global FIFO", 1);
	//! Считываем PID очередного writer-а
	int wr_pid = 0;
	int res = read(global_fifo_id, &wr_pid, sizeof(int));
	CHECK_ERR(res, ERR_WRITER_GET_RD_PID_READ, "ERROR while reading from global FIFO in writer", 1);
	if(res != sizeof(int)){
		return (-ERR_WRITER_GET_RD_PID_RSIZE);
	}
	close(global_fifo_id);
	return wr_pid;
}

int make_uniq_fifo_name(char * fifo_name, int rd_pid){

	CHECK_ERR_NULL(fifo_name, ERR_WRITER_MAKE_UNIQ_FIFO_NAME_ARG);

	snprintf(fifo_name , FIFO_NAME_MAX_SIZE, "fifo_%d", rd_pid);

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO %s is made by reader pid = %d\n", fifo_name, getpid());
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

int main(int argc, char **argv){

	if(argc != num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return ERR_WRITER_ARGC;
	}

	//! Получаем указатель на входной файл и открываем его
	int file_fd = make_file(argv[1]);
	if(file_fd < 0){
		fprintf(stderr, "Can't open input file writer with PID = %d", getpid());
		return file_fd;
	}

	//! Открываем глобальную FIFO для получения PID reader-а и считываем его
	int rd_pid = get_rd_pid();
	if(rd_pid < 0){
		fprintf(stderr, "Error while opening global FIFO in writer\n");
		return rd_pid;
	}

	//! Получаем имя уникального FIFO по PID-у reader-а
	char fifo_name[FIFO_NAME_MAX_SIZE] = {};
	int res = make_uniq_fifo_name(fifo_name, rd_pid);
	if(res < 0){
		fprintf(stderr, "Error while making unique FIFO name in reader");
		return res;
	}

	//! Открываем уникальную FIFO на запись в неблокирующем режиме, что бы избежать дедлока
	int fifo_id = open(fifo_name, O_WRONLY | O_NONBLOCK);
	CHECK_ERR(fifo_id, ERR_WRITER_OPEN_UNIQ_FIFO_NBLK, "Can't open unique FIFO in non-blocking mode in writer:", 1);
	
	//! Вызываем remove для уникальной FIFO, что бы гарантировать ее удаление в случае смерти процесса
	res = remove(fifo_name);
	CHECK_ERR(res, ERR_WRITER_UNIQ_FIFO_RMV, "ERROR while removing unique FIFO in writer", 1);
	
	//! Через fnctl меняем на блокирующий режим, что бы нормально передавать данные
	res = fcntl(fifo_id, F_SETFL, O_WRONLY);
	if(res == -1){
		close(fifo_id);
		remove(fifo_name);
		CHECK_ERR(res, ERR_WRITER_FCNTL, "ERROR in writer while changing unique FIFO flags", 1);
	}

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

