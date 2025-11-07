CC = gcc
CFLAGS = -Wall -Iinclude -g
LDFLAGS = -lreadline
SRCS = src/main.c src/shell.c src/execute.c
OUT = bin/myshell

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRCS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT) $(LDFLAGS)

clean:
	rm -rf bin
