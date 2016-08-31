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

void print_help()
{
    printf("Usage:\n\
			lz78 -c -i <input_file> -o <output_file> for compression\n\
			lz78 -d -i <input_file> -o <output_file> for decompression\n\n\
			Other options:\n\
			-s <dictionary_size>\n\
			-h \thelp\n\n");
}

void print_content(char*dest)
{
	my_bitio=bit_open(dest, 0);
	uint64_t data;
	int ret;
	
	ret=0;
	while (1)
	{
		ret=bit_read(my_bitio, 9, &data);
		if (ret<0)
		{
			break;
		}
		
		printf ("read: %lu ", data);
		
	}
}

int main(int argc, char *argv []) {
    int fd, compr=-1, s=0, h=0, ret;
    //compr is set to 1 if we want to compress, set to 2 if we want to decompress
    char* source=NULL, *dest=NULL;
    unsigned int dict_size=DICT_SIZE;//, d_dict_size;
    struct bitio* fd_bitio = NULL;
    int opt;
	

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
				s=1;
				dict_size = atoi(optarg);
				break;
			case 'h':
				h=1;
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
		fd = open(source, O_RDONLY);
		if (fd < 0){
			fprintf(stderr, "Error: file can't be opened in read mode\n");
			exit(1);
		}
		fd_bitio = bit_open(dest, 1);
		if (fd_bitio == NULL){
			fprintf(stderr, "Error: file can't be opened in write mode\n");
			close(fd);
			exit(1);
		}
		//<add header>
		
		//open the bitio stream in write mode
		my_bitio=bit_open(dest,1);
		if (my_bitio==NULL)
		{
			printf("Unable to open a bitio stream\n");
			return -1;
		}
		
		//initialize the hash function
		ret = hash_table_create(dict_size);
		if (ret==-1)
		{
			printf("Unable to create the hash table\n");
			return -1;
		}
		
		//call the compression algorithm
		ret = compress (source);
		if (ret==-1)
		{
			printf("Unable to perform the compression\n");
			free(hash_table);
			return -1;
		}
		
		printf("Compression completed.\n");
		//******************************************************************************/
		print_content(dest);
		/******************************************************************************/
		free(hash_table);
	} else if (compr==1){		//decompressing
		fd = open(dest, (O_CREAT | O_TRUNC | O_WRONLY));
		if (fd < 0){
			fprintf(stderr, "Error: file can't be opened in write mode\n");
			exit(1);
		}
		fd_bitio = bit_open(source, 0);
		if (fd_bitio == NULL){
			fprintf(stderr, "Error: file can't be opened in read mode\n");
			close(fd);
			exit(1);
		}
		//<check header>
		//<call decompress function>
		printf("Decompression completed.\n");
	}

	printf("End of main...\n");
	
	
		
	return 0;
}