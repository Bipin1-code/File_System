
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static void die(const char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(){
	int fd[2];
	if(pipe(fd) == -1)
		die("pipe");
	    
	pid_t pid = fork();
	if(pid == -1)
		die("fork");
		
	if(pid == 0){
		//child -> producer
		if(close(fd[0]) == - 1)
			die("close read end (child)");
		  
		if(dup2(fd[1], STDOUT_FILENO) == -1)
			die("dup2 stdout");
			
		if(close(fd[1]) == -1)
			die("close write end (child)");
		
		execlp("echo", "echo", "hello", (char*)NULL);
		perror("execlp echo");
		_exit(127);
	}else{
		//parent -> consumer
		if(close(fd[1]) == -1)
			die("close write end (parent)");
			
		if(dup2(fd[0], STDIN_FILENO) == -1)
			die("dup2 stdin");
			
		if(close(fd[0]) == -1)
			die("close read end (parent)");

		execlp("wc", "wc", "-c", NULL);
		perror("execlp wc");
		_exit(127);
	}
}
//So this whole code is equivalent to this: echo hello | wc -c
