
//Regex
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

typedef enum{
  RGX_FileR = 0,
  RGX_ReadFL,
  RGX_Invalid
} RegexFunc;

size_t strLen(const char *s){
  size_t len = 0;
  for(size_t i = 0 ; s[i] != '\0'; i++)
    len++;
  
  return len;
}

int strCmp(const char *s, const char *p){
  while(*s && *p){
    if(*s != *p)
      return -1;

    s++;
    p++;
  }

  return (*s == '\0' && *p == '\0') ? 0 : -1;
}

int isPunctuation(char c){
  char *p = "'\";:\\/`~+=_-%$!?.,";
  while(*p){
    if(c == *p)
      return 1;
    p++;
  }
  return 0;
}

RegexFunc parseRXF(const char *s){
  //Our First string manipulation begin form here
  if(strCmp(s, "fileApplyRegex") == 0) return RGX_FileR;
  if(strCmp(s, "readLine") == 0) return RGX_ReadFL;
  return RGX_Invalid;
}

typedef struct {
  int nChar;
  int capacity;
  char *chars; 
} Line;

//User provide file path and pattern
//We need to check that pattern on file, just that's it
//return 0 on Success and -1 on Failure
int fileApplyRegex(const char *path, const char *pattern){
  int fd = open(path, O_RDONLY);
  if(fd == -1){
    perror("open");
    return -1;
  }

  char buffer[4096];
  ssize_t n;
    
  Line line = {
    .nChar = 0,
    .capacity = 1024,
    .chars = malloc(1024)
  };
  
  if(!line.chars){
    close(fd);
    return -1;
  }

  int matched = 0;
  int j = 0;
  char word[64];
  int lineCount = 0;
  
  while((n = read(fd, buffer, sizeof(buffer))) != 0){
    if(n == -1){
      perror("read");
      return -1;
    }
   
    for(int i = 0; i < n; i++){
      char c =  buffer[i];

      //build full line
      if(line.nChar + 1 >= line.capacity){
	line.capacity *= 2;
	char *temp = realloc(line.chars, line.capacity);
	if(!temp){
	  free(line.chars);
	  close(fd);
	  return -1;
	}
	line.chars = temp;
      }
      line.chars[line.nChar++] = c;

      //build word
      if(c != ' ' && c != '\n' && !isPunctuation(c)){
	if(j < (int)sizeof(word) - 1)
	  word[j++] = c;
      }else{
	//space occurr word finished
	word[j] = '\0';
	if(j > 0 && strCmp(word, pattern) == 0)
	  matched = 1;
	
	j = 0;

	//line ends
	if(c == '\n'){
	  lineCount++;
	  if(matched){
	    line.chars[line.nChar - 1] = '\0';
	    printf("[%d] %s\n", lineCount, line.chars);
	  }
	  line.nChar = 0;
	  matched = 0;	
	}
      }
    }
  }
  //EOF flush (last line without '\n')
  if(line.nChar > 0){
    word[j] = '\0';
    if(j > 0 && strCmp(word, pattern) == 0)
      matched = 1;

    if(matched){
      line.chars[line.nChar] = '\0';
      printf("[%d] %s\n", ++lineCount, line.chars);
    }
  }

  free(line.chars);
  close(fd);

  return 0;
}

char* readLine(int fd){
  static char buffer[4096];
  static ssize_t bufLen = 0;
  static ssize_t bufPos = 0;

  Line line = {
    .nChar = 0,
    .capacity = 1024,
    .chars = malloc(1024)
  };

  if(!line.chars){
    return NULL;
  }

  while(1){
    //state A: Need More Data
    if(bufPos >= bufLen){
      bufLen = read(fd, buffer, sizeof(buffer));
      bufPos = 0;

      if(bufLen <= 0) break;
    }

    //state B: Consume Data
    char c = buffer[bufPos++];
    if(line.nChar + 1 >= line.capacity){
      line.capacity *= 2;
      char *temp = realloc(line.chars, line.capacity);
      if(!temp){
	free(line.chars);
	return NULL;
      }
      line.chars = temp;
    }
    line.chars[line.nChar++] = c;

    //state C: Line complete because c encounter new line character
    if(c == '\n') break;
  }

  //state D: EOF means read() = 0; red all bytes from a given file.
  if(line.nChar == 0){
    free(line.chars);
    return NULL;
  }

  line.chars[line.nChar] = '\0';
  return line.chars;
}

int main(int argc, char **argv){
  //argv[1] is functions name [feature program offer]
  //argv[2] is pattern

  //first file:
  //idea when needs to provide three things:
  //argv[1] = fileApplyRegex(path or fileName, pattern)
  //this means: argv[2] = filename in this case, argv[3] = pattern
  if(argc < 3){
    fprintf(stderr, "Usage <%s> <func> <file> <Optional pattern>\n", argv[0]);
    return 1;
  }

  RegexFunc rxF = parseRXF(argv[1]);
  switch(rxF){
     case RGX_FileR:{
       if(argc < 4){
	 fprintf(stderr, "%s expected <fileName> <pattern>", agrv[1]);
	 return 1;
       }
       if((fileApplyRegex(argv[2], argv[3])) == -1){
	 perror("fileApplyRegex");
	 return 1;
       }
       break;
     }
     case RGX_ReadFL:{
       //this is non-sense but I put it
       if(argc < 3){
	 fprintf(stderr, "%s expected <fileName or path>", agrv[1]);
	 return 1;
       }
       int fd = open(argv[2], O_RDONLY);
       if(fd == -1){
	 perror("open");
	 return 1;
       }
       char *line = readLine(fd);
       printf("%s\n",line); //user can read this result and use it.
       free(line);
       close(fd);
       break;
     }
     default:{
       printf("Invalid file operator.\n");
    }
  }
  
  return 0;
}
