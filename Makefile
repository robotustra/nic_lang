# example http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
CC=gcc
CFLAGS=-I.
DEPS = common.h sh_inner.h
OBJ = sh_inner.o shell.o main.o

LIBS = -ldl -lm



%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

nic: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:

	rm *.o nic
