CC = cc
CFLAGS = -c -Wall
LDFLAGS = -lcrypto

SOURCES = bitio.c compressor.c decompressor.c header.c lz78.c
OBJECTS = $(SOURCES:.c=.o)

EXECUTABLE = lz78
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	
.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
bitio.o: bitio.h
decompressor.o: decompressor.h bitio.h
compressor.o: compressor.h bitio.h
header.o: header.h bitio.h portable_endian.h
lz78.o: bitio.h compressor.h decompressor.h header.h lz78.c
	
clean:
	rm $(OBJECTS) $(EXECUTABLE)
