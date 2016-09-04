#include "compressor.h"
#include <string.h>

int hash_add(uint32_t, char, uint32_t);
int hash_init();
int hash_reset();
unsigned long generate_hash_index(uint32_t, char);
/****************************************************************************************************/
void print_hash_table();
/******************************************************************************************************/

unsigned long hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		return hash;
}


int hash_table_create(uint64_t size){
	int ret;
	
	dictionary_size = size;
	
	//hash_table_size = size+(size/2);
	//let's try a bigger expansion factor
	hash_table_size = size*2;
	
	//allocate memory for the hash table
	hash_table = (struct hash_elem*)calloc(hash_table_size, sizeof(struct hash_elem)); // it should be better
	
	if(hash_table==NULL){
		printf("Error: hash table has not been created.\n");
		return -1;
	}
	
	ret = hash_init();
	if (ret!=0){
		printf("Error: hash table has not been initialized.\n");
		free(hash_table);
		return -1;
	}
	
	return 0;
}


int hash_init(){
	int i, ret;
	char c;
	
	actual_bits_counter=9;
	
	//add all ASCII characters
	for (i=0; i<=255; i++)
	{
		c=i;
		//printf("Added %c\n", c);
		
		ret = hash_add(0, c, ++hash_elem_counter);
		if (ret!=0){
			printf("Error in hash_add\n");
			return -1;
		}
	}
	
	return 0;
}


int hash_add (uint32_t father, char symbol, uint32_t child){
	unsigned long index;
	int collision;
	
	if (child > dictionary_size)
		hash_reset();
	
	//eventually update the number of bits for the symbols
	if((1<<actual_bits_counter)==hash_elem_counter+1) 
			actual_bits_counter++;
	
	//obtain the index into the hash table
	index = generate_hash_index(father, symbol);
	
	//add the element
	collision = 1;
	while (collision)
	{
		if (hash_table[index].child_index == 0) 
		{
			//There isn't a valid value in this entry (empty entry): insert the element
			hash_table[index].father_index = father;
			hash_table[index].character = symbol;
			hash_table[index].child_index = hash_elem_counter;
			collision = 0;
		}
		else
		{
			printf("Collision for index %lu! -> Check the next entry\n", index);
			//Hash table treated as a circular array
			index = (index+1)%hash_table_size;
		}
	}
	
	/************************************ONLY FOR TESTING PURPOSES******************************************************/
		print_hash_table();
	/******************************************************************************************************************/
	
	return 0;
}


unsigned long generate_hash_index (uint32_t father, char symbol){
	unsigned char buffer[33];
	char s[33];
	
	unsigned long index;
	
	//generate the string using symbol and father_index
	sprintf((char*)buffer, "%c", symbol);
	sprintf((char*)s, "%d", father);
	strcat ((char*)buffer, s);
	
	
	/*sprintf((char*)buffer, "%d", father);
	sprintf(s, "%c", symbol);
	strcat ((char*)buffer, s);*/
	
	//compute hash for the string
	index = hash(buffer);
	//obtain an index from the hash
	index %= hash_table_size;
	
	return index;
}


/*************************************************************************
The function searches a node in the hash table using char-father_index as a key.
If it finds the node, then it returns the index; otherwise it returns 0.
**************************************************************************/
uint32_t hash_search (uint32_t father, char symbol){
	unsigned long index;
	int not_found;
	
	index = generate_hash_index(father, symbol);
	
	not_found = 1;
	do
	{
		if (hash_table[index].father_index==father && hash_table[index].character==symbol)
		{
			//The wanted entry was found
			not_found = 0;
		}else
		{
			if (hash_table[index].child_index==0)
			{
				//Empty entry
				return 0;
			} else  //Search into the next entry
				index = (index+1)%hash_table_size;
		}		
	}while (not_found);
	
	return hash_table[index].child_index;
}

int hash_reset(){
	int ret;
	
	//reset all fields
	bzero(hash_table, hash_table_size*sizeof (struct hash_elem));
	
	//add all root's children
	ret = hash_init();	
	if (ret!=0){
			printf("Error when reinitializing the hash table");
			return -1;
	}
	return 0;	
}

/************************************ONLY FOR TESTING PURPOSES******************************************************/
void print_hash_table(){
	FILE *f;
	int i;
	
	f = fopen("hash_table_file.txt", "w");
	if (f == NULL)
	{
		printf("Unable to create hash_table_file");
		return;
	}
	
	for (i=0; i<hash_table_size; i++)
	{
		fprintf(f, "%i)father_index: %i\tcharacter: %c\tchild_index: %i\n", i,
				hash_table[i].father_index, hash_table[i].character, hash_table[i].child_index);
	}
	
	fclose(f);
}	
/******************************************************************************************************************/

int emit(uint64_t symbol)
{
	int ret;
	
	ret = bit_write(my_bitio_c, actual_bits_counter, symbol);
	if (ret < 0)
	{
		printf("Unable to perform the bit_write\n");
		return -1;
	}		
	
	return 0;
}

int compress (char* input_file_name)
{
	int ret, c;
	FILE* input_file;
	
	printf("\nIn compress\n");
	
	input_file = fopen(input_file_name, "r");
	
	//read a character and perform the compression until EOF is found
	while(1){
		c = getc(input_file);
		
		//printf("Character read: %c\n", c);
		
		again:	if (c==EOF)
				{
					//printf("EOF reached\n");
					
					//emit the last father_index
					ret = emit((uint64_t)hash_elem_pointer);
					if (ret<0)
					{
						printf("Unable to emit the EOF\n");
						free(hash_table);
						return -1;
					}
					//printf("<%i>\n", hash_elem_pointer);
					
					//emit the EOF value
					ret = emit((uint64_t)0);
					if (ret<0)
					{
						printf("Unable to emit the EOF\n");
						free(hash_table);
						return -1;
					}					
					
					//flush my_bitio_c stream
					ret = bit_flush(my_bitio_c);
					if (ret<0)
					{
						printf("Unable to flush the bitio stream\n");
						free(hash_table);
						return -1;
					}
					break;
				}
			
				//search the character into the hash table
				ret = hash_search(hash_elem_pointer, (char)c);
				if (ret!=0)
				{
					//correspondent child was found so simply advance the pointer 
					hash_elem_pointer = ret;
				}
				else
				{
					//emit the father_index
					ret = emit((uint64_t)hash_elem_pointer);
					if (ret<0)
					{
						printf("Unable to emit the EOF\n");
						free(hash_table);
						return -1;
					}
					
					//add the new node 
					ret = hash_add(hash_elem_pointer, (char)c, ++hash_elem_counter);
					if (ret!=0){
						printf("Error in hash_add");
						free(hash_table);
						return -1;
					}
					
					//restart the search from the root
					hash_elem_pointer = 0;
					//now i search the caracter c again
					goto again;
				}
	}
	
	free(hash_table);
	return 0;
}
