
#include <stdio.h>
#include <unistd.h>

int main(){
	int fd[2]; // int array
	pipe(fd); //create two file descriptor and return those

	//redirect stdout->pipe
	dup2(fd[1], STDOUT_FILENO);//fd = 3 point to kernel object, now fd = 1 point to same object as fd = 3

    write(fd[1], "G\n", 2); //goes to pipe buffer
    printf("H\n"); //ALSO goes to pipe buffer
    	
	return 0;
}
//Nothing appears on the screen
/* Why?
   - stdout (fd = 1) no longer points to the terminal
   - it points to the pipe
*/
