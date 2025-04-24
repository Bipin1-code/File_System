
//We are making a virtual disk for to simulated the Disk.

#include <stdio.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 10

int main(){
  const char *disk_filename = "disk.dat";
  FILE *disk;
  disk = fopen(disk_filename, "wb");
  if(!disk){
    perror("Failed to create virtual disk\n");
    return 1;
  }
  char block[BLOCK_SIZE] = {0};

  for(size_t i = 0; i < NUM_BLOCKS; i++){
    fwrite(block, sizeof(char), BLOCK_SIZE, disk);
  }
  printf("Virtual disk '%s' created with %d blocks of %d bytes each.\n", disk_filename, NUM_BLOCKS,BLOCK_SIZE);

  fclose(disk);
  
  return 0;
}
