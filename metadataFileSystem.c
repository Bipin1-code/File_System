
/*
  This program simulate a block of disk file to preserve the matadata of other blocks
  where we write the content. 
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BLOCKSIZE 512

int create_virtualFile(const char *diskNam, const char *blockNam){
  FILE *disk = fopen(diskNam, "rb+");
  if(!disk){
    perror("ERROR: opening disk failed.\n");
    return 1;
  }
  //this is the content which we wanna write inside the specific block
  // We can also make it user given input like:
  // fgets(data, sizeofmessage, stdin);
  const char *data = "I am writing on file, whose name is File2.";
  //Write metabata to block 1
  fseek(disk, 1 * BLOCKSIZE, SEEK_SET);
  char metadata[BLOCKSIZE] = {0};
  snprintf(metadata, BLOCKSIZE, "%s|%d|%zu", blockNam, 2, strlen(data));
  fwrite(metadata, sizeof(char), BLOCKSIZE, disk);

  //Write file content to block 2
  fseek(disk, 2 * BLOCKSIZE, SEEK_SET);
  char filecontent[BLOCKSIZE] = {0};
  strncpy(filecontent, data, BLOCKSIZE);
  fwrite(filecontent, sizeof(char), BLOCKSIZE, disk);

  fclose(disk);
  
  return 0;
}

// For to read the specific block 
int read_block(const char *diskname, const int block){
  FILE *disk = fopen(diskname, "rb");
  if(!disk){
    perror("ERROR: file opening failed!\n");
    return 1;
  }
  fseek(disk, block * BLOCKSIZE, SEEK_SET);
  char buffer[BLOCKSIZE] = {0};
  fread(buffer, sizeof(char), BLOCKSIZE, disk);
  // printf("The metadataFile (block 1) content is :\n %s \n", buffer);
  printf("%d block content is : \n %s \n", block, buffer);
  fclose(disk);
  return 0;
}

int main(){
  const char *diskfilename = "disk.dat";
  //const char *blockname = "File2";
  // create_virtualFile(diskfilename, blockname); //1. diskfile, 2.block wher we want to perform task
  read_block(diskfilename, 2);  //arugment 2 is the position of block 
  return 0;
}

//Now next step is to make user's can do read and write on file form CLI
