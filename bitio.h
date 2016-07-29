#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

struct bitio;

struct bitio* bit_open (const char* name, uint32_t mode);
int bit_close (struct bitio* );
int bit_write (struct bitio*, uint32_t size, uint64_t data);
int bit_read (struct bitio*, uint32_t size, uint64_t* data);
