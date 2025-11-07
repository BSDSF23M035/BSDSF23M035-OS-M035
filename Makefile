CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/shell.c src/execute.c
BIN = bin/myshell

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

clean:
	rm -rf bin
