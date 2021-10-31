#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DEF_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH 

char *get_word (char *end){
        int counter = 0, bytes, sing = 0;
        char c, *word = NULL;
	c = getchar();
	while(c == ' ' || c == '\t'){
		c = getchar();
	}
/*	if(c == '<' || c == '>'){
		if(c == '>'){
			sing++;
		}else{
			sing--;
		}
		c = gethar();
	}
*/
        while(1){
		bytes = (counter + 1) * sizeof(char);
                word = realloc(word, bytes);
                if(c == ' ' || c == '\t' || c == '\n'){
			*end = c;
			word[counter] = '\0';
			return word;
                }
                else {
                        word[counter] = c;
                        counter++;
                        c = getchar();
                }
        }
}

int check_end(char *string){
        int ans1, ans2;
        ans1 = strcmp(string, "exit");
        ans2 = strcmp(string, "quit");
        if(ans1 == 0 || ans2 == 0){
                return 1;
        }
        else{
                return 0;
        }
}

void launch(char **command){
	if(fork() == 0){
		if(execvp(*command, command) == -1){
			perror("incorrect command");
			exit(1);
		}
	}
	wait(NULL);
	
}

char **get_list(){
	char end = 0, **string = NULL;
	int size = 0, bytes;
        while(end != '\n'){
		bytes = (size + 2) * sizeof(char*);
		string = realloc(string, bytes);
		string[size] = get_word(&end);
		size++;
        }
	if(string[size - 1] != NULL){
		string[size] = NULL;
	}else{
		free(string[size]);
	}
	return string;
}

void memclear(char **list) {
	int i;
	for(i = 0; list[i] != NULL; i++) {
		free(list[i]);
	}
	free(list);
}

int main(int argc, char **argv) {
	 while(1){
		char **list = get_list();
                if(check_end(*list)){
                        break;
                }
		launch(list);
        	putchar('\n');
		memclear(list);
	 }
        return 0;
}

