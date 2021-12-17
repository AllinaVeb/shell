#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *get_word(char *end, int *sign_pipe){
	char *word = NULL;
	int size = 0;
	char c = getchar();
	while(c == ' ' || c == '\t'){
		c = getchar();
	}
	while(1){
		if(c == ' ' || c == '\t' || c == '\n'){
			*end = c;
			if(size == 0){
				return NULL;
			}
			word[size] = '\0';
			return word;
		}
		if(c == '|'){
		//	c = getchar();
		//	if(c == ' '){
				*sign_pipe = 1;
				return NULL;
		//	}
		//	else{
//				sign_or = 1;
//			}
		}
		word = realloc(word, (size + 2) * sizeof(char));
		word[size] = c;
		size++;
		c = getchar();
	}
}

char **get_list(char *end, int *sign_pipe, int *num_cmd){
	char **list = NULL;
	int size = 0;
	while(1){
		list = realloc(list, (size + 2) * sizeof(char*));
		list[size] = get_word(end, sign_pipe);
		if(list[0] == NULL){
			return NULL;
		}
		size++;
		if(*end == '\n'){
			list[size] = NULL;
			*num_cmd = size;
			return list;
		}
	}
}

char ***get_cmd(int *num_cmd){
	char end = 0, ***cmd = NULL;
	int size = 0, sign_pipe = 0;
	while(1){
		cmd = realloc(cmd, (size + 2) * sizeof(char**));
		cmd[size] = get_list(&end, &sign_pipe, num_cmd);
		if(cmd[0] == NULL){
			return NULL;
		}
		if(sign_pipe){
			printf("we are in get_cmd, sign_pipe = 1\n");
		}
		size++;
		if(end == '\n'){
			cmd[size] = NULL;
			return cmd;
		}
	}
}

void memclear(char ***cmd, int size){
	for(int i = 0; cmd[i] != NULL; i++){
		for(int j = 0; j < size; j++){
		free(cmd[i][j]);
		}
		free(cmd[i]);
	}
	free(cmd);
}

void change_dir(char ***cmd, char *home){
	char a[100];
	if(cmd[0][1] == NULL || strcmp(cmd[0][1], "~") == 0){
		chdir(home);
	}else{
		chdir(cmd[0][1]);
	}
	printf("%s\n", getcwd(a,100));
}


int main(int argc, char **argv){
	char *home = getenv("HOME");
	while(1){
		int size = 0;
		char ***cmd = get_cmd(&size);
		while(cmd == NULL){
			cmd = get_cmd(&size);
		}
		if(strcmp(*cmd[0], "exit") == 0 || strcmp(*cmd[0], "quit") == 0){
			memclear(cmd, size);
			break;
		}
		if(strcmp(*cmd[0], "cd") == 0){
			change_dir(cmd, home);
		}
		for(int i = 0; cmd[i]; i++){
			for(int j = 0; j < size; j++){
				printf("cmd[i = %d][j = %d] = %s\n", i, j, cmd[i][j]);
			}
		}	
		putchar('\n');
		memclear(cmd, size);
	}
	return 0;
}
