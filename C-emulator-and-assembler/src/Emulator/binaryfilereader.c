#include <stdio.h>
#include <stdlib.h>
#include "data.h"

//validates the input file and opens it
static FILE *openFile(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    perror("Error opening file");
  }
  return file;
}

//reads 32 bits from input file at a time and writes it in memory
static void writeInstructionsToMemory(const char *filename) {
  FILE *file = openFile(filename);
  fread(memory, sizeof(int32_t), MEMORY_SIZE, file);
  fclose(file);
}

//loads the input file into memory
void loadFileIntoMemory(int argc, char const **argv) {
  if (argc != 3) {
    fprintf(stderr, "Wrong number of command-line arguments\n");
    exit(0);
  }
  //gets pointer to the input file
  const char *file = argv[1]; 
  writeInstructionsToMemory(file);
}
