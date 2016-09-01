/*to build the header of the compressed and decompressed file
it contains useful info about the original and compressed file*/

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "bitio.h"

struct header{
	//bit to say if file is compressed or not
	char* compr_alg;	//numerical code --> int
	int dict_size;
	int symb_size;
	//file metadata
	char* orig_filename;
	int orig_size;
	time_t orig_creation_time; // maybe better double (difference of times)
	//checksum BUT HOW TO DO IT????!?!?
};

//MORE OR LESS...
void add_header(struct bitio* file, struct header* hd, char* alg, int d_size, int s_size, char* file_name, int f_size, time_t* time);
struct header* get_header(struct bitio* file);