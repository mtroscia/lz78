#include "bitio.h"
#include "header.h"
#include "compressor.h"
#include "decompressor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define DICT_SIZE 65535
#define LZ78 1

void print_help()
{
	fprintf(stderr, "\nUsage:\n");
	fprintf(stderr, "lz78 -c -i <input_file> -o <output_file> for compression\n");
	fprintf(stderr, "lz78 -d -i <input_file> -o <output_file> for decompression\n\n");
	fprintf(stderr, "Other options:\n");
	fprintf(stderr, "-s <dictionary_size>\n");
	fprintf(stderr, "-h \thelp\n");
	fprintf(stderr, "-v verbose\n\n");
}

//testing
void print_content(char* dest)
{
	my_bitio_c = bit_open(dest, 0);
	uint64_t data;
	int ret;
	
	ret=0;
	while (1)
	{
		ret = bit_read(my_bitio_c, 9, &data);
		if (ret<0)
		{
			break;
		}
		
		fprintf(stderr, "Read: %lu ", data);
		
	}
	
	bit_close(my_bitio_c);
}

int decide_file(char* d_name, FILE* fp_s, struct header* hd, struct timeval t, int verbose){
	struct stat file_info;
	uint64_t size_compr;
	int header_size, ret, ret_r, i, buf_size = 1024;
	FILE* fp_d = NULL;
	void* buf = malloc(buf_size);
	struct bitio* b;
	struct timeval stop;
	
	if (fp_s == NULL || hd == NULL) {
		fprintf(stderr, "One of the passed parameters is NULL\n");
		return -1;
	}
	
	if (stat(d_name, &file_info)){
		fprintf(stderr, "Error in reading the file info\n");
		return -1;
	}
	
	header_size = 8+8+8*hd->orig_filename_len+64+64+8*SHA256_DIGEST_LENGTH+8+32;
	size_compr = (uint64_t)file_info.st_size - (uint64_t)header_size;
	
	if (verbose == 1){
		fprintf(stderr, "\n--Compressed file--\n");
		fprintf(stderr, "Output file name: %s\n", d_name);
	}
	
	if (size_compr > hd->orig_size){
		
		if (verbose == 1){
			fprintf(stderr, "Output file size: %luB\n", hd->orig_size);
			fprintf(stderr, "Compressed file is larger than the original one.\n\n");
		}
		
		hd->compressed = 0;
		
		ret = remove(d_name);
		if (ret != 0){
			fprintf(stderr, "Error in removing the file\n");
			return -1;
		}
		
		b = bit_open(d_name, 1);
		if (b==NULL) {
			fprintf(stderr, "Error in opening bitio...\n");
			return -1;
		}
		
		ret = bit_write(b, 8, (uint8_t)hd->compressed);
		if (ret != 0) {
			fprintf(stderr, "Error in bit_write()\n");
			return -1;
		}
		ret = bit_write(b, 8, (uint8_t)hd->orig_filename_len);
		if (ret != 0) {
			fprintf(stderr, "Error in bit_write()\n");
			return -1;
		}
		for (i=0; i<hd->orig_filename_len; i++){
			ret = bit_write(b, 8, (char)hd->orig_filename[i]);
			if (ret != 0) {
				fprintf(stderr, "Error in bit_write()\n");
				return -1;
			}
		}
		ret = bit_write(b, 64, hd->orig_size);
		if (ret != 0) {
			fprintf(stderr, "Error in bit_write()\n");
			return -1;
		}
		ret = bit_write(b, 64, hd->orig_creation_time);
		if (ret != 0) {
			fprintf(stderr, "Error in bit_write()\n");
			return -1;
		}
		for (i=0; i<SHA256_DIGEST_LENGTH; i++){
			ret = bit_write(b, 8, (unsigned char)hd->checksum[i]);
			if (ret != 0) {
				fprintf(stderr, "Error in bit_write()\n");
				return -1;
			}
		}
		
		ret = bit_flush(b);
		if (ret != 0) {
			fprintf(stderr, "Error in bit_flush()\n");
			return -1;
		}
		
		fp_d = get_pointer(b);
		if (fp_d == NULL) {
			fprintf(stderr, "NULL pointer\n");
			return -1;
		}
		
		fseek(fp_d, header_size-40, SEEK_SET);
		fseek(fp_s, 0, SEEK_SET);
		
		do {
			ret_r = fread(buf, sizeof(char), buf_size, fp_s);
			if (ret_r<0) {
				fprintf(stderr, "Error in fread()\n");
				return -1;
			}
			ret = fwrite(buf, sizeof(char), ret_r, fp_d);
			if (ret<0) {
				fprintf(stderr, "Error in fwrite()\n");
				return -1;
			}
		} while (ret_r);
		
	} else {
		if (verbose == 1){
			fprintf(stderr, ("Output file size: %luB\n", size_compr);
			fprintf(stderr, "Percentage of compression %0.2f%%\n", (double)size_compr/(double)hd->orig_size*100);
			gettimeofday(&stop, NULL);
			fprintf(stderr, "Compression completed in %i milliseconds\n\n", (int)(stop.tv_sec-t.tv_sec)*1000+(int)(stop.tv_usec-t.tv_usec)/1000);
		}
		
	}
	
	return 0;
	
}

int obtain_orig_file(struct bitio* b, struct header* hd, char* dest){
	int header_size, ret, ret_r, buf_size = 1024;
	FILE* file_r, *file_w;
	void* buf;
	
	file_r = get_pointer(b);
	if (file_r <= 0){
		fprintf(stderr, "Error in file pointer\n");
		return -1;
	}
	file_w = fopen(dest, "w");
	if (file_w <= 0){
		fprintf(stderr, "Error in file pointer\n");
		return -1;
	}
	
	buf = malloc(hd->orig_size);
			
	header_size = 8+8+8*hd->orig_filename_len+64+64+8*SHA256_DIGEST_LENGTH;
	fseek(file_r, header_size, SEEK_SET);
			
	do {
		ret_r = fread(buf, sizeof(char), buf_size, file_r);
		if (ret_r<0) {
			fprintf(stderr, "Error in fread()\n");
			return -1;
		}
		ret = fwrite(buf, sizeof(char), ret_r, file_w);
		if (ret<0) {
			fprintf(stderr, "Error in fwrite()\n");
			return -1;
		}
	} while (ret_r);
							
	fclose(file_r);
	fclose(file_w);
	return 0;
}


int main(int argc, char *argv []) {
    int compr = -1, ret, verbose = 0, opt;
    //compr is set to 1 to compress, set to 2 to decompress
    char* source = NULL, *dest = NULL;
    unsigned int dict_size = DICT_SIZE;
	FILE* file;
	struct header* hd = NULL;
	struct timeval start, stop;

    while ((opt = getopt(argc, argv, "cdi:o:s:hv"))!=-1) {
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
				if (compr == 1)
					fprintf(stderr,"\nYou can't choose the dictionary size in the decompression phase.\nThis option will be ignored\n");
				else{
					dict_size = atoi(optarg);
					dict_size = (dict_size < 500)? 500 : (dict_size > 100000)? 100000 : dict_size;
				}
				break;
			case 'h':
				print_help();
				break;
			case 'v':
				verbose = 1;
				break;
			case '?':
				if(optopt == 'i'){
					fprintf(stderr,"An input file is required\n");
					exit(1);
				} else if (optopt == 'o'){
					fprintf(stderr,"No name specified for destination file\n");
					exit(1);
				} else if (optopt == 's'){
					fprintf(stderr,"No dimension specified for dictionary size\n");
					exit(1);
				} else if (isprint(optopt)){
					fprintf (stderr, "Unknown option `-%c'\n", optopt);
					exit(1);
				} else {
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

	
	if (compr == 0 && dest == NULL){	
		fprintf(stderr, "You don't have specified an output name.\n");
		
		char *extension;
		
		dest = calloc (strlen (source) + 1, sizeof(char));
			
		strcpy(dest, source);
		extension = strrchr(dest, '.');
		if (extension != NULL)
			*extension = '\0';

		strcat(dest, ".cgt");
			
		fprintf(stderr, "\nWe will use this name: %s\n", dest);
	}
	
	if (source == NULL){
		fprintf(stderr, "Error: you must always specify the input files\n");
		exit(1);
	}
    
	if (compr==0){		//COMPRESSION	
	
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
		
		hd = generate_header(file, source, LZ78, dict_size, verbose);
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
			fprintf(stderr, "Unable to create the hash table\n");
			exit(1);
		}
		
		//call the compression algorithm
		gettimeofday(&start, NULL);
		ret = compress(source);
		if (ret == -1)
		{
			fprintf(stderr, "Unable to perform the compression\n");
			free(hash_table);
			exit(1);
		}
		
		ret = bit_flush(my_bitio_c);
		if (ret == -1){
			fprintf(stderr, "Error: cannot flush the buffer\n");
			fclose(file);
			exit(1);
		}
		
		ret = bit_close(my_bitio_c);
		if (ret == -1){
			fprintf(stderr, "Error: cannot flush the buffer\n");
			fclose(file);
			exit(1);
		}
		
		if (verbose == 1){
			fprintf(stderr, "Compression completed.\n");
		}
		
		
		ret = decide_file(dest, file, hd, start, verbose);
		if (ret == -1){
			exit(1);
		}
		
	} else if (compr==1){		//DECOMPRESSION
		
		my_bitio_d = bit_open(source, 0);
		if (my_bitio_d == NULL){
			fprintf(stderr, "Error in bit_open()\n");
			free (dictionary);
			return -1;
		}
		
		hd = get_header(my_bitio_d);
		if (hd == NULL) {
			exit(1);
		}
		
		if (dest == NULL){
			dest = calloc (hd ->orig_filename_len, sizeof(char));
			strcpy (dest, hd->orig_filename);
		}
		
		gettimeofday(&start, NULL);
		
		if (verbose == 1){
			fprintf(stderr, "\n--Compressed file--\n");
			fprintf(stderr, "File name: %s\n\n", source);
		}
		
		if (hd->compressed == 1){		//original file compressed
			//initialize all the data structure
			ret = init_decomp(hd->dict_size);
			if (ret < 0){
				fprintf(stderr, "Unable to perform the initialization phase\n");
				exit(1);
			}
			
			//decode
			ret = decompress(dest);
			if (ret<0){
				fprintf(stderr, "Error in decompression\n");
				exit(1);
			}
			
			ret = bit_close(my_bitio_d);
			if (ret < 0)
			{
				fprintf(stderr, "Unable to close the file\n");
				exit (1);
			}
			
		} else {			//original file not compressed
		
			ret = obtain_orig_file(my_bitio_d, hd, dest);
			if (ret == -1){
				exit(1);
			}
		}
		
		if (verbose == 1){
			fprintf(stderr, "Decompression completed.\n");
		}
		
		if (verbose == 1){
			fprintf(stderr, "\n--Decompressed file--\n");
			gettimeofday(&stop, NULL);
			fprintf(stderr, "Decompression completed in %i milliseconds\n", (int)(stop.tv_sec-start.tv_sec)*1000+(int)(stop.tv_usec-start.tv_usec)/1000);
		}
		
		file = fopen(dest, "r");
		
		ret = check_integrity(hd, file); 
		if (ret == -1) {
			exit(1);
		}
		
		if (verbose == 1){
			fprintf(stderr, "No errors occur while decompressing.\n\n");
		}
	}
		
	return 0;
}
