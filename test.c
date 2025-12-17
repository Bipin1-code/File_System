#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

void showContent(int fd){
	char buffer[50];
	ssize_t nBytesRead, nBytesWritten;
	while((nBytesRead = read(fd, buffer, sizeof(buffer))) != 0){
        if(nBytesRead == -1){
        	perror("read");
        	return;
        }
		nBytesWritten = 0;
		while(nBytesWritten < nBytesRead){
			ssize_t written = write(STDOUT_FILENO, buffer + nBytesWritten, nBytesRead - nBytesWritten);
            if(written == -1){
            	perror("write");
            	return;
            }
            nBytesWritten += written;
		}
	}
}

int main(){
  	const char *fileName = "textFile.txt";

  	if(access(fileName, F_OK) != 0){
  		printf("File %s not found.\n", fileName);
  		return 1;
  	}
  	
    int fd = open(fileName, O_RDONLY);
    if(fd == -1){
    	perror("open");
    	return 1;
    }
    
  	showContent(fd);
  	close(fd);
	return 0;
}
