#include "header.h"

int create_checksum(FILE* fd, int size, unsigned char** out) {	
	int ret, ret_r, i;
	SHA256_CTX* ctx;
	int buf_size = 128;
	void* in = malloc(buf_size);
	int num_blocks = size/buf_size + 1;
	
	printf("Composing SHA256...\t");
	fseek(fd, 0, SEEK_SET);
	
	ctx = (SHA256_CTX*)malloc(sizeof(SHA256_CTX));

	ret = SHA256_Init(ctx);
	if (ret != 1) {
		printf("Error in initializing the context...\n");
		return -1;
	}
	
	for (i=0; i<num_blocks; i++) {
		if (i != num_blocks-1) {
			ret_r = fread(in, buf_size, 1, fd);
			if (ret_r < 0) {
				return -1;
			}
			ret = SHA256_Update(ctx, in, buf_size);
			if (ret == 0) {
				return -1;
			}
		} else {
			ret_r = fread(in, buf_size, 1, fd);
			if (ret_r < 0) {
				return -1;
			}
			ret = SHA256_Update(ctx, in, ret_r);
			if (ret == 0) {
				return -1;
			}
		}
	}
	ret = SHA256_Final(*out, ctx); 
	if (ret == 0) {
		return -1;
	}
	
	printf("OK\n");
	
	/*int ret;
	void* in;
	in = malloc(size);
	ret = fread(in, size, 1, fd);
	if (ret < 0) {
		return -1;
	}
	printf("Read from file...\n");
	
	SHA256((unsigned char*)in, size, *out);
	printf("SHA256 is OK...\n");
	
	printf("%s\n", (char*)*out);*/
	
	return 0;
}

//testing
void print_header(struct header* hd){
	printf("hd->compressed %i\n", hd->compressed);
	printf("hd->orig_filename_len %i\n", hd->orig_filename_len);
	printf("hd->orig_filename %s\n", hd->orig_filename);
	printf("hd->orig_size %lu\n", hd->orig_size);
	printf("hd->orig_creation_time %lu\n", hd->orig_creation_time);
	printf("hd->checksum %s\n", hd->checksum);
	printf("hd->compr_alg %i\n", hd->compr_alg);
	printf("hd->dict_size %i\n", hd->dict_size);
}
	
struct header* generate_header(FILE* file, char* file_name, uint8_t alg, int d_size){
	struct header* hd;
	struct stat file_info;
	int ret, i;
	unsigned char* out = malloc(SHA256_DIGEST_LENGTH);
	
	printf("Composing the header...\n");
	
	hd = (struct header*)calloc(1, sizeof(struct header));
	if (hd == NULL){
		return NULL;
	}
	
	hd->compressed = 1;
	hd->orig_filename_len = strlen(file_name); 
	hd->orig_filename = malloc(hd->orig_filename_len+1);	//to include \0 
	strcpy(hd->orig_filename, file_name);
	
	if (stat(file_name, &file_info)) {
		printf("Error in reading the info of the file\n");
		free(hd);
		return NULL;
	}
	
	hd->orig_size = file_info.st_size;
	hd->orig_creation_time = (uintmax_t)file_info.st_ctime;
	
	ret = create_checksum(file, hd->orig_size, &out);	
	if (ret == -1) {
		printf("Error in creating the checksum...\n");
		free(hd);
		return NULL;
	}
	memcpy(hd->checksum, out, SHA256_DIGEST_LENGTH);
	
	hd->compr_alg = alg;
	hd->dict_size = d_size;
	
	print_header(hd);
	//printf("OK...\n");
	
	return hd;
}

int add_header(struct bitio* my_bitio, struct header* hd) {
	int ret, i;
	
	printf("Start writing into output file...\n");
	
	ret = bit_write(my_bitio, 8, (uint8_t)hd->compressed);
	if (ret != 0) {
		return -1;
	}
	ret = bit_write(my_bitio, 8, (uint8_t)hd->orig_filename_len);
	if (ret != 0) {
		return -1;
	}
	for (i=0; i<hd->orig_filename_len; i++) {
		ret = bit_write(my_bitio, 8, (char)(hd->orig_filename[i]));
		if (ret != 0) {
			return -1;
		}
	}
	ret = bit_write(my_bitio, sizeof(off_t)*8, (off_t)(hd->orig_size));
	if (ret != 0) {
		return -1;
	}
	ret = bit_write(my_bitio, sizeof(uintmax_t)*8, (uintmax_t)(hd->orig_creation_time));
	if (ret != 0) {
		return -1;
	}
	for (i=0; i<32; i++) {
		ret = bit_write(my_bitio, 8, (unsigned char)(hd->checksum[i]));
		if (ret != 0) {
			return -1;
		}
	}
	ret = bit_write(my_bitio, 8, (uint8_t)(hd->compr_alg));
	if (ret != 0) {
		return -1;
	}
	ret = bit_write(my_bitio, sizeof(int)*8, (int)(hd->dict_size));
	if (ret != 0) {
		return -1;
	}
	
	printf("Write is complete...\n");
	return 0;
}

