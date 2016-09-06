lz78: bitio.c compressor.c decompressor.c header.c lz78.c portable_endian.h
	gcc -g -lcrypto bitio.c compressor.c decompressor.c header.c lz78.c -o lz78