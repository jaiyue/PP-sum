CC = gcc
CFLAGS = -Wall -g
target = runme 
library = libFilmMaster2000.a
input = test.bin
output = output.bin

all: $(target) $(library)

$(target): main.o libFilmMaster2000.a
	$(CC) $(CFLAGS) $^ -o $@ -L. -lFilmMaster2000

$(library): func.o
	ar rcs libFilmMaster2000.a func.o
func.o: func.c func.h
	$(CC) $(CFLAGS) -c $< -o $@
main.o: main.c
	$(CC) $(CFLAGS) -c $^ -o $@

test: $(target) 
	./$(target) $(input) $(output)
	@echo output file: $(output)

clean:
	rm -f *.o $(target) $(library) $(output)
