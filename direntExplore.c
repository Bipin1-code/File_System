//Exploring dirent.h 

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

int main(int agrc, char *agrv[]){

    if(agrc < 2){
    	fprintf(stderr, "Usage: %s <directory>\n", agrv[0]);
    	return 1;
    }

    struct stat st;
	if((lstat(agrv[1], &st)) == -1){
		perror("lstat");
		return 1;
	}

    if(S_ISDIR(st.st_mode)){
		DIR *dir = opendir(agrv[1]);
		if(!dir){
			perror("opendir");
			return 1;
		}

		struct dirent *entry;
		printf("  inode   | Access | User | Group | link |  size |  name \n");
		while((entry = readdir(dir)) != NULL){
			if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' ||
			        (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
			        continue;
			
			char path[4096];
			snprintf(path, sizeof(path), "%s/%s", agrv[1], entry->d_name);

			struct stat est;
			if(lstat(path, &est) == -1){
				perror("lstat");
				return 1;
			}
			struct passwd *pw = getpwuid(est.st_uid);
			struct group *gr = getgrgid(est.st_gid);
			
			printf("%8lu %8o %8s %8s %5d %5zu %4s \n",
		 	(unsigned long)est.st_ino,
		 	est.st_mode & 0777,
		 	pw->pw_name,
		 	gr->gr_name,
		 	(int)est.st_nlink,
		 	(size_t)est.st_size,
		 	entry->d_name);
		}	
		closedir(dir);
	}
	else{
	    fprintf(stderr, "%s is not a directory\n", agrv[1]);
	    return 1;
	}
	
	return 0;
}
