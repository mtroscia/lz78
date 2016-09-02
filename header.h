/*to build the header of the compressed and decompressed file
it contains useful info about the original and compressed file*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <time.h>
#include "bitio.h"

struct header{
	uint8_t compressed;
	
	//file metadata
	uint8_t orig_filename_len;
	char* orig_filename;
	off_t orig_size;
	uintmax_t orig_creation_time; //w.r.t 01/01/1970
	unsigned char checksum[SHA256_DIGEST_LENGTH];	//256/8
	
	//compression information
	uint8_t compr_alg;
	int dict_size;
};

struct header* generate_header(FILE* file, char* file_name, uint8_t alg, int d_size);
int add_header(struct bitio* my_bitio, struct header* hd);

struct header* get_header(struct bitio* file);
int check_integrity(struct header* hd, FILE* f);