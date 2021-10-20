#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *get_word (char *end){
        int counter = 0, bytes;
        char c, *word = NULL;
	c = getchar();
        while(1){
                if(c == ' ' || c == '\t' || c == '\n'){
			bytes = (counter + 1) * sizeof(char);
                        word = realloc(word, bytes);
                        *end = c;
                        word[counter] = '\0';
                        return word;
                }
                else {
			bytes = (counter + 1) * sizeof(char);
			word = realloc(word, bytes);
                        word[counter] = c;
                        counter++;
                        c = getchar();
                }
        }
}

char **get_list(){
	char end, **string = NULL;
	int size = 0, bytes;
        while(end != '\n'){
		bytes = (size + 1) * sizeof(char*);
		string = realloc(string, bytes);
		string[size] = get_word(&end);
		size++;
        }
	bytes = (size + 1) * sizeof(char*);
	string = realloc(string, bytes);
	string[size] = '\0';
	int i = 0;
	while(string[i] != NULL){
		fputs(string[i], stdout);
		putchar(' ');
		i++;
	}
	putchar('\n');
	return string;

}

void memclear(char **list) {
	int i;
	for(i = 0; list[i] != NULL; i++) {
		free(list[i]);
	}
	free(list[i]);
	free(list);
}

int main(int argc, char **argv) {
	char **list = get_list();
	memclear(list);
        return 0;
}
