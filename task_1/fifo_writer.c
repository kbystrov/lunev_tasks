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

#define FIFO_MODE 0666
#define BUF_MAX_SIZE 100

const char * global_fifo_name = "global_fifo";

int make_global_fifo(){
	//! Пытаемся создать глобальную FIFO для передачи PID процесса writer-а, если она еще не была создана
	(void) umask(0);
	errno = 0;
	if (mkfifo(global_fifo_name, (FIFO_MODE) ) != 0 ){
		if(errno == EEXIST){
        	printf("Global FIFO was created by another process\n");
		} else {
			fprintf(stderr, "ERROR while creating global FIFO");
			return -2;
		}
    } else {
		printf("Global FIFO was created by PID = %d\n", getpid());
	}
	errno = 0;
	//! Открываем ее для передачи и получаем ее FD, что бы потом передать туда свой PID reader-у 
	int global_fifo_id = open(global_fifo_name, O_WRONLY);
	if(global_fifo_id < 1){
		perror("Can't open file in writer:");
		return -3;
	}

	return global_fifo_id;
}

int make_file(const char * argv){

	CHECK_ERR_NULL(argv, -1);

	char * input_file_name = strdup(argv);
	CHECK_ERR_NULL(input_file_name, -2);

	int file_fd = open(input_file_name, O_RDONLY);
	CHECK_ERR(file_fd, -3, "Error while opening input file", 1);

	free(input_file_name);

	return file_fd;
}


int read_file_to_buf(FILE * input_file, char ** buf){

	if(input_file == NULL){
		fprintf(stderr, "Input file is NULL\n");
		return -5;
	}

	if(buf == NULL){
		fprintf(stderr, "Input buffer is NULL\n");
		return -6;
	}

	int res = fseek(input_file, 0L, SEEK_END);
	if(res < 0){
		perror("Can't fseek to end of input file");
		return -7;
	}
	
	long len = ftell(input_file);
	if(len < 0){
		fprintf(stderr, "Wrong size %ld input file\n", len);
		perror("Writer error:");
		return -8;
	}

	#ifdef DEBUG_PRINT_INFO
	printf("File len = %ld\n", len);
	#endif //! DEBUG_PRINT_INFO

	res = fseek(input_file, 0L, SEEK_SET);
	if(res < 0){
		perror("Can't fseek to start of input file:");
		return -9;
	}

	char * read_buf = malloc(len);
	if(read_buf == NULL){
		fprintf(stderr, "Can't allocate buffer of size = %ld bytes in child process\n", len);
		return -10;
	}

	long read_num = 0;
	long new_len = len;
	
	while (read_num < len){

		res = fread(read_buf + read_num, 1, new_len, input_file);
		if(res == 0){
			break;
		}

		read_num += res;
		new_len -= res;
		if(new_len < 1){
			break;
		}

	}

	*buf = read_buf;

	#ifdef DEBUG_PRINT_INFO
	printf("Read num = %ld\n", read_num);
	#endif //! DEBUG_PRINT_INFO

	return read_num;
}

int make_uniq_fifo(char * fifo_name){

	if(fifo_name == NULL){
		fprintf(stderr, "NULL input buffer in make_uniq_fifo()\n");
		return -11;
	}

	(void) umask(0);

	snprintf(fifo_name , BUF_MAX_SIZE, "fifo_%d", getpid());
	printf("Writer creates unique FIFO name = %s\n", fifo_name);

	repeat:
	errno = 0;
	if (mkfifo(fifo_name, (FIFO_MODE) ) == -1){
		if(errno == EEXIST){
			remove(fifo_name);
			goto repeat;
		} else {
        	perror("Unique FIFO already exists");
        	return -12;
		}
    }
	errno = 0;

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s is made by writer pid = %d\n", fifo_name, getpid());
	#endif //! DEBUG_PRINT_INFO

	return 0;
};


int write_to_fifo(int file_fd, int fifo_id){

	/*
	CHECK_ERR_NULL(input_file, -2);

	int res = fseek(input_file, 0L, SEEK_END);
	CHECK_ERR(res, -3, "Can't fseek to end of input file", 1);
	
	long len = ftell(input_file);
	CHECK_ERR(len, -4, "Ftell error", 1);
	*/

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

	/*
	size_t write_num = 0;
	size_t new_len = buf_len;

	long res = 0;

	while ( write_num < buf_len ){

		res = write(fifo_id, buf + write_num, new_len);
		if (res < 0){
			perror("Error during writing to FIFO");
			return -14;
		} else if (res == 0) {
			break;
		}

		write_num += res;
		new_len -= res;
		if(new_len < 1){
			break;
		}
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "Write num = %ld\n", write_num);
	#endif //! DEBUG_PRINT_INFO
	*/

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
		return -1;
	}
	//! Создаем глобальный FIFO для передачи своего PID reader-у
	int global_fifo_id = make_global_fifo();
	if(global_fifo_id < 1){
		fprintf(stderr, "Can't open global FIFO in writer with PID = %d", getpid());
		return global_fifo_id;
	}
	//! Получаем указатель на входной файл
	int file_fd = make_file(argv[1]);
	if(file_fd < 0){
		fprintf(stderr, "Can't open input file writer with PID = %d", getpid());
		return file_fd;
	}
	//! Считываем входной файл в промежуточный буфер
	/*
	char * file_data = NULL;
	long res = read_file_to_buf(input_file, &file_data);
	if (res < 0){
		fprintf(stderr, "Was error %ld in writer\n", res);
		fclose(input_file);
		return res;
	}
	
	long file_len = res;
	*/

	//! Создаем FIFO с уникальным именем в зависимости от PID writer-а для пары wrirer-reader
	char fifo_name[BUF_MAX_SIZE] = {};
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
	//! Открываем уникальную FIFO на запись
	int fifo_id = open(fifo_name, O_WRONLY);
	if(fifo_id < 1){
		perror("Can't open file in writer:");
		close(file_fd);
		return -13;
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

