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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

const int num_argc = 2;

#define BUF_SIZE 5000

int main(int argc, char **argv){
	
	if(argc != num_argc){
		printf("Wrong number of argc = %d! Should be %d!\n", argc, num_argc);
		return -1;
	}

	char * input_file_name = strdup(argv[1]);
	if(input_file_name == NULL){
		fprintf(stderr, "Error during strdup() for input file name\n");
		return -2;
	} 

	FILE * input_file = fopen(input_file_name, "r");

	if (input_file == NULL){
		fprintf(stderr, "Error while opening input file %s\n", input_file_name);
		free(input_file_name);
		return -3;
	}

	int fds[2];
	char buf[BUF_SIZE] = {};

	int res = pipe(fds);
	if(res == -1){
		perror("Error during pipe\n");
		return -4;
	}
			
	pid_t id = fork();

	if(id < 0){
		perror(NULL);
		printf("Error! Failure on creating child\n");
		return -5;
	} else if(id == 0){ ///< Child process
		close(fds[0]);

		int res = fseek(input_file, 0L, SEEK_END);
		if(res < 0){
			fprintf(stderr, "Can't fseek to end of input file %s\n", input_file_name);
			perror("Errno message:");
			close(fds[1]);
			return -8;
		}
		
		long len = ftell(input_file);
		if(len < 0){
			fprintf(stderr, "Wrong size %ld input file %s\n", len, input_file_name);
			perror("Errno message:");
			close(fds[1]);
			return -9;
		}

		#ifdef DEBUG_PRINT
		fprintf(stderr, "File len = %ld\n", len);
		#endif //! DEBUG_PRINT

		res = fseek(input_file, 0L, SEEK_SET);
		if(res < 0){
			fprintf(stderr, "Can't fseek to start of input file %s\n", input_file_name);
			perror("Errno message:");
			close(fds[1]);
			return -10;
		}

		char * read_buf = malloc(len);
		if(read_buf == NULL){
			fprintf(stderr, "Can't allocate buffer of size = %ld bytes in child process\n", len);
			return -11;
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

		#ifdef DEBUG_PRINT
		fprintf(stderr, "Read num = %ld\n", read_num);
		#endif //! DEBUG_PRINT

		long write_num = 0;
		new_len = len;

		while ( write_num < len ){

			res = write(fds[1], read_buf + write_num, new_len);
			if (res < 0){
				perror("Error during write:");
				close(fds[1]);
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

		#ifdef DEBUG_PRINT
		fprintf(stderr, "Write num = %ld\n", write_num);
		#endif //! DEBUG_PRINT

		close(fds[1]);
		free(read_buf);

	} else { ///< Parent process
		close(fds[1]);

		int res = 0;
		long new_read_len = 0;

		while ( (res = read(fds[0], buf, BUF_SIZE)) > 0 ) {
			fwrite(buf, 1, res, stdout);
			new_read_len += res;
		}

		#ifdef DEBUG_PRINT
		fprintf(stderr, "New read len = %ld\n", new_read_len);
		#endif //! DEBUG_PRINT

		close(fds[0]);

		if (res < 0){
			fprintf(stderr, "Error result %d while reading in parent process\n", res);
			perror("Errno message:");
			return -13;
		}
		
	}
	
	return 0;
}

