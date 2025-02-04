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

	sleep(2);

	if (mkfifo("namedpipe4", 0666) == -1)
    {
        perror("namedpipe4 already exists in writer 2");
        return -1;
    }

	printf("FIFO is made by writer 2 pid = %d\n", getpid());

	int fifo = open("namedpipe4", O_WRONLY);

	if(fifo < 1){
		printf("FIFO descriptor in writer 2: %d", fifo);
		perror("Can't open file in writer 2:");
	}

	printf("FIFO %d was opened in writer 2\n", fifo);

	int res = write(fifo, "Hello, very new world!", 23);
	if(res != 23){
		printf("Wrong number of bytes were written to FIFO by writer 2: %d", res);
	}

	remove("namedpipe4");

	printf("FIFO %d was removed in writer 2\n", fifo);

	close(fifo);

	printf("FIFO was closed in writer 2\n");
	
	return 0;
}

