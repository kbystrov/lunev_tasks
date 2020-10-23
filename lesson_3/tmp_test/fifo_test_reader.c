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

const int num_argc = 2;

#define BUF_SIZE 5000

int main(){

	(void) umask(0);

	printf("FIFO reader pid = %d\n", getpid());
	/*
	int fifo = open("namedpipe4", O_RDONLY);

	if(fifo < 1){
		printf("FIFO descriptor in reader: %d", fifo);
		perror("Can't open file in reader:");
		return -1;
	}
	*/

	int fifo = -1; 

	while ( (fifo = open("namedpipe4", O_RDONLY)) < 1){
		printf("Can't open namedpipe4\n");
	}

	//remove("namedpipe4");

	//printf("FIFO %d was removed in reader\n", fifo);

	printf("FIFO %d was opened in reader\n", fifo);

	int buf = 0;
	
	int res = read(fifo, &buf, sizeof(int));
	if(res < 0){
		perror("Reading error in reader:");
		return -1;
	}

	printf("Fisrt read result: %d\n", buf);

	res = read(fifo, &buf, sizeof(int));
	if(res < 0){
		perror("Reading error in reader:");
		return -1;
	}

	printf("Second read result: %d\n", buf);
	/*
	int res = read(fifo, buf, sizeof(int));
	if(res < 0){
		perror("Reading error in reader:");
		return -1;
	}

	printf("Second write result: %s\n", buf);
	*/

	return 0;
}

