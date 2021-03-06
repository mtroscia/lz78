//functions to build and retrieve the header from files

#include "header.h"

//generate SHA256 checksum of the original file
int create_checksum(FILE* fd, int size, unsigned char** out) {	
	int ret, ret_r, i;
	SHA256_CTX* ctx;
	int buf_size = 1024;
	void* in = malloc(buf_size);
	int num_blocks = size/buf_size + 1;
	
	fseek(fd, 0, SEEK_SET);
	ctx = (SHA256_CTX*) malloc(sizeof(SHA256_CTX));
	
	ret = SHA256_Init(ctx);
	if (ret != 1) {
		fprintf(stderr, "Error in initializing the context...\n");
		return -1;
	}
	
	for (i = 0; i < num_blocks; i++) {
		ret_r = fread(in, 1, buf_size, fd);
		if (ret_r < 0) {
			return -1;
		}
		ret = SHA256_Update(ctx, in, ret_r);
		if (ret != 1) {
			fprintf(stderr, "Error in updating the context...\n");
			return -1;
		}
	}
	
	ret = SHA256_Final(*out, ctx); 
	if (ret != 1) {
		fprintf(stderr, "Error in finalizing the context...\n");
		return -1;
	}
	
	return 0;
}
	
struct header* generate_header(FILE* file, char* file_name, uint8_t alg, int d_size, int verbose){
	struct header* hd;
	struct stat file_info;
	int ret;
	unsigned char* out;
	struct tm* info;
	
	if (file == NULL){
		fprintf(stderr, "One of the passed parameters is NULL\n");
		return NULL;
	}
	
	hd = (struct header*) calloc(1, sizeof(struct header));
	if (hd == NULL){
		return NULL;
	}
	
	hd->compressed = 1;
	hd->orig_filename_len = strlen(file_name);
	hd->orig_filename = malloc(hd->orig_filename_len+1);	 //to include \0 
	strcpy(hd->orig_filename, file_name);
	
	if (stat(file_name, &file_info)) {
		fprintf(stderr, "Error in reading the file info\n");
		free(hd);
		return NULL;
	}
	
	hd->orig_size = (uint64_t)file_info.st_size;
	hd->orig_creation_time = (uint64_t)file_info.st_ctime;
	
	out = malloc(SHA256_DIGEST_LENGTH);
	ret = create_checksum(file, hd->orig_size, &out);	
	if (ret == -1) {
		fprintf(stderr, "Error in creating the checksum...\n");
		free(hd);
		return NULL;
	}
	memcpy(hd->checksum, out, SHA256_DIGEST_LENGTH);
	
	hd->compr_alg = alg;
	hd->dict_size = d_size;
	
	if (verbose == 1){
		fprintf(stderr, "\n--Original file--\n");
		fprintf(stderr, "File name: %s\n", hd->orig_filename);
		fprintf(stderr, "Size: %luB\n", hd->orig_size);
		info = gmtime((time_t*)&hd->orig_creation_time);
		fprintf(stderr, "Creation time: %i-%02i-%02i %02i:%02i:%02i\n\n", info->tm_year+1900, info->tm_mon+1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec);
	}
	
	return hd;
}

int add_header(struct bitio* my_bitio, struct header* hd) {
	int ret, i;
	uint64_t temp64;
	uint32_t temp32;
	
	if (my_bitio==NULL || hd==NULL) {
		fprintf(stderr, "One of the passed parameters is NULL\n");
		return -1;
	}
	
	ret = bit_write(my_bitio, 8, (uint8_t)hd->compressed);
	if (ret != 0) {
		fprintf(stderr, "Error in bit_write()\n");
		return -1;
	}
	
	ret = bit_write(my_bitio, 8, (uint8_t)hd->orig_filename_len);
	if (ret != 0) {
		fprintf(stderr, "Error in bit_write()\n");
		return -1;
	}
	
	for (i=0; i<hd->orig_filename_len; i++) {
		ret = bit_write(my_bitio, 8, (char)(hd->orig_filename[i]));
		if (ret != 0) {
			fprintf(stderr, "Error in bit_write()\n");
			return -1;
		}
	}
	
	temp64 = htole64((uint64_t)hd->orig_size);
	ret = bit_write(my_bitio, 64, temp64);
	if (ret != 0) {
		fprintf(stderr, "Error in bit_write()\n");
		return -1;
	}
	
	temp64 = htole64((uint64_t)hd->orig_creation_time);
	ret = bit_write(my_bitio, 64, temp64);
	if (ret != 0) {
		fprintf(stderr, "Error in bit_write()\n");
		return -1;
	}
	
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		ret = bit_write(my_bitio, 8, (unsigned char)(hd->checksum[i]));
		if (ret != 0) {
			fprintf(stderr, "Error in bit_write()\n");
			return -1;
		}
	}
	
	ret = bit_write(my_bitio, 8, (uint8_t)(hd->compr_alg));
	if (ret != 0) {
		fprintf(stderr, "Error in bit_write()\n");
		return -1;
	}
	
	temp32 = htole32((int)hd->dict_size);
	ret = bit_write(my_bitio, 32, temp32);
	if (ret != 0) {
		fprintf(stderr, "Error in bit_write()\n");
		return -1;
	}
	
	return 0;
}

struct header* get_header(struct bitio* my_bitio){
	int ret, i;
	uint64_t buf = 0;
	struct header* hd;
	
	if (my_bitio == NULL) {
		fprintf(stderr, "One of the passed parameters is NULL\n");
		return NULL;
	}
	
	hd = (struct header*) calloc(1, sizeof(struct header));
	if (hd == NULL){
		return NULL;
	}
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		fprintf(stderr, "Error in bit_read()\n");
		return NULL;
	}
	hd->compressed = (uint8_t)buf;
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		fprintf(stderr, "Error in bit_read()\n");
		return NULL;
	}
	hd->orig_filename_len = (uint8_t)buf;
	
	hd->orig_filename = malloc(hd->orig_filename_len+1);	//to consider /0
	for (i = 0; i < hd->orig_filename_len; i++) {
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			fprintf(stderr, "Error in bit_read()\n");
			return NULL;
		}
		hd->orig_filename[i] = (char)buf;
	}
	hd->orig_filename[hd->orig_filename_len] = '\0';
	
	ret = bit_read(my_bitio, 64, &buf);
	if (ret != 64) {
		fprintf(stderr, "Error in bit_read()\n");
		return NULL;
	}
	hd->orig_size = le64toh(buf);

	ret = bit_read(my_bitio, 64, &buf);
	if (ret != 64) {
		fprintf(stderr, "Error in bit_read()\n");
		return NULL;
	}
	hd->orig_creation_time = le64toh(buf);
	
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			fprintf(stderr, "Error in bit_read()\n");
			return NULL;
		}
		hd->checksum[i] = (unsigned char)buf;
	}
	
	if (hd->compressed == 1){
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			fprintf(stderr, "Error in bit_read()\n");
			return NULL;
		}
		hd->compr_alg = (uint8_t)buf;
		
		ret = bit_read(my_bitio, 32, &buf);
		if (ret != 32) {
			fprintf(stderr, "Error in bit_read()\n");
			return NULL;
		}
		hd->dict_size = le32toh((int)buf);
	}
	
	return hd;
}

int check_integrity(struct header* hd, FILE* file){
	unsigned char* computed_checksum;
	struct stat file_info;
	uint64_t size;
	int ret, fd;
	
	if (hd == NULL || file == NULL) {
		fprintf(stderr, "One of the passed parameters is NULL\n");
		return -1;
	}
	
	fseek(file, 0, SEEK_SET);
	
	fd = fileno(file);
	if (fd <= 0) {
		fprintf(stderr, "Error in fileno()\n");
		return -1;
	}
	
	if (fstat(fd, &file_info)) {
		fprintf(stderr, "Error in reading the file info\n");
		return -1;
	}
	size = (uint64_t)file_info.st_size;
	
	if (size != hd->orig_size) {
		fprintf(stderr, "Different file length\n");
		return -1;
	}
	
	computed_checksum = malloc(SHA256_DIGEST_LENGTH);
	ret = create_checksum(file, hd->orig_size, &computed_checksum);	
	if (ret == -1) {
		fprintf(stderr, "Error in creating the checksum...\n");
		return -1;
	}
	
	if (memcmp(computed_checksum, hd->checksum, SHA256_DIGEST_LENGTH) != 0){
	    fprintf(stderr, "Checksum verification failed\n");
	    return -1;
	}
	
	return 0;
}