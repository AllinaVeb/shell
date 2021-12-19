#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DEF_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

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

int file_in(char *files){
	if(files){
		int fd = open(files, O_RDONLY, S_IROTH);
		if(fd < 0){
			return -1;
		}
		return 1;
	}
	return 0;
}

int file_out(char *files){
	if(files){
		int fd = open(files, O_WRONLY | O_CREAT, DEF_MODE);
		if(fd < 0){
			return -1;
		}
	}
	return 1;
}

void launch(char **cmd, int fd_in, int fd_out){
	if(fork() == 0){
/*		if(fd_in != 0){
			dup2(fd_in, 0);
			close(fd_in);
		}
		if(fd_out != 1){
			dup2(fd_out, 1);
			close(fd_out);
		}
		*/
		if(execvp(cmd[0], cmd) < 0){
			perror("incorrect command");
			exit(1);
		}
	}
}

void conv_pipe(char ***cmd, int size, char *files[]){
	printf("we are in fun conv_pipe\n");
	int pipefd[size + 1][2], i;
	pipefd[0][0] = file_in(files[0]);
	pipefd[size][1] = file_out(files[1]);
	printf("pipefd[0][0] = %d, pipefde[%d][1] = %d", pipefd[0][0], size, pipefd[size][1]);
	for(i = 0; i < size; i++){
		if((i + 1) != size){
			pipe(pipefd[i]);
		}
		launch(cmd[i], pipefd[i][0], pipefd[i][1]);
		if(pipefd[i][0] != 0){
			close(pipefd[i][0]);
		}
		if(pipefd[i][1] != 1){
			close(pipefd[i][1]);
		}
		wait(NULL);
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
		for(size = 0; cmd[size]; size++){
			for(int j = 0; cmd[size][j]; j++){
				printf("cmd[i = %d][j = %d] = %s\n", size, j, cmd[size][j]);
			}
		}
		printf("files[0] = %s, files[1] = %s\n", files[0], files[1]);
		if(strcmp(*cmd[0], "cd") == 0){
			change_dir(cmd, home);
		}else{
			if(sign_and){
				conv_and(cmd, size);
			}
			if(sign_or){
				conv_or(cmd, size);
			}
			if(sign_pipe){
				conv_pipe(cmd, size, files);
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
		}
		putchar('\n');
		free(files[0]);
		free(files[1]);
		memclear(cmd);
	}
	return 0;
}
