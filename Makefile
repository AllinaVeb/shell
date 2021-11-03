CFLAGS=-Wall -fsanitize=address

shell:  source/shell.c
	gcc source/shell.c -o bin/shell $(CFLAGS)
