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

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		return hash;
}

int hash_table_create(uint64_t size){
	int ret;
	
	dictionary_size=size;
	hash_table_size = size+(size/2);
	printf("dictionary_size = %i - hash_table_size = %i\n", dictionary_size, hash_table_size);
	
	/*********************************************************************************************************/
	//hash_table = (struct hash_elem*)calloc(1, sizeof(struct hash_elem)*size);
	hash_table= (struct hash_elem*)calloc(hash_table_size,sizeof(struct hash_elem)); // sel la vogliamo usare come un array forse questa dizione è più corretta
	/*********************************************************************************************************/
	
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
	
	//aggiungiamo tutti i caratteri ASCII
	for (i=0; i<=255;i++)
	{
		c=i;
		printf("added %c\n", c);
		
		ret = hash_add(0,c,++hash_elem_counter);
		if (ret!=0){
			printf("Error in hash_add");
			free(hash_table);
			return -1;
		}
		printf ("Actual hash_elem_counter = %i\n", hash_elem_counter);
		
	}
	
	printf("contenuto di hash_table[0]:\tfather_index %i - character %c - child_index %i\n",
			hash_table[11598].father_index, hash_table[11598].character,hash_table[11598].child_index);
	
	return 0;
}

int hash_add(uint32_t father, char symbol, uint32_t child){
	unsigned long index;
	
	if (child>dictionary_size)
		hash_reset();
	
	//eventually update the number of bits for the symbols
	if((1<<actual_bits_counter)==hash_elem_counter) 
			actual_bits_counter++;
	
	//obtain the index into the hash table
	index=generate_hash_index(father, symbol);
	
	//add the elem
	int collision=1;
	while (collision)
	{
		if (hash_table[index].child_index==0) 
		{
			//there isn't a valid value in this entry (empty entry)
			//I can make the insert 
			hash_table[index].father_index=father;
			hash_table[index].character=symbol;
			hash_table[index].child_index=hash_elem_counter;
			collision=0;
		}
		else
		{
			printf("Collision for index %lu!->check the next entry", index);
			//I treat the hash table as a circular array
			index=(index+1)%hash_table_size;
		}
	}
	
	/************************************ONLY FOR TESTING PURPOSES******************************************************/
	
		print_hash_table();
	
	/******************************************************************************************************************/
	
	return 0;
}



unsigned long generate_hash_index(uint32_t father, char symbol){
	unsigned char buffer[33];
	char s[2];
	
	unsigned long index;
	
	//<generate the string using father and symbol>
	sprintf((char*)buffer, "%d", father);
	//symbol from char to string
	sprintf(s, "%c", symbol);
	strcat ((char*)buffer,s);
	//compute hash for the string
	index=hash(buffer);
	//obtain an index from the hash
	index%=hash_table_size;
	
	printf("string: %s- hash index: %lu\n", buffer, index);
	
	return index;
}


/********************************************************
search a node in the hash table using father-char as key.
if it found the node, then it return the index.
Otherwise it return 0;
********************************************************/
uint32_t hash_search (uint32_t father, char symbol){
	unsigned long index;
	int not_found;
	
	index=generate_hash_index(father, symbol);
	
	not_found=1;
	do
	{
		if (hash_table[index].father_index==father && hash_table[index].character==symbol)
		{
			//I have found the wanted entry
			not_found=0;
			printf("%i %c found:\t child value=%i\n",hash_table[index].father_index,
					hash_table[index].character,hash_table[index].child_index );
		}else
		{
			if (hash_table[index].child_index==0)
			{
				//this entry is an empty one
				return 0;
			}else  //search into the next entry
				index=(index+1)%hash_table_size;
		}		
	}while (not_found);
	
	return hash_table[index].child_index;
}

int hash_reset(){
	int ret;
	
	//<resetta tutti i campi della hash>
	bzero(hash_table, hash_table_size*sizeof (struct hash_elem));
	
	//add all root's children
	ret=hash_init();	
	if (ret!=0){
			printf("Error reinitialize the hash table");
			free(hash_table);
			return -1;
	}
	return 0;	
}



/************************************ONLY FOR TESTING PURPOSES******************************************************/
void print_hash_table(){
	FILE *f;
	int i;
	
	f=fopen("hash_table_file.txt", "w");
	if (f == NULL)
	{
		printf("Unable to create hash_table_file");
		return;
	}
	
	for (i=0; i<hash_table_size; i++)
	{
		fprintf(f,"%i)father_index: %i\tcharacter: %c\tchild_index: %i\n",i,
				hash_table[i].father_index, hash_table[i].character, hash_table[i].child_index);
	}
}
	
/******************************************************************************************************************/
