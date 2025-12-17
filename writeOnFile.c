
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void writeInfile(int fd){
	printf("You can now write.\n <press `~` when you done.>\n");
	fflush(stdout);
	char buffer[1024];
	ssize_t n;
    while((n = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0){
         for(ssize_t i = 0; i < n; i++){
             if(buffer[i] == '~'){
             	ssize_t off = 0;
             	while(off < i){
             		ssize_t w = write(fd, buffer + off, i - off);
             		if(w == -1){
             			perror("write");
             			return;
             		}
             		off += w;
             	}
             	return;
		     }		
         }
         ssize_t off = 0;
         while(off < n){
         	ssize_t w = write(fd, buffer + off, n - off);
         	if(w == -1){
         		perror("write");
         		return;
         	}
         	off += w;
         }
    }
}

int main(){
    const char *fileName = "writeText.txt";
    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0){
    	perror("open");
	}
    writeInfile(fd);
	close(fd);
	return 0;
}
