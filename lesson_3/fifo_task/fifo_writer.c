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

const int num_argc = 2;

#define FIFO_MODE 0666

const char * fifo_name = "fifo_name";

FILE * make_file(const char * argv){

	if(argv == NULL){
		fprintf(stderr, "Input argv is NULL in make_file()\n");
		return NULL;
	}

	char * input_file_name = strdup(argv);
	if(input_file_name == NULL){
		fprintf(stderr, "Error during strdup() for input file name\n");
		return NULL;
	} 

	FILE * input_file = fopen(input_file_name, "r");
	if (input_file == NULL){
		fprintf(stderr, "Error while opening input file %s\n", input_file_name);
		free(input_file_name);
		return NULL;
	}

	free(input_file_name);

	return input_file;
}


int read_file_to_buf(FILE * input_file, char ** buf){

	if(input_file == NULL){
		fprintf(stderr, "Input file is NULL\n");
		return -1;
	}

	if(buf == NULL){
		fprintf(stderr, "Input buffer is NULL\n");
		return -2;
	}

	int res = fseek(input_file, 0L, SEEK_END);
	if(res < 0){
		perror("Can't fseek to end of input file");
		return -1;
	}
	
	long len = ftell(input_file);
	if(len < 0){
		fprintf(stderr, "Wrong size %ld input file\n", len);
		perror("Writer error:");
		return -2;
	}

	#ifdef DEBUG_PRINT_INFO
	printf("File len = %ld\n", len);
	#endif //! DEBUG_PRINT_INFO

	res = fseek(input_file, 0L, SEEK_SET);
	if(res < 0){
		perror("Can't fseek to start of input file:");
		return -3;
	}

	char * read_buf = malloc(len);
	if(read_buf == NULL){
		fprintf(stderr, "Can't allocate buffer of size = %ld bytes in child process\n", len);
		return -4;
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


int write_to_fifo(int fifo_id, const char * buf, size_t buf_len){

	if(buf == NULL){
		fprintf(stderr, "Input buffer is NULL\n");
		return -2;
	}

	size_t write_num = 0;
	size_t new_len = buf_len;

	long res = 0;

	while ( write_num < buf_len ){

		res = write(fifo_id, buf + write_num, new_len);
		if (res < 0){
			perror("Error during writing to FIFO");
			return -12;
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

	return 0;
}

int main(int argc, char **argv){

	if(argc != num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return -1;
	}

	FILE * input_file = make_file(argv[1]);

	char * file_data = NULL;

	long res = read_file_to_buf(input_file, &file_data);
	if (res < 0){
		fprintf(stderr, "Was error %ld in writer\n", res);
		return res;
	}

	long file_len = res;

	(void) umask(0);

	if (mkfifo(fifo_name, (FIFO_MODE) ) == -1){
        perror("FIFO already exists");
        return -1;
    }

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s is made by writer pid = %d\n", fifo_name, getpid());
	#endif //! DEBUG_PRINT_INFO

	int fifo_id = open(fifo_name, O_WRONLY);
	if(fifo_id < 1){
		perror("Can't open file in writer:");
	}

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s with id = %d was opened in writer\n", fifo_name, fifo_id);
	#endif //! DEBUG_PRINT_INFO

	res = write_to_fifo(fifo_id, file_data, file_len);
	if (res < 0){
		fprintf(stderr, "Was error %ld in writer\n", res);
		return res;
	}

	free(file_data);

	if ( remove(fifo_name) == -1 ){
		fprintf(stderr, "FIFO %s with id = %d in writer\n", fifo_name, fifo_id);
		perror("Can't remove FIFO in writer");
        return -2;
	}

	#ifdef DEBUG_PRINT_INFO
	printf("FIFO %s with id = %d was removed in writer\n", fifo_name, fifo_id);
	#endif //! DEBUG_PRINT_INFO

	if ( close(fifo_id) == -1){
		fprintf(stderr, "FIFO %s with id = %d in writer\n", fifo_name, fifo_id);
		perror("Can't close FIFO in writer");
        return -3;
	}
	
	return 0;
}

