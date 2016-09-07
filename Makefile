lz78: bitio.c compressor.c decompressor.c header.c lz78.c portable_endian.h
	gcc bitio.c compressor.c decompressor.c header.c lz78.c -g -lcrypto  -o lz78