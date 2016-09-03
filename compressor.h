/*it contains function to build the hash table used by the compressor
and to compress the file*/

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "bitio.h"

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
	//unsigned long key; //father-character
	uint32_t father_index;
	char character;
	uint32_t child_index; //new node
//	int filled; //this number is a flag that signal if the elem is occupied
// 	struct hash_elem* next; 
};

struct hash_elem* hash_table;
int hash_table_size; 	//max number of element in the hash table
int dictionary_size; 	//max number of element in the dictionary
int hash_elem_counter;	//number of element in the hash table
int hash_elem_pointer;	//pointer to the actual node
int actual_bits_counter;//number of bits for the current symbol
unsigned long hash(unsigned char*);

struct bitio* my_bitio_c;

//create table of dimension dict_size
int hash_table_create(uint64_t size);

//compress algorithm
int compress(char* input_file_name);

//add an element into the hash table
//int hash_add(uint32_t father, char symbol, uint32_t child);

//initialize the hash with all the first level children
//int hash_init();

//look for an element into the hash table
//uint32_t hash_search(uint32_t father, char symbol);

//reset the hash table
//int reset();





