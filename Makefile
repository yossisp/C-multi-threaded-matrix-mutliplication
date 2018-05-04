CC = gcc
CFLAGS = -Wall -pedantic
BIN = main *.o *~
OBJ = main.o


main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
clean:
	rm -f $(BIN)