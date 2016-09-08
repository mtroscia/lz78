//functions to build the array used by the decompressor and functions to decompress

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bitio.h"

struct array_elem{
	uint32_t father_index;
	uint8_t character;
};

struct array_elem* dictionary;
int dictionary_size; 	//max number of element in the array
int array_elem_counter;	//number of element in the hash table
int actual_bits_counter;//number of bits for the current symbol
int unknown_node;
char* decomp_buffer;
struct bitio* my_bitio_d;

//initialize all the stuff
int init_decomp(int dict_size);

//perform the decompression phase
int decompress(char* input_file_name, int dict_size);