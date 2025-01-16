CC = gcc
CFLAGS = -Wall -g -fopenmp
TARGET = runme
LIBRARY = libFilmMaster2000.a
INPUT = test.bin
OUTPUTS = breverse.bin bscale.bin bclip.bin bswap.bin creverse.bin cswap.bin cclip.bin cscale.bin areverse.bin aswap.bin aclip.bin ascale.bin

.PHONY: all test clean

all: $(TARGET)

$(TARGET): main.o $(LIBRARY)
	$(CC) $(CFLAGS) main.o -o $(TARGET) -L. -lFilmMaster2000

$(LIBRARY): func.o
	ar rcs $(LIBRARY) func.o

func.o: func.c func.h
	$(CC) $(CFLAGS) -c func.c -o func.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

test: $(TARGET)
	@echo Running tests...
	./$(TARGET) $(INPUT) areverse.bin reverse
	./$(TARGET) $(INPUT) aswap.bin swap_channel 0,2
	./$(TARGET) $(INPUT) aclip.bin clip_channel 1 [10,200]
	./$(TARGET) $(INPUT) ascale.bin scale_channel 1 1.5
	./$(TARGET) $(INPUT) breverse.bin -S reverse
	./$(TARGET) $(INPUT) bswap.bin -S swap_channel 0,2
	./$(TARGET) $(INPUT) bclip.bin -S clip_channel 1 [10,200]
	./$(TARGET) $(INPUT) bscale.bin -S scale_channel 1 1.5
	./$(TARGET) $(INPUT) creverse.bin -M reverse
	./$(TARGET) $(INPUT) cswap.bin -M swap_channel 0,2
	./$(TARGET) $(INPUT) cclip.bin -M clip_channel 1 [10,200]
	./$(TARGET) $(INPUT) cscale.bin -M scale_channel 1 1.5
	
	@echo All tests completed.
clean:
	@echo Cleaning up...
	rm -f *.o $(TARGET) $(LIBRARY) $(OUTPUTS)
	@echo Clean done.
