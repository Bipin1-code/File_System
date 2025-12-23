#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

typedef enum{
	OP_STAT = 0,
	OP_DIR,
	OP_RMDIR,
	OP_DELFL,
	OP_DELREC,
    OP_INVALID
} FileOp;

FileOp parseOp(const char *s){
	if(strcmp(s, "myStat") == 0) return OP_STAT;
	if(strcmp(s, "myMkDir") == 0) return OP_DIR;
	if(strcmp(s, "myRmDir") == 0) return OP_RMDIR;
	if(strcmp(s, "delFile") == 0) return OP_DELFL;
	if(strcmp(s, "rmRecursive") == 0) return OP_DELREC;
	return OP_INVALID;
}
//stat() 
int myStat(const char *fileName){
	struct stat st;
	if(stat(fileName, &st) == -1){
		perror("stat");
		return -1;
	}
	printf("File = %s\nInode = %lu\nSize = %ld bytes\n\
	link = %lu\n", fileName, 
    (unsigned long)st.st_ino,
    (long) st.st_size,
    (unsigned long)st.st_nlink);
    
    //mode_t has three type file type, special bits, Permission
    //for to get type we have mask S_IFMT, 07000, 0777 respectively
    mode_t type = st.st_mode & S_IFMT; 

    if(type == S_IFREG)
    	printf("Type: regular file\n");
    else if(type == S_IFDIR)
    	printf("Type: directory\n");
    else if(type == S_IFLNK)
    	printf("Type: symlink\n");
    else
       	printf("Type: other\n");

   printf("Mode: %06o\n", st.st_mode & 0777);
   time_t t = st.st_mtim.tv_sec;
   struct tm *tm = localtime(&t);
   
   printf("Time = %04d-%02d-%02d %02d:%02d:%02d\n",
          tm->tm_year + 1900,
          tm->tm_mon + 1,
          tm->tm_mday,
          tm->tm_hour,
          tm->tm_min,
          tm->tm_sec);

   return 0;
}

//mkdir()
int myMkDir(const char *dirName){
    //we decide to use 0777 because dir should be able read, write and execute for all;
    //keep in mode_t 0777 gonna be umake ~(0022); ulimately it becomes 0777 & 0755  = 755
	if(mkdir(dirName, 0777) == -1){
		if(errno == EEXIST){
			printf("%s already exists.\n", dirName);
			return -1;
		}
		else if(errno == ENOENT){
			printf("A component of the path does not exist.\n");
			return -1;	
		}
		else if(errno == EACCES){
		    printf("Permission denied to create directory.\n");
		    return -1;
		 }
		else if(errno == ENOSPC){
			printf("No space left on device.\n");
			return -1;
	    }
	    else{
	       printf("ERROR OCCURRED\n");
	       return -1;
	   }
	}
	return 0;
}

//rmdir()
int myRmDir(const char *dirName){
	if(rmdir(dirName) == -1)
		return -1;
	return 0;
}

//rm()
int delFile(const char *fileName){
	if(unlink(fileName) == -1)
		return -1;
	return 0;
}

int rmRecursive(const char *path){
	struct stat st;

	if(lstat(path, &st) == -1){
		perror("lstat");
		return -1;
	}

	if(S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)){
		if(unlink(path) == -1){
			perror("unlink");
			return -1;
		}
		return 0;
	}

	if(S_ISDIR(st.st_mode)){
		DIR *dir = opendir(path);
		if(!dir){
			perror("opendir");
			return -1;
		}
		
		struct dirent *entry;
		while((entry = readdir(dir)) != NULL){
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			char childPath[4096];
			snprintf(childPath, sizeof(childPath), "%s/%s", path, entry->d_name);

            if(rmRecursive(childPath) == -1){
            	closedir(dir);
            	return -1;
            }			
		}
		closedir(dir);

		if(rmdir(path) == -1){
			perror("rmdir");
			return -1;
		}
		return 0;
	}

	fprintf(stderr, "Unsupported file type: %s\n", path);
	return -1;
}

int main(int argc, char *agrv[]){
    if(argc < 3){
    	write(STDOUT_FILENO, "Usage: fs <op> <path>\n", 21);
    	return 1;
    }
    FileOp op = parseOp(agrv[1]);
    switch(op){
    	case OP_STAT:
    	{
    		if(myStat(agrv[2]) == -1)
    			perror("myStat");
    		break;
    	}
    	case OP_DIR:
    	{
    		if(myMkDir(agrv[2]) == 0){	
    		const char *message = "Directory is successfully created.";
    		size_t mlen =  strlen(message);
    		write(STDOUT_FILENO, message, mlen);
    		}else{
    			perror("myMkDir");
    		}
    		break;
    	}
    	case OP_RMDIR:
    	{
    		if(myRmDir(agrv[2]) == -1){
    			if(errno == ENOTEMPTY)
    				printf("Directory not empty.\n");
    			else
    				perror("myRmDir");
    		}
    		break;
    	}
    	case OP_DELFL:
    	{
    		if(delFile(agrv[2]) == -1){
    			perror("deleteFile");
    		}
    		break;
    	}
    	case OP_DELREC:
    	{
    		if(rmRecursive(agrv[2]) == -1){
    			fprintf(stderr, "Failed to remove %s", agrv[2]);
    			return 1;
    		}
    		break;
    	}
    	default:
   	    	write(STDOUT_FILENO, "Invalid file Operator\n", 22);
    }
	return 0;
}
