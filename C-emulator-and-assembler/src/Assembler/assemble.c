#include <stdlib.h>
#include <stdio.h>
#include "hashTable.h"
#include "filehandling.h"

// Create a reference for the symbol table.
HashTable* symbolTable;

/**
  The main function of the program.
  @param argc The number of command-line arguments.
  @param argv An array of strings representing the command-line arguments.
  The first argument should be the assembly file to process,
  and the second argument should be the output file to generate.
  @return The exit status of the program.
*/
int main(int argc, char** argv) {
  // Check if the correct number of command-line arguments is provided
  if (argc != 3) {
    fprintf(stderr, "Wrong number of command-line arguments\n");
    exit(EXIT_FAILURE);
  }

  // Create a hash table and assign the symbolTable pointer.
  symbolTable = create_hash_table(1);

  // Process the assembly file and generate the output file
  process_assembly_file(argv[1], argv[2]);

  // Free the memory used by the symbol table
  free_hash_table(symbolTable);

  // Return the exit status indicating successful execution
  return EXIT_SUCCESS;
}