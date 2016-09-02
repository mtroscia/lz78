#include "decompressor.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

int array_reset();

int array_add(uint32_t father_index, char character)
{	
	array_elem_counter++;
	
	if (array_elem_counter == dictionary_size)
		array_reset();
	
	//eventually update the number of bits for the symbols
	if((1<<actual_bits_counter)==array_elem_counter+1) 
			actual_bits_counter++;
	
	//add the element
	
	dictionary[array_elem_counter].father_index =father_index;
	dictionary[array_elem_counter].character =character;
	
	printf ("Added child:%i - father:%i - character:%c\n", array_elem_counter,
		     dictionary[array_elem_counter].father_index, dictionary[array_elem_counter].character);
	
	/************************************ONLY FOR TESTING PURPOSES******************************************************/
		//print_hash_table();
	/******************************************************************************************************************/
	
	return 0;
}


int array_init(){
	int i, ret;
	char c;
	
	//initially we have 257 element
	actual_bits_counter=9;
	
	
	//add all ASCII characters
	for (i=0; i<=255; i++)
	{
		c=i;
		printf("Added %c\n", c);
		
		ret = array_add(0, c);
		if (ret!=0){
			printf("Error in array_add\n");
			return -1;
		}
		
		printf ("Actual array_elem_counter = %i\n", array_elem_counter);
		
	}
	
	//printf("Content of array_table[66]:\tfather_index %i - character %c - child_index %i\n",
	//		dictionary[66].father_index, hash_table[66].character,hash_table[66].child_index);
	
	return 0;
}


int array_reset(){
	int ret;
	
	//reset all fields
	bzero(dictionary, dictionary_size*sizeof (struct array_elem));
	
	//add all root's children
	ret = array_init();	
	if (ret!=0){
			printf("Error when reinitializing the hash table");
			return -1;
	}
	return 0;
}


//initialize decompression
int array_create()
{	
	int ret;
	
	decomp_buffer=calloc (8, sizeof (char)); 
	
	/**************************************change 10000 with the dict_size value included in the header****/
	dictionary_size=10000;
	/**************************************************************************************************/
	dictionary=calloc(dictionary_size, sizeof (struct array_elem)); //10000 =dict size 
	if (dictionary ==NULL){
		printf("unable to create a valid tree for the decompression phase.\n");
		return -1;
	}
	
	ret = array_init();
	if (ret < 0)
	{
		printf ("Error in array_init");
	}
	return 0;
	
}