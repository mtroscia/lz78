#include "header.h"

void print_bytes(const void* object, size_t size){
  const unsigned char* bytes = object;
  size_t i;

  for(i = 0; i < size; i++)
  {
    printf("%02x ", bytes[i]);
  }
}

int create_checksum(FILE* fd, int size, unsigned char** out) {	
	int ret, ret_r, i;
	SHA256_CTX* ctx;
	int buf_size = 128;
	void* in = malloc(buf_size);
	int num_blocks = size/buf_size + 1;
	
	//printf("Composing SHA256...\t");
	fseek(fd, 0, SEEK_SET);
	
	ctx = (SHA256_CTX*)malloc(sizeof(SHA256_CTX));

	ret = SHA256_Init(ctx);
	if (ret != 1) {
		printf("\nError in initializing the context...\n");
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
	
	print_bytes(out, SHA256_DIGEST_LENGTH);
	printf("\n");
	
	//printf("OK\n");
	
	return 0;
}

//testing
void print_header(struct header* hd){
	printf("hd->compressed %i\n", hd->compressed);
	printf("hd->orig_filename_len %i\n", hd->orig_filename_len);
	printf("hd->orig_filename %s\n", hd->orig_filename);
	printf("hd->orig_size %lu\n", hd->orig_size);
	printf("hd->orig_creation_time %lu\n", hd->orig_creation_time);
	printf("hd->checksum ");
	print_bytes(hd->checksum, SHA256_DIGEST_LENGTH);
	printf("\nhd->compr_alg %i\n", hd->compr_alg);
	printf("hd->dict_size %i\n", hd->dict_size);
}
	
struct header* generate_header(FILE* file, char* file_name, uint8_t alg, int d_size){
	struct header* hd;
	struct stat file_info;
	int ret, i;
	
	unsigned char* out;
	
	//printf("Composing the header...\n");
	
	hd = (struct header*)calloc(1, sizeof(struct header));
	if (hd == NULL){
		return NULL;
	}
	
	hd->compressed = 1;
	hd->orig_filename_len = strlen(file_name);
	hd->orig_filename = malloc(hd->orig_filename_len+1);	 //to include \0 
	strcpy(hd->orig_filename, file_name);
	
	if (stat(file_name, &file_info)) {
		printf("Error in reading the file info\n");
		free(hd);
		return NULL;
	}
	
	hd->orig_size = (uint64_t)file_info.st_size;
	hd->orig_creation_time = (uint64_t)file_info.st_ctime;
	
	out = malloc(SHA256_DIGEST_LENGTH);
	ret = create_checksum(file, hd->orig_size, &out);	
	if (ret == -1) {
		printf("Error in creating the checksum...\n");
		free(hd);
		return NULL;
	}
	memcpy(hd->checksum, out, SHA256_DIGEST_LENGTH);
	
	hd->compr_alg = alg;
	hd->dict_size = d_size;
	
	//print_header(hd);
	
	return hd;
}

int add_header(struct bitio* my_bitio, struct header* hd) {
	int ret, i;
	uint64_t temp64;
	uint32_t temp32;
	
	if (my_bitio==NULL || hd==NULL) {
		printf("Error\n");
		return -1;
	}
	
	//printf("Start writing into output file...\n");
	
	ret = bit_write(my_bitio, 8, (uint8_t)hd->compressed);
	if (ret != 0) {
		return -1;
	}
	print_bytes(&hd->compressed, sizeof(uint8_t));
	printf("\n");
	
	ret = bit_write(my_bitio, 8, (uint8_t)hd->orig_filename_len);
	if (ret != 0) {
		return -1;
	}
	print_bytes(&hd->orig_filename_len, sizeof(uint8_t));
	printf("\n");
	
	for (i=0; i<hd->orig_filename_len; i++) {
		ret = bit_write(my_bitio, 8, (char)(hd->orig_filename[i]));
		if (ret != 0) {
			return -1;
		}
	}
	print_bytes(hd->orig_filename, hd->orig_filename_len);
	printf("\n");
	
	temp64 = htole64((uint64_t)hd->orig_size);
	print_bytes((void*)&temp64, sizeof(uint64_t));
	printf("\n");
	ret = bit_write(my_bitio, 64, temp64);
	if (ret != 0) {
		return -1;
	}
	
	temp64 = htole64((uint64_t)hd->orig_creation_time);
	print_bytes((void*)&temp64, sizeof(uint64_t));
	printf("\n");
	ret = bit_write(my_bitio, 64, temp64);
	if (ret != 0) {
		return -1;
	}
	
	for (i=0; i<SHA256_DIGEST_LENGTH; i++) {
		ret = bit_write(my_bitio, 8, (unsigned char)(hd->checksum[i]));
		if (ret != 0) {
			return -1;
		}
	}
	print_bytes(hd->checksum, SHA256_DIGEST_LENGTH);
	printf("\n");
	
	ret = bit_write(my_bitio, 8, (uint8_t)(hd->compr_alg));
	if (ret != 0) {
		return -1;
	}
	print_bytes(&hd->compr_alg, sizeof(uint8_t));
	printf("\n");
	
	temp32 = htole32((int)hd->dict_size);
	print_bytes((void*)&temp32, sizeof(int));
	printf("\n");
	ret = bit_write(my_bitio, 32, temp32);
	if (ret != 0) {
		return -1;
	}
	
	printf("Header has been written\n");
	return 0;
}

struct header* get_header(struct bitio* my_bitio){
	int ret, i;
	uint64_t buf = 0, temp;
	struct header* hd;
	
	if (my_bitio==NULL) {
		printf("Error\n");
		return NULL;
	}
	
	hd = (struct header*)calloc(1, sizeof(struct header));
	if (hd == NULL){
		return NULL;
	}
	
	printf("Start reading from input file...\n");
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		return NULL;
	}
	hd->compressed = (uint8_t)buf;
	print_bytes(&hd->compressed, sizeof(uint8_t));
	printf("\n");
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		return NULL;
	}
	hd->orig_filename_len = (uint8_t)buf;
	print_bytes(&hd->orig_filename_len, sizeof(uint8_t));
	printf("\n");
	
	hd->orig_filename = malloc(hd->orig_filename_len+1);	//to consider /0
	for (i=0; i<hd->orig_filename_len; i++) {
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			return NULL;
		}
		hd->orig_filename[i] = (char)buf;
	}
	hd->orig_filename[hd->orig_filename_len] = '\0';
	print_bytes(hd->orig_filename, hd->orig_filename_len);
	printf("\n");
	
	ret = bit_read(my_bitio, 64, &buf);
	if (ret != 64) {
		return NULL;
	}
	hd->orig_size = le64toh(buf);
	print_bytes(&(hd->orig_size), sizeof(uint64_t));
	printf("\n");

	ret = bit_read(my_bitio, 64, &buf);
	if (ret != 64) {
		return NULL;
	}
	hd->orig_creation_time = le64toh(buf);
	print_bytes(&(hd->orig_creation_time), sizeof(uint64_t));
	printf("\n");
	
	for (i=0; i<SHA256_DIGEST_LENGTH; i++) {
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			return NULL;
		}
		hd->checksum[i] = (unsigned char)buf;
	}
	print_bytes(hd->checksum, SHA256_DIGEST_LENGTH);
	printf("\n");
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		return NULL;
	}
	hd->compr_alg = (uint8_t)buf;
	print_bytes(&hd->compr_alg, sizeof(uint8_t));
	printf("\n");
	
	ret = bit_read(my_bitio, sizeof(int)*8, &buf);
	if (ret != sizeof(int)*8) {
		return NULL;
	}
	hd->dict_size = le32toh((int)buf);
	print_bytes(&hd->dict_size, sizeof(int));
	printf("\n");
	
	printf("Header has been read\n");
	
	//print_header(hd);
	
	return hd;
}

int check_integrity(struct header* hd, FILE* file){
	unsigned char* computed_checksum;
	int ret;
	
	computed_checksum = malloc(SHA256_DIGEST_LENGTH);
	ret = create_checksum(file, hd->orig_size, &computed_checksum);	
	if (ret == -1) {
		printf("Error in creating the checksum...\n");
		free(hd);
		return -1;
	}
	
	if (strcmp(computed_checksum, hd->checksum) != 0){
		printf("Checksum verification failed\n");
		return -1;
	}
	
	return 0;
}