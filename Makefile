CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lreadline

OBJS = main.o execute.o shell.o

all: shell

shell: $(OBJS)
	$(CC) $(CFLAGS) -o shell $(OBJS) $(LDFLAGS)

main.o: main.c shell.h
	$(CC) $(CFLAGS) -c main.c

execute.o: execute.c shell.h
	$(CC) $(CFLAGS) -c execute.c

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o shell
