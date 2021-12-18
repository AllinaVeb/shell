#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *get_word(char *end, char c){
	char *word = NULL;
	int size = 0;
	while(1){
		if(c == ' ' || c == '\t' || c == '\n'){
			*end = c;
			if(size == 0){
				return NULL;
			}
			word[size] = '\0';
			return word;
		}
		word = realloc(word, (size + 2) * sizeof(char));
		word[size] = c;
		size++;
		c = getchar();
	}
}

char **get_list(char *end, int *sign_pipe, int *sign_and){
	char **list = NULL;
	int size = 0;
	while(1){
		char c = getchar();
		while(c == ' ' || c == '\t'){
			c = getchar();
		}
		list = realloc(list, (size + 1) * sizeof(char*));
		if(c == '|'){
                //      c = getchar();
                //      if(c == ' '){
                                *sign_pipe = 1;
                                break;
                //      }
                //      else{
//                              *sign_or = 1;
//                              break;
//                      }
                }
                if(c == '&'){
                        c = getchar();
                        if(c == '&'){
                                *sign_and = 1;
                                break;
//                      }
//                      else{
//                              *sign_phone = 1;
//                              break;
                        }
                }
		list[size] = get_word(end, c);
		if(list[0] == NULL){
			return NULL;
		}
		size++;
		if(*end == '\n'){
			list = realloc(list, (size + 1) * sizeof(char*));
			break;
		}
	}
	list[size] = NULL;
	return list;
}

char ***get_cmd(int *sign_pipe, int *sign_and){
	char end = 0, ***cmd = NULL;
	int size = 0;
	while(1){
		cmd = realloc(cmd, (size + 2) * sizeof(char**));
		cmd[size] = get_list(&end, sign_pipe, sign_and);
		if(cmd[0] == NULL){
			return NULL;
		}
		size++;
		if(end == '\n'){
			cmd[size] = NULL;
			return cmd;
		}
	}
}

void memclear(char ***cmd){
	for(int i = 0; cmd[i] != NULL; i++){
		for(int j = 0; cmd[i][j]; j++){
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

void conv_and(char ***cmd, int size){
	for(int i = 0; i <= size; i++){
		pid_t pid = fork();
		if(pid == 0){
			if(execvp(*cmd[i], cmd[i]) == -1){
				perror("incorrect command");
				exit(1);
			}
		}
		int wstatus;
		waitpid(pid, &wstatus, 0);
		if(WIFEXITED(wstatus == 0)){
			break;
		}
	}
}

int main(int argc, char **argv){
	char *home = getenv("HOME");
	int size;
	while(1){
		int sign_pipe = 0, sign_and = 0;
		char ***cmd = get_cmd(&sign_pipe, &sign_and);
		while(cmd == NULL){
			cmd = get_cmd(&sign_pipe, &sign_and);
		}
		if(strcmp(*cmd[0], "exit") == 0 || strcmp(*cmd[0], "quit") == 0){
			memclear(cmd);
			break;
		}
		if(strcmp(*cmd[0], "cd") == 0){
			change_dir(cmd, home);
		}
		for(size = 0; cmd[size]; size++){
			for(int j = 0; cmd[size][j]; j++){
				printf("cmd[i = %d][j = %d] = %s\n", size, j, cmd[size][j]);
			}
		}
		if(sign_and){
			conv_and(cmd, size);
		}
		else{
			if(fork() == 0){
				if(execvp(**cmd, *cmd) == -1){
                       			perror("incorrect command");
                       			exit(1);
               			}
			}
			wait(NULL);
		}
		putchar('\n');
		memclear(cmd);
	}
	return 0;
}
