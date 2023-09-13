#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "filehandling.h"
#include "assembler_utilities.h"

int currentAddress = 0;

// Append the instruction to the binary file
void write_binary_to_file(int32_t value, const char* filename) {
  FILE* file = fopen(filename, "ab"); // Open in append binary mode
  if (file == NULL) {
    perror("Error opening file");
    return;
  }
  // Write the binary to the file
  fwrite(&value, sizeof(int32_t), 1, file);

  fclose(file);
}

// Takes in a string corresponding to a line of assembly and splits it up into words
void tokenize(char* line, char** tokens) {
  // Tokenize the line and store the tokens in the tokens array
  int numTokens = 0;
  char* token = strtok(line, DELIMITERS);
  while (token != NULL) {
    if (numTokens >= MAX_TOKENS_PER_LINE) {
      break; // Reached the maximum tokens in a given line
    }
    strncpy(tokens[numTokens], token, MAX_TOKEN_LENGTH - 1);
    numTokens++;
    token = strtok(NULL, DELIMITERS);
  }
  //printf("num of tokens: %d\n",numTokens);
  // for (int j = 0; j < numTokens; j++) {
  //   printf("\"%s\"", tokens[j]);
  //   if (j < numTokens - 1) {
  //     printf(", ");
  //   }
  // }
}

int find_num_tokens(char** tokens) {
  int numTokens = 0;
  for (int i = 0; i < MAX_TOKENS_PER_LINE; i++) {
    // printf("current string: %c\n",tokens[i][0]);
    if (tokens[i][0] == '\0') {
      //if we reach empty token, the rest must also be empty
      break;
    }
    numTokens++;
  }
  return numTokens;
}

// Opens file and tokenizes it line by line. Then starts the first and the second pass
void process_assembly_file(const char* inputFilename, const char* outputFilename) {

  //Clear Output file.
  fclose(fopen(outputFilename, "w"));

  FILE* file = fopen(inputFilename, "r");
  if (file == NULL) {
    perror("Error opening file");
    return;
  }
  char*** tokenisedLines = safe_malloc(MAX_LINES * sizeof(char**));
  for (int i = 0; i < MAX_LINES; i++) {
    tokenisedLines[i] = safe_malloc(MAX_TOKENS_PER_LINE * sizeof(char*));
    for (int j = 0; j < MAX_TOKENS_PER_LINE; j++) {
      tokenisedLines[i][j] = safe_malloc(MAX_TOKEN_LENGTH * sizeof(char));
    }
  }

  int numLines = 0;
  char line[MAX_TOKENS_PER_LINE * MAX_TOKEN_LENGTH];

  // This is the first pass of the assembler, take each line and check if it is a label,
  // if not add it to the tokenized lines array so it can be processed in the second pass
  while (fgets(line, sizeof(line), file) != NULL) {
    // Remove the newline char
    line[strcspn(line, "\n")] = '\0';

    // Skip empty lines
    if (line[0] == '\0') {
      continue;
    }
    // Handle labels separately
    char* findLabel = strchr(line, ':');
    if (findLabel != NULL) {
      *findLabel = '\0'; // Remove the colon
      int jumps = (currentAddress);
      // Pass this label into the instruction parser and don't add to tokenized lines.
      insert_into_hash_table(symbolTable, line, &jumps, sizeof(int*));
      continue;
    }

    // We now know this line is neither a label nor a comment
    tokenize(line, tokenisedLines[numLines]);

    numLines++;
    currentAddress = numLines * sizeof(int32_t);
    if (numLines >= MAX_LINES) {
      break; // Reached the maximum number of lines
    }
  }

  fclose(file);

  // This is the second pass of the assembler, for now we just print the tokenized lines
  for (int i = 0; i < numLines; i++) {
    int count = find_num_tokens(tokenisedLines[i]);
    if (count == 0) continue;
    currentAddress = i * sizeof(int32_t); //set current address based on what instruction we are on
    int instruction = instruction_processor(find_num_tokens(tokenisedLines[i]), tokenisedLines[i]);
    write_binary_to_file(instruction, outputFilename);
  }


  // Free the tokenised lines.
  for (int i = 0; i < MAX_LINES; i++) {
    for (int j = 0; j < MAX_TOKENS_PER_LINE; j++) {
      free(tokenisedLines[i][j]);
    }
    free(tokenisedLines[i]);
  }
  free(tokenisedLines);
}
