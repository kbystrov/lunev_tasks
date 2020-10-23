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

int main(){

	(void) umask(0);

	FILE * tmp_file = tmpfile();
	if(tmp_file == NULL){
		perror("Error while creating temporary file");
		return -1;
	}

	int fd = fileno(tmp_file);
	if(fd < 0){
		perror("Error during examining file descriptor of TMP file");
		return -2;
	}
		
	printf("Tmp file is created with fd = %d; PID = %d\n", fd, getpid());

	int wr_size = write(fd, "Hello from another process\n", 28);
	if(wr_size != 28){
		fprintf(stderr, "Wrong number of written bytes = %d, but should be %d", wr_size, 28);
		perror("Error while writing to TMP file");
		return -3;
	}
	
	while(1){

	}

	if ( fclose(tmp_file) ){
		perror("Error while closing TMP file");
	}
	
	return 0;
}

