#include "decompressor.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
int init_decomp(char* source_file_name)
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
	
	my_bitio=bit_open(source_file_name, 0);
	if (my_bitio==NULL){
		printf ("Error in bit_open()\n");
		free (dictionary);
		return -1;
	}
	
	return 0;
}


int extend_buffer()
{
	int i;
	char *new_buffer;

	new_buffer= calloc((sizeof decomp_buffer - 1) * 2,sizeof (char));
	if (new_buffer == NULL)
	{
		printf("Unable to allocate enough memory\n");
		return -1;
	}
	
	//compy the element from the old buffer to the new one
	for (i=0; i < strlen(decomp_buffer); i++)
	{
		new_buffer[i] = decomp_buffer[i];
	}
	
	decomp_buffer = new_buffer;
	return 0;
}

int emit_symbols(FILE* f)
{
	char  temp;
	int i, j = 0;
	
	//we need to reverse the order of the characters contained into the buffer
	i = 0;
	j = strlen(decomp_buffer) - 1;
 
	while (i < j) {
		temp = decomp_buffer[i];
		decomp_buffer[i] = decomp_buffer[j];
		decomp_buffer[j] = temp;
		i++;
		j--;
	}
 
	printf ("string : %s\n", decomp_buffer);
	
	//for (i=0; i<hash_table_size; i++)
	{
		fprintf(f, "%s",decomp_buffer);
	}
	return 0;
}

int find_path(uint32_t child_index, int unknown_node, FILE* f)
{
	int i, ret;
	char character;
	
	i=0;
	
	while (1)
	{	
		//read all the array's info
		character = dictionary[child_index].character;
		child_index = dictionary[child_index].father_index;
		
		decomp_buffer[i]=character;
                
		if (child_index == 0)
			//we have reached the root of the tree
			break;
		
		if (i == (sizeof decomp_buffer -1))
		{
			// the decomp_buffer is too small. We need a bigger one
			ret=extend_buffer();
			
			if(ret < 0)
			{
				printf ("unable to extend the decompression buffer\n");
				return -1;
			}
		} 
		else
			i++;
	}
	 //at the first step we don't have any node with unknown character value
        if (unknown_node != 0)
	{
            printf("update the unknown node%i: %c\n",array_elem_counter-1,character);
            // there is a child node with an unknown character (it is the last one!)
            dictionary[array_elem_counter-1].character = character;
	}
	decomp_buffer[++i] = '\0';
	
	ret = emit_symbols(f);
	
	return 0;
}

int decode(FILE* f)
{
	uint64_t node_index;
	int ret, unknown_node;
	
	unknown_node=0;
	
	printf ("\n\nIn decode\n");
	while (1)
	{
		ret=bit_read(my_bitio, actual_bits_counter, &node_index);
		if (ret<0)
		{
			printf ("Error in read bit");
			return -1;
		}
		
		//we have reached the EOF symbol
		if (node_index == 0)
		{
			//end of file reached
			// <finalize>
			break;
		}
		
		//add an unknown node
		ret = array_add (node_index, '?');
		
                //brows the tree
		ret = find_path(node_index, unknown_node, f);
		
		unknown_node=1;
	} 
	return 0;
}


int extract(){
	
	
	
	return 0;
}


int decompress(char* dest_file_name)
{
	//open dest with fopen
	FILE *f;
	
	f = fopen(dest_file_name, "w");
	if (f == NULL)
	{
		printf("Unable to create hash_table_file");
		return -1;
	}
	
	decode(f);
	//<check the header>
	
	//leggere 2 byte compression+total length
	//bit_read (length-1-sizeof (int))*8      #of bit
	
	
	//<if (compressed)>
	//		decode(bitr)
	//else
	// 		extract from bitr

	fclose(f);
	return 0;
}