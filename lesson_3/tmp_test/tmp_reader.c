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
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

const int arg_num = 2;

int main(int argc, char ** argv){

	(void) umask(0);

	if(argc < arg_num){
		fprintf(stderr, "Wrong number of input arguments. Shold be at least %d\n", arg_num);
		return -1;
	}

	printf("File path is %s\n", argv[1]);

	int fd = open(argv[1], O_RDONLY);
	if(fd == -1){
		perror("Error during file opening");
		return -2;
	}

	sleep(20);

	char buf[30] = {};

	int rd_size = read(fd, buf, 28);
	
	fwrite(buf, 1, rd_size, stdout);
	
	return 0;
}

