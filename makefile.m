CC = gcc
CFLAGS = -Wall -f.sanitize=address

all: shell

shell: shell.c
	$(CC) shell.c -o shell $(CFLAGS)
