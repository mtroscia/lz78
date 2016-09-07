#include "decompressor.h"

int array_reset();

int array_add(uint32_t father_index, char character)
{	
	array_elem_counter++;
	
	//eventually update the number of bits for the symbols
	if((1<<actual_bits_counter)==array_elem_counter) 
			actual_bits_counter++;
	
	//add the element
	dictionary[array_elem_counter].father_index =father_index;
	dictionary[array_elem_counter].character =character;

	//print_hash_table();
	
	return 0;
}


int array_init(){
	int i, ret;
	char c;
	
	array_elem_counter = 0;
	//initially we have 257 element
	actual_bits_counter = 9;
	
	//add all ASCII characters
	for (i=0; i<=255; i++)
	{
		c=i;
		//printf("Added %c\n", c);
		
		ret = array_add(0, c);
		if (ret!=0){
			fprintf(stderr, "Error in array_add()\n");
			return -1;
		}
	}
	
	return 0;
}


int array_reset(){
	int ret;
	
	//reset all fields
	bzero(dictionary, dictionary_size*sizeof(struct array_elem));
	
	//add all root's children
	ret = array_init();	
	if (ret!=0){
		fprintf(stderr, "Error when reinitializing the hash table\n");
		return -1;
	}    
        
	return 0;
}


//initialize decompression
int init_decomp(int dict_size)
{	
	int ret;
	
	decomp_buffer = calloc(8, sizeof (char)); 
	
	dictionary_size = dict_size;
	dictionary = calloc(dictionary_size, sizeof (struct array_elem));
	if (dictionary == NULL){
		fprintf(stderr, "Unable to create a valid tree for the decompression phase.\n");
		return -1;
	}
	
	ret = array_init();
	if (ret < 0)
	{
		fprintf(stderr, "Error in array_init()\n");
		return -1;
	}
	
	return 0;
	
}

int extend_buffer()
{
	int i;
	char* new_buffer;

	new_buffer = calloc((sizeof(decomp_buffer))*2, sizeof(char));
	if (new_buffer == NULL)
	{
		fprintf(stderr, "Unable to allocate enough memory\n");
		return -1;
	}
	
	//copy the element from the old buffer to the new one
	for (i=0; i < sizeof(decomp_buffer); i++)
	{
		new_buffer[i] = decomp_buffer[i];
	}
	
	free(decomp_buffer);
	decomp_buffer = new_buffer;
	return 0;
}

int emit_symbols(FILE* f)
{
	char  temp;
	int i, j = 0;
	
	//need to reverse the order of the characters contained into the buffer
	i = 0;
	j = strlen(decomp_buffer)-1;
 
	while (i < j) {
		temp = decomp_buffer[i];
		decomp_buffer[i] = decomp_buffer[j];
		decomp_buffer[j] = temp;
		i++;
		j--;
	}
 
	//print into the file
	fprintf(f, "%s", decomp_buffer);
	
	return 0;
}

int find_path(uint32_t child_index, int unknown_node, FILE* f)
{
	int i, ret, unlikely;
	char character;
	
	i = 0;
	unlikely = 0;
	
	while (1)
	{	
		//read all the array's info
		character = dictionary[child_index].character;
		
		decomp_buffer[i]=character;       
				
		if ((i == 0) && (child_index == (array_elem_counter-1)))
        {
            unlikely = 1;
        }
				
		child_index = dictionary[child_index].father_index;
		
		if (child_index == 0)	//root of the tree reached
			break;
		
		if (i == (sizeof decomp_buffer -1))
		{
			// the decomp_buffer is too small
			ret = extend_buffer();
			
			if (ret<0)
			{
				fprintf(stderr, "Unable to extend the decompression buffer\n");
				return -1;
			}
		} 
		
		i++;
	}
		
	//at the first step we don't have any node with unknown character value
    if (unknown_node != 0)
	{
		//printf("Update the unknown node%i: %c\n", array_elem_counter-1, character);
		
		//there is a child node with an unknown character (it is the last one!)
		dictionary[array_elem_counter-1].character = character;
	}
	
	if (unlikely)
    {
        decomp_buffer[0] = decomp_buffer[i];    
    }
		
	decomp_buffer[++i] = '\0';
	
	ret = emit_symbols(f);
	
	return 0;
}

int decode(FILE* f)
{
	uint64_t node_index;
	int ret;
	
	unknown_node=0;
	
	printf ("\nIn decode\n");
	while (1)
	{
		ret = bit_read(my_bitio_d, actual_bits_counter, &node_index);
		if (ret<0)
		{
			fprintf(stderr, "Error in bit_read()\n");
			return -1;
		}
		
		//EOF symbol reached
		if (node_index == 0)
		{
			break;
		}
		
		if (array_elem_counter < dictionary_size-1)
        {
            //add an unknown node
            ret = array_add(node_index, -1);
				
            //browse the tree
            ret = find_path(node_index, unknown_node, f);
			if (ret == -1) {
				return -1;
			}
			
			unknown_node = 1;
        }
        else
		{
			//browse the tree
			++array_elem_counter;
			ret = find_path(node_index, unknown_node, f);
			if (ret == -1) {
				return -1;
			}
			
			array_reset();

            unknown_node = 0;
        }	
	}
	
	return 0;
}


int decompress(char* dest_file_name)
{
	//open dest with fopen
	FILE* f;
	int ret;
	
	f = fopen(dest_file_name, "w");
	if (f == NULL)
	{
		fprintf(stderr, "Unable to create hash_table_file\n");
		return -1;
	}
	
	ret = decode(f);
	if (ret == -1){
		fprintf(stderr, "Error in decode\n");
		return -1;
	}

	fclose(f);
	
	return 0;
}