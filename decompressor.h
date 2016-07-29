/*it contains function to build the array used by the decompressor
and functions to decompress*/

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

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
	int father_num;
	uint8_t c;
};

struct array_elem* decomp_tree;