/*
* fifo_reader.c
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


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <limits.h>
#include "common.h"


int make_global_fifo(){
	//! Пытаемся создать глобальную FIFO для передачи PID процесса reader-а, если она еще не была создана
	(void) umask(0);
	errno = 0;
	if (mkfifo(global_fifo_name, (FIFO_MODE) ) != 0 ){
		if(errno == EEXIST){
			#ifdef DEBUG_PRINT_INFO
        	printf("Global FIFO was created by another process\n");
			#endif //! DEBUG_PRINT_INFO
		} else {
			fprintf(stderr, "ERROR while creating global FIFO");
			return (-ERR_READER_MAKE_GLOBAL_FIFO_MKFIFO);
		}
    } else {
		#ifdef DEBUG_PRINT_INFO
		printf("Global FIFO was created by PID = %d\n", getpid());
		#endif //! DEBUG_PRINT_INFO
	}
	errno = 0;
	//! Открываем ее для передачи и получаем ее FD, что бы потом передать туда свой PID writer-у 
	int global_fifo_id = open(global_fifo_name, O_WRONLY);
	CHECK_ERR(global_fifo_id, ERR_READER_MAKE_GLOBAL_FIFO_OPEN, "Can't open file in writer:", 1);

	return global_fifo_id;
}

int make_uniq_fifo(char * fifo_name){

	CHECK_ERR_NULL(fifo_name, ERR_READER_MAKE_UNIQ_FIFO_ARG);

	(void) umask(0);
	snprintf(fifo_name , FIFO_NAME_MAX_SIZE, "fifo_%d", getpid());
	#ifdef DEBUG_PRINT_INFO
	printf("Reader creates unique FIFO name = %s\n", fifo_name);
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
				return (-ERR_READER_MAKE_UNIQ_FIFO_MKFIFO_RMV);
			}
		} else {
        	perror("ERROR while creating unique FIFO in reader");
        	return (-ERR_READER_MAKE_UNIQ_FIFO_MKFIFO_OTHER);
		}
    }
	errno = 0;

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s is made by writer pid = %d\n", fifo_name, getpid());
	#endif //! DEBUG_PRINT_INFO

	return 0;
};

int send_pid(int fd){
	int pid = getpid();
	int res = write(fd, &pid, sizeof(int));
	CHECK_ERR(res, ERR_READER_SEND_PID_WRITE, "ERROR in reader while sending its PID to writer", 1);
	if(res != sizeof(int)){
		return (-ERR_READER_SEND_PID_WSIZE);
	}
	return res;
}

int check_wr_end(int fifo_id){
	fd_set rfds;
    struct timeval tv;
    int retval;

	int try_num = 7;

	while(try_num > 0){
		tv.tv_sec = 1;
    	tv.tv_usec = 0;

		FD_ZERO(&rfds);
    	FD_SET(fifo_id, &rfds);

		retval = select(fifo_id + 1, &rfds, NULL, NULL, &tv);
		CHECK_ERR(retval, ERR_READER_CHECK_WR_END, "ERROR in reader while call select for writer end", 1);

		if (retval && FD_ISSET(fifo_id, &rfds) ){
            return 0;
		} else {
			try_num--;
		}

	}

	return -1;
}

int main(){
	
	//! Создаем глобальный FIFO для передачи своего PID writer-у
	int global_fifo_id = make_global_fifo();
	if(global_fifo_id < 1){
		fprintf(stderr, "Can't open global FIFO in reader with PID = %d", getpid());
		return global_fifo_id;
	}

	//! Создаем FIFO с уникальным именем в зависимости от PID reader-а для пары wrirer-reader
	char fifo_name[FIFO_NAME_MAX_SIZE] = {};
	int res = make_uniq_fifo(fifo_name);
	if (res < 0){
		fprintf(stderr, "ERROR = %d in reader at %s: %s in %d line\n", res, __FILE__, __PRETTY_FUNCTION__, __LINE__);
		return res;
	}

	//! Открываем уникальную FIFO для пары writer-reader в неблокирующем режиме, что бы предотвратить дедлок
	(void) umask(0);
	int fifo_id = open(fifo_name, O_RDONLY | O_NONBLOCK);
	CHECK_ERR(fifo_id, ERR_READER_UNIQFIFO_OPEN_NBLK , "ERROR while opening unique FIFO in reader in non-blocking mode", 1);
	
	//! Через fnctl меняем на блокирующий режим, что бы нормально передавать данные
	res = fcntl(fifo_id, F_SETFL, O_RDONLY);
	if(res == -1){
		close(global_fifo_id);
		remove(fifo_name);
		CHECK_ERR(res, ERR_READER_FCNTL, "ERROR in reader while changing unique FIFO flags", 1);
	}

	//! Посылаем через глобальный FIFO свой PID writer-у, по которому он откроет уникальную FIFO для записи
	res = send_pid(global_fifo_id);
	if(res < 0){
		fprintf(stderr, "Was error %d in reader\n", res);
		close(global_fifo_id);
		remove(fifo_name);
		return res;
	}

	//! Ждем когда writer будет готов к записи, опрашивая его через select фикс. число попыток
	res = check_wr_end(fifo_id);
	close(global_fifo_id);
	if(res == -1){
		remove(fifo_name);
		fprintf(stderr, "ERROR in reader: writer is not ready\n");
		return res;
	}

	//! Считываем данные из FIFO и печатаем их в stdout 
	long rd_res = 0;
	long read_len = 0;
	char buf[PIPE_BUF] = {};

	while ( (rd_res = read(fifo_id, buf, PIPE_BUF)) > 0 ) {
		fwrite(buf, 1, rd_res, stdout);
		read_len += rd_res;
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "Reader has read %ld bytes\n", read_len);
	#endif //! DEBUG_PRINT_INFO
	
	return 0;
}

