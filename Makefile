CC=gcc
CFLAGS=-Wall -g
TARGET=myshell
SRCS=src/main.c src/shell.c src/execute.c
OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
