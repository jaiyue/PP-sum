CC = gcc
CFLAGS = -Wall -g 
TARGET = runme
LIBRARY = libFilmMaster2000.a
INPUT = test.bin
OUTPUTS = breverse.bin bscale.bin bclip.bin bswap.bin

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
	./$(TARGET) $(INPUT) breverse.bin -S reverse
	./$(TARGET) $(INPUT) bswap.bin -S swap_channel 0,2
	./$(TARGET) $(INPUT) bclip.bin -M clip_channel 1 [10,200]
	./$(TARGET) $(INPUT) bscale.bin -S scale_channel 1 1.5

	@echo All tests completed.
clean:
	@echo Cleaning up...
	rm -f *.o $(TARGET) $(LIBRARY) $(OUTPUTS)
	@echo Clean done.
