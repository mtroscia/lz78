/*it contains function to build the array used by the decompressor
and functions to decompress*/

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "bitio.h"

struct array_elem{
	uint32_t father_index;
	uint8_t character;
};

struct bitio* my_bitio_d;
struct array_elem* dictionary;
int dictionary_size; 	//max number of element in the array
int array_elem_counter;	//number of element in the hash table
int actual_bits_counter;//number of bits for the current symbol
char* decomp_buffer;
unsigned long hash(unsigned char*);


//initialize all the stuff
int init_decomp();

//perform the decompression phase
int decompress(char* input_file_name);