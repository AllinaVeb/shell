CFLAGS=-Wall -fsanitize=address,leak

shell:  source/shell.c
	gcc source/shell.c -o bin/shell $(CFLAGS)
