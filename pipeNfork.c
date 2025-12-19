
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
	int fd[2];

	if(pipe(fd) == -1){
		perror("pipe");
		return 1;
	}
	/*
	//this proves pipe give two file descriptors
	printf("read end = %d\n", fd[0]); //3
	printf("write end = %d\n", fd[1]); //4
    */

	pid_t pid = fork(); //creates two child process almost-exact copy of a caller
	if(pid == 0){
	    close(fd[0]); //close read end
		write(fd[1], "hello from child\n", 17); //write on pipe buffer
		close(fd[1]); //write done , so close write end then read get EOF
	}else{
		// pid > 0 means parent
		close(fd[1]); //close write end
		char buf[64];
		int n = read(fd[0], buf, sizeof(buf)); //read bytes from pipe buffer
		write(STDOUT_FILENO, buf, n); //show buf content on display o
		close(fd[0]);
		wait(NULL);
	}
	//kernal destorys the pipe when all file descriptors referring to it closed.
	
	return 0;
}
