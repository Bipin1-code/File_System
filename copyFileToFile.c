//Copy a file content to another file.

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *agrv[]){
    if(argc != 3){
    	write(STDOUT_FILENO, "usage: cp src dst\n", 18);
    	return 1;
    }

    int fdIn = open(agrv[1], O_RDONLY);
    int fdtoC = open(agrv[2], O_WRONLY | O_CREAT, O_TRUNC, 0644);

    if( fdIn == -1 || fdtoC == -1){
    	perror("open");
    	return 1;
    }
    char buffer[50];
    ssize_t n;
    while((n = read(fdIn, buffer, sizeof(buffer))) != 0){
    	if(n == -1){
    		perror("read");
    		return 1;
    	}
    	ssize_t off = 0;
    	while(off < n){
    		ssize_t w = write(fdtoC, buffer + off, n - off);
    		off += w;
    	}	
    }
    
    close(fdIn);
    close(fdtoC);
    
	return 0;
}
