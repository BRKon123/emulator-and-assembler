#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "assembler_utilities.h"


#define REHASH_FACTOR 0.7

// For the implementaiton of the symbol table we will be using a hash table.
// This us to reduce the time taken to search the table.

// How To Use
/*
  HashTable *hashTable = create_hash_table(int size);
        Creates Table
        Note that the table is dynamic so it does not matter if the number of elements
        exceed the size estimation
  int a = 1;
  int b = 2;
  int c = 3;
  insert_into_hash_table(hashTable, "key ", &a, sizeof(int));
  insert_into_hash_table(hashTable, "key2", &b, sizeof(int));
  insert_into_hash_table(hashTable, "key3", &c, sizeof(int));
  int* value1 = search_hash_table(hashTable, "key ");
  int* value2 = search_hash_table(hashTable, "key2");
  int* value3 = search_hash_table(hashTable, "key3");
  int* value4 = search_hash_table(hashTable, "key4");
  printf("%d\n", *value1);
  printf("%d\n", *value2);
  printf("%d\n", *value3);
  printf("%p\n",  value4); <- Should return nil pointer, note we are printing the value.
  CHECK THAT THE POINTER IS NOT NIL BEFORE USE!!! (even though i didnt show it here)
*/

extern void rehash_hash_table();

// Node is a struct containing the key and value of elements in the hash table.
// As separate chaining is used, an value for the next node is also included.
typedef struct Node {
  char* key;
  void* value;
  size_t size;
  struct Node* next;
} Node;


// The hash table will be dynamically sized and so holds values for both the size of the hash 
// table and number of items (count.)
// "Items" is stored as a pointer to array again because of separate chaining. 
typedef struct HashTable {
  int size;
  int count;
  Node** items;
} HashTable;


/**
  Creates a new node with the given key and value.
  @param key the key of the node, represented as a null - terminated string
  @param value the value of the node, represented as an integer
  @return a pointer to the newly created node
**/
Node* create_node(char* key, void* value, size_t value_size) {
  Node* node = (Node*)safe_malloc(sizeof(Node));
  node->key = strdup(key);

  node->size = value_size;

  // Allocate memory for the value and copy the contents
  node->value = safe_malloc(value_size);
  memcpy(node->value, value, value_size);

  node->next = NULL;
  return node;
}

/**
  Creates a new hash table with the given size.
  @param size the size of the hash table
  @return a pointer to the newly created hash table
**/
HashTable* create_hash_table(int size) {

  // size cannot be < 1 other wise a SIGFPE will occur when calculating the hash.
  assert(size > 0);

  HashTable* table = (HashTable*)safe_malloc(sizeof(HashTable));
  table->size = size;
  table->count = 0;
  table->items = (Node**)calloc(table->size, sizeof(Node*));
  ALLOC_EXCEPTION(table->items);
  return table;
}

/**
  Computes a hash value for the given key using the djb2 algorithm.
  @param key the key to hash, represented as a null-terminated string
  @return the hash value of the key
**/
unsigned long hash(const char* key) {
  unsigned long hash = 1234;
  int x;
  while ((x = *key++)) {
    hash = ((hash << 5) + hash) + x;
  }
  return hash;
}

/**
  Inserts a new entry into the hash table.
  @param table The hash table to insert into.
  @param key The key associated with the entry.
**/
void insert_into_hash_table(HashTable* table, char* key, void* value, size_t value_size) {
  int index = hash(key) % table->size;
  Node* node = table->items[index];

  if (node == NULL) {
    table->items[index] = create_node(key, value, value_size);
    table->count++;
  }
  else {
    Node* prev;
    while (node != NULL) {
      if (strcmp(node->key, key) == 0) {
        node->value = safe_malloc(value_size);
        memcpy(node->value, value, value_size);
        return;
      }
      prev = node;
      node = node->next;
    }
    prev->next = create_node(key, value, value_size);
    table->count++;
  }

  if (table->count / (float)table->size > REHASH_FACTOR) {
    rehash_hash_table(table, table->size * 2);
  }
}

/**
  Searches for an entry with the specified key in the hash table.
  @param table The hash table to search in.
  @param key The key to search for.
  @return The value associated with the key if found, or -1 if not found.
  check if pointer is nil before using!!!
**/
void* search_hash_table(HashTable* table, char* key) {
  int index = hash(key) % table->size;
  Node* node = table->items[index];

  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      return node->value;
    }
    node = node->next;
  }
  return 0;
}

/**
  Rehashes the hash table with a new size, creating a new table and transferring entries.
  @param table The hash table to rehash.
  @param newSize The new size of the hash table.
**/
void rehash_hash_table(HashTable* table, int newSize) {
  HashTable* newTable = create_hash_table(newSize);

  for (int i = 0; i < table->size; i++) {
    Node* node = table->items[i];
    while (node != NULL) {
      insert_into_hash_table(newTable, node->key, node->value, node->size);
      node = node->next;
    }
  }


  free(table->items);
  *table = *newTable;
}

/**
  Frees the memory occupied by the hash table and its associated nodes.
  @param table The hash table to free.
**/
void free_hash_table(HashTable* table) {
  for (int i = 0; i < table->size; i++) {
    Node* node = table->items[i];
    while (node != NULL) {
      Node* temp = node;
      node = node->next;
      free(temp->key);
      free(temp->value);
      free(temp);
    }
  }
  free(table->items);
  free(table);
}