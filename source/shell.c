#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DEF_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

pid_t *son_pids = NULL;
int proc_num = 0;

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

char **get_list(char *end, int *sign_pipe, int *sign_and, int *sign_or, char *files[], int *sign_phone){
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
                      }
                      else{
                              *sign_phone = 1;
			      c = getchar();
			      if(c == ' '){
				      break;
			      }else if(c == '\n'){
				      *end = '\n';
			      }
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

char ***get_cmd(int *sign_pipe, int *sign_and, int *sign_or, char *files[], int *sign_phone){
	char end = 0, ***cmd = NULL;
	int size = 0;
	while(1){
		cmd = realloc(cmd, (size + 2) * sizeof(char**));
		cmd[size] = get_list(&end, sign_pipe, sign_and, sign_or, files, sign_phone);
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
	free(son_pids);
	son_pids = NULL;
	proc_num = 0;
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
	for(int i = 0; i < size; i++){
		son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
		son_pids[i] = fork();
		if(son_pids[i] == 0){
			if(execvp(*cmd[i], cmd[i]) < 0){
				perror("incorrect command");
				exit(1);
			}
		}
		int wstatus;
		waitpid(son_pids[i], &wstatus, 0);
		if(WIFEXITED(wstatus == 0)){
			break;
		}
		proc_num++;
	}
}

void conv_or(char ***cmd, int size){
	for(int i = 0; i < size; i++){
		son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
		son_pids[i] = fork();
		if(son_pids[i] == 0){
			if(execvp(*cmd[i], cmd[i]) < 0){
				perror("incorrect command");
				exit(1);
			}
		}
		int wstatus;
		waitpid(son_pids[i], &wstatus, 0);
		if(WIFEXITED(wstatus != 0)){
			break;
		}
		proc_num++;
	}
}

int *conv_pipe(char ***cmd, int size, char *files[], int sign_phone){
	son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
	int fd[size - 1][2], i = 0;	
	pipe(fd[i]);
	if(files[0] != NULL){
		int file_fd = open(files[0], O_RDONLY, S_IROTH);
		if(file_fd < 0){
			return NULL;
		}
		son_pids[i] = fork();
		if(son_pids[i] == 0){
			dup2(file_fd, 0);
			close(file_fd);
			dup2(fd[i][1], 1);
			close(fd[i][1]);
			close(fd[i][0]);
			if(execvp(*cmd[i], *cmd) < 0){
                               perror("incorrect command");
                               exit(1);
                        }
		}
		close(file_fd);
	}
	else{
		son_pids[i] = fork();
		if(son_pids[i] == 0){
			dup2(fd[i][1], 1);
			close(fd[i][1]);
			close(fd[i][0]);
			if(execvp(*cmd[i], *cmd) < 0){
                 	       perror("incorrect command");
			       exit(1);
                	}
		}
	}
	close(fd[i][1]);
	proc_num++;
	for(i = 1; i + 1 < size; i++){
		son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
		pipe(fd[i]);
		son_pids[i] = fork();
		if(son_pids[i] == 0){
			dup2(fd[i - 1][0], 0);
			close(fd[i - 1][0]);
			dup2(fd[i][1], 1);
			close(fd[i][1]);
			close(fd[i][0]);
			if(execvp(*cmd[i], *cmd) < 0){
                               perror("incorrect command");
                               exit(1);
                        }
		}
		close(fd[i - 1][0]);
		close(fd[i][1]);
		proc_num++;
	}
	son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
	if(files[1] != NULL){
		int file_fd = open(files[1], O_WRONLY | O_CREAT, DEF_MODE);
			if(file_fd < 0){
				return NULL;
			}
			son_pids[i] = fork();
			if(son_pids[i] == 0){
				dup2(fd[i - 1][0],0);
				close(fd[i - 1][0]);
				dup2(file_fd, 1);
				close(file_fd);
				if(execvp(*cmd[i], *cmd) < 0){
					perror("incorrect command");
					exit(1);
            			}
			}
			close(file_fd);
	}
	else{
		son_pids[i] = fork();
	       	if(son_pids[i] == 0){
			dup2(fd[i - 1][0], 0);
			close(fd[i - 1][0]);
			if(execvp(*cmd[i], *cmd) < 0){
                               perror("incorrect command");
                               exit(1);
                        }

		}
	}
	close(fd[i - 1][0]);	
	proc_num++;
	if(sign_phone){
		return NULL;
	}
	for(i = 0; i < size; i++){
		wait(NULL);
	}	
	return NULL;
}


void handler(int signo){
	puts("received SIGINT");
	for(int i = 0; i > proc_num; i++){
		if(son_pids[i]){	
			kill(son_pids[i], SIGKILL);
		}
		waitpid(son_pids[i], NULL, 0);
		son_pids[i] = 0;
	}
}

int forwarding(char ***cmd, char *files[]){
	son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
	if(files[0] != NULL && files[1] != NULL){
		int filefd0 = open(files[0], O_RDONLY, S_IROTH);
		int filefd1 = open(files[1], O_WRONLY | O_CREAT, DEF_MODE);
		if(filefd0 < 0 || filefd1 < 0){
			return 0;
		}
		son_pids[0] = fork();
		if(son_pids[0] == 0){
			dup2(filefd0, 0);
			dup2(filefd1, 1);
			close(filefd0);
			close(filefd1);
			if(execvp(*cmd[0], *cmd) < 0){
                                        perror("incorrect command");
                                        exit(1);
                                }
		}
		close(filefd0);
		close(filefd1);
	}
	else{
		if(files[0] != NULL){
			int file_fd = open(files[0], O_RDONLY, S_IROTH);
			if(file_fd < 0){
				return 0;
			}
			son_pids[0] = fork();
			if(son_pids[0] == 0){
				dup2(file_fd, 0);
				close(file_fd);
				if(execvp(*cmd[0], *cmd) < 0){
                                        perror("incorrect command");
                                        exit(1);
                                }
			}
			close(file_fd);
		}
		if(files[1] != NULL){
			int file_fd = open(files[1], O_WRONLY | O_CREAT, DEF_MODE);
			if(file_fd < 0){
				return 0;
			}
			son_pids[0] = fork();
			if(son_pids[0] == 0){
				dup2(file_fd, 1);
				close(file_fd);
				if(execvp(*cmd[0], *cmd) < 0){
		                        perror("incorrect command");
                		        exit(1);
				}
			}
			close(file_fd);
		}
	}
	proc_num++;
	wait(NULL);
	return 0;
}

int main(int argc, char **argv){
	char *home = getenv("HOME");
	signal(SIGINT, handler);
	int size;
	while(1){
		char *files[] = {NULL, NULL};
		int sign_pipe = 0, sign_and = 0, sign_or = 0, sign_phone = 0;
		char ***cmd = get_cmd(&sign_pipe, &sign_and, &sign_or, files, &sign_phone);
		while(cmd == NULL){
			cmd = get_cmd(&sign_pipe, &sign_and, &sign_or, files, &sign_phone);
		}
		if(strcmp(*cmd[0], "exit") == 0 || strcmp(*cmd[0], "quit") == 0){
			memclear(cmd);
			break;
		}
		for(size = 0; cmd[size]; size++){
			for(int j = 0; cmd[size][j]; j++){
			}
		}
		if(strcmp(*cmd[0], "cd") == 0){
			change_dir(cmd, home);
		}
		else{
			if(sign_and){
				conv_and(cmd, size);
			}
			else if(sign_or){
				conv_or(cmd, size);
				}
			else if(sign_pipe){
				conv_pipe(cmd, size, files, sign_phone);
				if(son_pids != NULL){
					for(int i = 0; i < size; i++){
						if(son_pids[i]){
							waitpid(son_pids[i], NULL, 0);
						}
					}
					proc_num = 0;
					free(son_pids);
				}
			}
			else if(files[0] || files[1]){
				forwarding(cmd, files);
			}
			else{	
				son_pids = realloc(son_pids, (proc_num + 1) * sizeof(pid_t));
				son_pids[0] = fork();
				if(son_pids[0] == 0){
					if(execvp(*cmd[0], cmd[0]) < 0){
                                		perror("incorrect command");
                                		exit(1);
					}
				}
				waitpid(son_pids[0], NULL, 0);
				son_pids[0] = 0;
			}
		}
		putchar('\n');
		free(files[0]);
		free(files[1]);
		memclear(cmd);
	}
	return 0;
}
