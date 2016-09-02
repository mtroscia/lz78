/*it contains function to build the array used by the decompressor
and functions to decompress*/

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "bitio.h"

/*array whose entries contain the symbols
  it is of type <label, symbol> where label=index+1 as the root (EOF) has not to be stored
*/
 

/*	-) check header

	-) initilize array (first level children)
	//first time
	-) read a new character c
	-) add to node c a new child; remember its entry into last_entry as the symbol is updated later
	-) emit the symbol corresponding to the read character (remember index=character-1)
	while (new read character!=0) {
		-) save the path from the actual node (that is character-1) to the root
		-) update last_entry with the symbol of the actual node 
		-) emit symbols of the path in the inverse order in which they have been saved
		-) add a child to the actual node, remembering its entry into last_entry to update it later
	}
	-) emit EOF
*/

struct array_elem{
	uint32_t father_index;
	uint8_t character;
};

struct array_elem* dictionary;
int dictionary_size; 	//max number of element in the array
int array_elem_counter;	//number of element in the hash table
int actual_bits_counter;//number of bits for the current symbol
char* decomp_buffer;
struct bitio* my_bitio;

//???int hash_elem_pointer;	//pointer to the actual node

unsigned long hash(unsigned char*);

//initialize all the stuff
int init_decomp(char*);

//decode the file
int decompress(char* input_file_name);

