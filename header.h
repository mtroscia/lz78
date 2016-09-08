/*to build the header of the compressed and decompressed file
it contains useful info about the original and compressed file*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <time.h>
#include "portable_endian.h"
#include "bitio.h"

struct header{
	uint8_t compressed;
	
	//file metadata
	uint8_t orig_filename_len;
	char* orig_filename;
	uint64_t orig_size;
	uint64_t orig_creation_time; //w.r.t 01/01/1970
	unsigned char checksum[SHA256_DIGEST_LENGTH];
	
	//compression information
	uint8_t compr_alg;
	int dict_size;
};

//void print_bytes(const void*, size_t);
//void print_header(struct header* hd);

struct header* generate_header(FILE* file, char* file_name, uint8_t alg, int d_size, int v);
int add_header(struct bitio* my_bitio, struct header* hd);

struct header* get_header(struct bitio* file);
int check_integrity(struct header* hd, FILE* file);