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
int init_decomp(char* dest_file_name)
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
	
	my_bitio=bit_open(dest_file_name, 1);
	if (my_bitio==NULL){
		printf ("Error in bit_open()\n");
		free (dictionary);
		return -1;
	}
	
}

int decode(struct bitio* bitr)
{
	
	
	return 0;
}


int extract(struct bitio* bitr){
	
	
	
	return 0;
}


int decompress(char* input_file_name)
{
	struct bitio* bitr;
	
	bitr=bit_open(input_file_name, 0);
	if (bitr==NULL){
		printf ("Error in bit_open()\n");
		free (dictionary);
		return -1;
	}
	
	//<check the header>
	
	//leggere 2 byte compression+total length
	//bit_read (length-1-sizeof (int))*8      #of bit
	
	
	//<if (compressed)>
	//		decode(bitr)
	//else
	// 		extract from bitr

	return 0;
}



