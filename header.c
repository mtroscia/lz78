#include "header.h"

void print_bytes(const void* object, size_t size){
  const unsigned char* bytes = object;
  size_t i;

  for(i=0; i<size; i++)
  {
    printf("%02x ", bytes[i]);
  }
  printf("\n");
}

int create_checksum(FILE* fd, int size, unsigned char** out) {	
	int ret, ret_r, i;
	SHA256_CTX* ctx;
	int buf_size = 128;
	void* in = malloc(buf_size);
	int num_blocks = size/buf_size + 1;
	
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
			ret = SHA256_Update(ctx, in, ret_r);
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
	
	return 0;
}

//testing
void print_header(struct header* hd){
	printf("hd->compressed\t\t %i\n", hd->compressed);
	printf("hd->orig_filename_len\t %i\n", hd->orig_filename_len);
	printf("hd->orig_filename\t %s\n", hd->orig_filename);
	printf("hd->orig_size\t\t %lu\n", hd->orig_size);
	printf("hd->orig_creation_time\t %lu\n", hd->orig_creation_time);
	printf("hd->checksum\t\t ");
	print_bytes(hd->checksum, SHA256_DIGEST_LENGTH);
	printf("hd->compr_alg\t\t %i\n", hd->compr_alg);
	printf("hd->dict_size\t\t %i\n", hd->dict_size);
}
	
struct header* generate_header(FILE* file, char* file_name, uint8_t alg, int d_size){
	struct header* hd;
	struct stat file_info;
	int ret, i;
	unsigned char* out;
	
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
	
	print_header(hd);
	
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
	
	temp64 = htole64((uint64_t)hd->orig_size);
	ret = bit_write(my_bitio, 64, temp64);
	if (ret != 0) {
		return -1;
	}
	
	temp64 = htole64((uint64_t)hd->orig_creation_time);
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
	
	ret = bit_write(my_bitio, 8, (uint8_t)(hd->compr_alg));
	if (ret != 0) {
		return -1;
	}
	
	temp32 = htole32((int)hd->dict_size);
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
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		return NULL;
	}
	hd->orig_filename_len = (uint8_t)buf;
	
	hd->orig_filename = malloc(hd->orig_filename_len+1);	//to consider /0
	for (i=0; i<hd->orig_filename_len; i++) {
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			return NULL;
		}
		hd->orig_filename[i] = (char)buf;
	}
	hd->orig_filename[hd->orig_filename_len] = '\0';
	
	ret = bit_read(my_bitio, 64, &buf);
	if (ret != 64) {
		return NULL;
	}
	hd->orig_size = le64toh(buf);

	ret = bit_read(my_bitio, 64, &buf);
	if (ret != 64) {
		return NULL;
	}
	hd->orig_creation_time = le64toh(buf);
	
	for (i=0; i<SHA256_DIGEST_LENGTH; i++) {
		ret = bit_read(my_bitio, 8, &buf);
		if (ret != 8) {
			return NULL;
		}
		hd->checksum[i] = (unsigned char)buf;
	}
	
	ret = bit_read(my_bitio, 8, &buf);
	if (ret != 8) {
		return NULL;
	}
	hd->compr_alg = (uint8_t)buf;
	
	ret = bit_read(my_bitio, sizeof(int)*8, &buf);
	if (ret != sizeof(int)*8) {
		return NULL;
	}
	hd->dict_size = le32toh((int)buf);
	
	printf("Header has been read\n");
	print_header(hd);
	
	return hd;
}

int check_integrity(struct header* hd, FILE* file){
	unsigned char* computed_checksum;
	struct stat file_info;
	uint64_t size;
	int ret;
	int fd;
	
	fd = fileno(file);
	if (fd <= 0) {
		printf("Error in fd...\n");
		return -1;
	}
	
	if (fstat(fd, &file_info)) {
		printf("Error in reading the file info\n");
		return -1;
	}
	size = (uint64_t)file_info.st_size;
	printf("size %lu received %lu\n", size, hd->orig_size);
	if (size != hd->orig_size) {
		printf("Different file length\n");
		return -1;
	}
	
	computed_checksum = malloc(SHA256_DIGEST_LENGTH);
	ret = create_checksum(file, size, &computed_checksum);	
	if (ret == -1) {
		printf("Error in creating the checksum...\n");
		return -1;
	}
	printf("Received header ");
	print_bytes(hd->checksum, SHA256_DIGEST_LENGTH);
	printf("Computed header ");
	print_bytes(computed_checksum, SHA256_DIGEST_LENGTH);
	
	if (memcmp(computed_checksum, hd->checksum, SHA256_DIGEST_LENGTH) != 0){
	    printf("Checksum verification failed\n");
	    return -1;
	  }
	
	return 0;
}