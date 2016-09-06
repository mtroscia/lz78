#include "bitio.h"
#include "header.h"
#include "compressor.h"
#include "decompressor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define DICT_SIZE 65535
#define LZ78 1

void print_help()
{
    printf("Usage:\n");
	printf("lz78 -c -i <input_file> -o <output_file> for compression\n");
	printf("lz78 -d -i <input_file> -o <output_file> for decompression\n\n");
	printf("Other options:\n");
	printf("-s <dictionary_size>\n");
	printf("-h \thelp\n\n");
}

void print_content(char* dest)
{
	my_bitio_c=bit_open(dest, 0);
	uint64_t data;
	int ret;
	
	ret=0;
	while (1)
	{
		ret=bit_read(my_bitio_c, 9, &data);
		if (ret<0)
		{
			break;
		}
		
		printf ("read: %lu ", data);
		
	}
	
	bit_close(my_bitio_c);
}

int main(int argc, char *argv []) {
    int compr=-1, ret;// s=0, h=0;
    //compr is set to 1 if we want to compress, set to 2 if we want to decompress
    char* source=NULL, *dest=NULL;
    unsigned int dict_size=DICT_SIZE;//, d_dict_size;
    int opt;
	FILE* file;
	struct header* hd=NULL;

    while ((opt = getopt(argc,argv,"cdi:o:s:h"))!=-1) {
        switch (opt) {
			case 'c':
				compr = 0;
				break;
			case 'd':
				compr = 1;
				break;
			case 'i':
				source = optarg; //take the input name from optarg
				break;
			case 'o':
				dest = optarg; //take the output name from optarg
				break;
			case 's':
				//s=1;
				dict_size = atoi(optarg);
				break;
			case 'h':
				//h=1;
				print_help();
            break;
        case '?':
            if(optopt=='i'){
                fprintf(stderr,"An input file is required\n");
                exit(1);
            } else if (optopt=='o'){
                fprintf(stderr,"No name specified for destination file\n");
                exit(1);
            } else if (optopt=='s'){
                fprintf(stderr,"No dimension specified for dictionary size\n");
                exit(1);
            } else if (isprint(optopt)){
          		fprintf (stderr, "Unknown option `-%c'\n", optopt);
				exit(1);
			}else {
                fprintf(stderr, "Unknown option character `\\x%x'.\n Try -h for help\n", optopt);
                exit(1);
            }
            break;
			default:
				abort();
         } //switch (opt)
    } //while ()
	
	if (compr==-1){
		fprintf(stderr, "Error: you must specify either -c or -d option\n");
		print_help();
		exit(1);
	}

	if (compr!=-1 && (source==NULL || dest==NULL)){	
		fprintf(stderr, "Error: you must always specify input and output files\n");
		print_help();
		exit(1);
	}
    
	if (compr==0){		//compressing
		file = fopen(source, "r");
		if (file < 0){
			fprintf(stderr, "Error: file can't be opened in read mode\n");
			exit(1);
		}
		my_bitio_c = bit_open(dest, 1);
		if (my_bitio_c == NULL){
			fprintf(stderr, "Error: file can't be opened in write mode\n");
			fclose(file);
			exit(1);
		}
		
		hd = generate_header(file, source, LZ78, dict_size);
		if (hd == NULL) {
			exit(1);
		}
		ret = add_header(my_bitio_c, hd);
		if (ret == -1) {
			exit(1);
		}
		
		//initialize the hash table
		ret = hash_table_create(dict_size);
		if (ret == -1)
		{
			printf("Unable to create the hash table\n");
			exit(1);
		}
		
		//call the compression algorithm
		ret = compress(source);
		if (ret == -1)
		{
			printf("Unable to perform the compression\n");
			free(hash_table);
			exit(1);
		}
		
		ret = bit_flush(my_bitio_c);
		if (ret == -1){
			fprintf(stderr, "Error: cannot flush the buffer\n");
			fclose(file);
			exit(1);
		}
		
		printf("Compression completed.\n");
		
		//<choose between origin and compressed>
		
		//if(!<compressed>)
		//	<open new file>
		//	<add header for origin>
		//	<append origin file>

		ret = bit_close(my_bitio_c);
		if (ret == -1){
			fprintf(stderr, "Error: cannot flush the buffer\n");
			fclose(file);
			exit(1);
		}
		
		//******************************************************************************/
		//print_content(dest);
		/******************************************************************************/
		
	} else if (compr==1){		//decompressing
		
		my_bitio_d = bit_open(source, 0);
		if (my_bitio_d == NULL){
			printf ("Error in bit_open()\n");
			free (dictionary);
			return -1;
		}
		
		hd = get_header(my_bitio_d);
		if (hd == NULL) {
			exit(1);
		}
		
		//initialize all the data structure
		ret = init_decomp(hd->dict_size);
		if (ret < 0){
			printf("Unable to perform the initialization phase.\n");
			exit(1);
		}
		
		//decode
		ret = decompress(dest);
		if (ret<0){
			printf("Error in decompression\n");
			exit(1);
		}
		
		ret = bit_close(my_bitio_d);
		if (ret < 0)
		{
			printf("unable to close the file\n");
			exit (1);
		}

		file = fopen(dest, "r");
		
		ret = check_integrity(hd, file); 
		if (ret == -1) {
			exit(1);
		}
		
		printf("Decompression completed.\n");
	}
		
	return 0;
}
