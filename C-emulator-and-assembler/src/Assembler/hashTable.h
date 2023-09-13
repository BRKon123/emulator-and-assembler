#ifndef ST_H
#define ST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler_utilities.h"

typedef struct Node {
  char* key;
  void* value;
  struct Node* next;
} Node;

typedef struct HashTable {
  int size;
  int count;
  Node** items;
} HashTable;

Node* create_node(char* key, void* value, size_t value_size);
HashTable* create_hash_table(int size);
unsigned long hash(const char* key);
void insert_into_hash_table(HashTable* table, char* key, void* value, size_t value_size);
void* search_hash_table(HashTable* table, char* key);
void rehash_hash_table(HashTable* table, int newSize);
void free_hash_table(HashTable* table);

#endif
