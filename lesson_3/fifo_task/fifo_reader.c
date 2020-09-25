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

const char * fifo_name = "fifo_name";

#define BUF_SIZE 4096

int main(){

	(void) umask(0);

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO reader pid = %d\n", getpid());
	#endif //! DEBUG_PRINT_INFO

	int fifo_id = open(fifo_name, O_RDONLY);

	if(fifo_id < 1){
		printf("FIFO descriptor in reader: %d", fifo_id);
		perror("Can't open file in reader:");
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO %d was opened in reader: %s\n", fifo_id, fifo_name);
	#endif //! DEBUG_PRINT_INFO

	long res = 0;
	long read_len = 0;
	char buf[BUF_SIZE] = {};

	while ( (res = read(fifo_id, buf, BUF_SIZE)) > 0 ) {
		fwrite(buf, 1, res, stdout);
		read_len += res;
	}

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "Reader has read %ld bytes\n", read_len);
	#endif //! DEBUG_PRINT_INFO

	close(fifo_id);
	if (res < 0){
		fprintf(stderr, "Error result %ld while reading in parent process\n", res);
		perror("Errno message:");
		return -13;
	}

	remove(fifo_name);

	#ifdef DEBUG_PRINT_INFO
	fprintf(stderr, "FIFO %d was removed in reader\n", fifo_id);
	#endif //! DEBUG_PRINT_INFO
	
	return 0;
}

