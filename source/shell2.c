#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int son_pid = -1;

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

char **get_list(char *end, int *sign_pipe, int *sign_and, int *sign_or, char *files[]){
	char **list = NULL;
	int size = 0;
	while(1){
		char c = getchar();
		while(c == ' ' || c == '\t'){
			c = getchar();
		}
		list = realloc(list, (size + 1) * sizeof(char*));
		if(c == '<' || c == '>'){
			if(c == '<'){
				c = getchar();
				while(c == ' ' || c == '\t'){
					c = getchar();
				}
				files[0] = get_word(end, c);
			}else{
				c = getchar();
				while(c == ' ' || c == '\t'){
					c = getchar();
				}
				files[1] = get_word(end, c);
			}
			if(*end == '\n'){
				break;
			}
			continue;
		}
		if(c == '|'){
			c = getchar();
			if(c == ' '){
				*sign_pipe = 1;
				break;
			}else{
				*sign_or = 1;
				break;
			}
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

char ***get_cmd(int *sign_pipe, int *sign_and, int *sign_or, char *files[]){
	char end = 0, ***cmd = NULL;
	int size = 0;
	while(1){
		cmd = realloc(cmd, (size + 2) * sizeof(char**));
		cmd[size] = get_list(&end, sign_pipe, sign_and, sign_or, files);
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
			if(execvp(*cmd[i], cmd[i]) < 0){
				perror("incorrect command");
				exit(1);
			}
		}
		int wstatus;
		waitpid(pid, &wstatus, -1);
		if(WIFEXITED(wstatus != 0)){
			break;
		}
	}
}

void conv_or(char ***cmd, int size){
	for(int i = 0; i <= size; i++){
		pid_t pid = fork();
		if(pid == 0){
			if(execvp(*cmd[i], cmd[i]) < 0){
				perror("incorrect command");
				exit(1);
			}
		}
		int wstatus;
		waitpid(pid, &wstatus, 0);
		if(WIFEXITED(wstatus != 0)){
			break;
		}
	}
}

void handler(int signo){
	puts("received SIGINT");
	kill(son_pid, SIGKILL);
}


int main(int argc, char **argv){
	char *home = getenv("HOME");
//	signal(SIGINT, handler);
	int size;
	while(1){
		char *files[] = {NULL, NULL};
		int sign_pipe = 0, sign_and = 0, sign_or = 0;
		char ***cmd = get_cmd(&sign_pipe, &sign_and, &sign_or, files);
		while(cmd == NULL){
			cmd = get_cmd(&sign_pipe, &sign_and, &sign_or, files);
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
		printf("files[0] = %s, files[1] = %s\n", files[0], files[1]);
		if(sign_and){
			conv_and(cmd, size);
		}
		if(sign_or){
			conv_or(cmd, size);
		}
		else{
			if(fork() == 0){
				if(execvp(**cmd, *cmd) < 0){
                       			perror("incorrect command");
                       			exit(1);
               			}
			}
			wait(NULL);
		}
		putchar('\n');
		free(files[0]);
		free(files[1]);
		memclear(cmd);
	}
	return 0;
}
