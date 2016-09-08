//functions to build the hash table used by the compressor and to compress the file

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bitio.h"


struct hash_elem{
	uint32_t father_index;
	char character;
	uint32_t child_index; //new node
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





