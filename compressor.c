#include "compressor.h"

unsigned long hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		return hash;
}

int hash_table_create(uint64_t size){
	int ret;
	hash_table_size = size;
	hash_table = (hash_elem*)calloc(1, sizeof(hash_elem)*size);
	if(hash_table==NULL){
		printf("Error: hash table has not been created.\n");
		return -1;
	}
	
	ret = hash_init();
	if (ret!=0){
		printf("Error: hash table has not been initialized.\n");
		return -1;
	}
	
	return 0;
}

int hash_init(){
	
}

int hash_add(uint32_t father, char symbol, uint32_t child){
	//<generate the string using father and symbol>
	//<compute hash of that string>
}