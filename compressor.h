/*it contains function to build the hash table used by the compressor
and to compress the file*/

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

// hash table whose key is <father label, symbol> and whose value is <label>

/*	-) initilize hash table (first level children)
	-) pointer on the first level children whose label is the first read symbol
	while (new read symbol!=EOF) {
		if (the pointed node doesn't have a child whose label is the read symbol) {
			-) emit label of the node
			-) add a new node as child with the newly read symbol into the hash table
			-) move the pointer to the first level child whose label is the read symbol
		}
	}
	-) emit the label of the pointed node
	-) emit 0 (EOF)
	
	-) add header
*/

struct hash_elem{
	unsigned long key;
	uint32_t child_index;
	int filled; //this number is a flag that signal if the elem is occupied
 	struct hash_elem* next; 
};

struct hash_elem* hash_table;
int hash_table_size;



unsigned long hash(unsigned char*);

//create table of dimension dict_size
int hash_table_create(uint64_t size);

//add an element into the hash table
int hash_add(uint32_t father, char symbol, uint32_t child);

//initialize the hash with all the first level children
int hash_init();

//look for an element into the hash table

//reset the hash table



