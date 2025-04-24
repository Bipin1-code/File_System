
//We created the virtual disk and did equal partition of it.
//Now, in this program we need to do perform read and write on specific block of the disk.

#include <stdio.h>

#define BLOCK_SIZE 512

int write_block(const char *diskname, size_t blocksIdx){
  FILE *disk;
  disk = fopen(diskname, "rb+");
  if(!disk){
    perror("Error opening disk");
    return 1;
  }
  fseek(disk, blocksIdx * BLOCK_SIZE, SEEK_SET);
  fwrite("Here is the message!", sizeof(char), BLOCK_SIZE, disk);

  fclose(disk);
  return 0;
}

int read_block(const char * diskname, size_t blocksIdx){
  FILE *disk;
  disk = fopen(diskname, "rb");
  if(!disk){
    perror("ERROR: opening disk");
    return 1;
  }
  fseek(disk, blocksIdx * BLOCK_SIZE, SEEK_SET);
  char buffer[BLOCK_SIZE]= {0};
  fread(buffer, sizeof(char), BLOCK_SIZE, disk);
  printf("Data in block 2: %s\n", buffer);
  fclose(disk);
  return 0;
}

int main(){
  const char *diskfilename = "disk.dat";
  // write_block(diskfilename, 2);
  //printf("Write_block finished its takes on block 2.\n");
  read_block(diskfilename, 2);
  printf("Read_block finished its takes on block 2.\n");
 
  return 0;
}
