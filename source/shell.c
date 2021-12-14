#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DEF_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH 

char *get_word (char *end, char c){
        int counter = 0, bytes;
        char *word = NULL;
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
	pid_t pid = fork();
	if(pid == 0){
		if(execvp(*command, command) == -1){
			perror("incorrect command");
			exit(1);
		}
	}
	int wstatus;
	wait(&wstatus);
//	printf("%d\n", WEXITSTATUS(wstatus));
//	wait(NULL);
}

char **get_list(char *end, int *sign, int *sign_pipe, int *sign_conv){
	char c, **string = NULL;
	int size = 0, bytes;
        while(*end != '\n'){
		c = getchar();
		while(c == ' ' || c == '\t'){
			c = getchar();
		}
		if(c == '\n'){
			break;
		}
		bytes = (size + 2) * sizeof(char*);
		string = realloc(string, bytes);
        	if(c == '<' || c == '>'){
			c = getchar();
               		if(c == '>'){
                       		*sign = 1;
                	}else{
                        	*sign = -1;
                	}
       		}
		if(c == '|'){
			c = getchar();
			if(c == '|'){
				*sign_conv = -1; // command or ||
			}else{
				*sign_pipe = 1;
			}
		}
		if(c == '&'){
			c = getchar();
			if(c == '&'){
				*sign_conv = 1; // and &&
			}else{
				//phone mode
			}
		}
		if(*sign == 0 && *sign_pipe == 0 && *sign_conv == 0){
			string[size] = get_word(end, c);
			if(string[size] == NULL){
				return 0;
			}
			size++;
		}else{
			break;
		}
        }
	string[size] = NULL;
	return string;
}

char ***get_cmd(){
	char ***cmd = NULL, end = 0;
	int sign_pipe = 0, sign_conv = 0, sign = 0, size = 0;
	while(1){
		cmd = realloc(cmd, (size + 2) * sizeof(char**));
		cmd[size] = get_list(&end, &sign, &sign_pipe, &sign_conv);
		if(end == '\n'){
			break;
		}
		size++;
	}
	cmd[size] = NULL;
	return cmd;
}	

void memclear(char **list) {
	int i;
	for(i = 0; list[i] != NULL; i++) {
		free(list[i]);
	}
	free(list);
}

/*int forwarding(int sign, char **list){
	int sign2 = 0, sign_pipe = 0, sign_and = 0; //need for correct get_list for file
	char **file = get_list(&sign2, &sign_pipe, &sign_and);
      	if(sign > 0){
		int fd = open(file[0], O_WRONLY | O_CREAT, DEF_MODE);
		if(fd < 0){
        	        return 0;
		}
		int savefd = dup(1);
		dup2(fd, 1);
		launch(list);
		close(fd);
		dup2(savefd, 1);
		close(savefd);
	}else{ 
		if(sign < 0){
			int fd = open(file[0], O_RDONLY, S_IROTH);
			if(fd < 0){
               	 		return 0;
        		}
			int savefd = dup(0);
			dup2(fd, 0);
			launch(list);
			close(fd);
			dup2(savefd, 0);
			close(savefd);
		}
	}
	memclear(file);
	return 1;
}
*/
/*void creatpipe(char **cmd1, int sign1) {
//	char ***cmd = NULL;
//	int size = 0, bytes = 0, 
	int i = 0, fd[i][2];
	char **cmd = NULL;
	pipe(fd[i]);
	if(fork() == 0){
		dup2(fd[i][1],1);
		close(fd[i][1]);
		close(fd[i][0]);
		launch(cmd1);
	}
	close(fd[i][1]);
	wait(NULL);
	memclear(cmd1);
	for(i = 1; 1; i++){
//		bytes = (size + 2) * sizeof(char**);
//		printf(" bytes  =  %d\n", bytes);
//		cmd = realloc(cmd, bytes);
		int sign = 0, sign_pipe = 0, sign_and = 0;//for correct call get_list
		cmd = get_list(&sign, &sign_pipe, &sign_and);
//		size++;
		char **pipe_sign = get_list(&sign, &sign_pipe, &sign_and);
		if(!sign_pipe){
			memclear(pipe_sign);
			break;
		}
		memclear(pipe_sign);
		pipe(fd[i]);
		if(fork() == 0){
			dup2(fd[i - 1][0], 0);
			close(fd[i - 1][0]);
			dup2(fd[i][1], 1);
			close(fd[i][1]);
			close(fd[i][0]);
			launch(cmd);
		}
		memclear(cmd);
		close(fd[i - 1][0]);
		close(fd[i][1]);
		wait(NULL);
	}
	dup2(fd[i][0], 0);
	close(fd[i][0]);
	launch(cmd);
//	cmd[i + 1] = NULL;
//	for(i = 1; cmd[i] != NULL; i++){
//		memclear(cmd[i]);
//	}
	memclear(cmd);
}
*/

/*
	int fd[2], sign2 = 0, sign_pipe1 = 0, sign_and = 0; //sign_pipe1 = 0 for correct read cmd2
	pipe(fd);
	char **cmd2 = get_list(&sign2, &sign_pipe1, &sign_and);
	if(fork() == 0){
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		if(sign1){
			if(forwarding(sign1, cmd1)){
			}else{
				exit(1);
			}

		}else{
			if(execvp(*cmd1, cmd1) == -1){
                        perror("incorrect command");
                        exit(1);
			}
		}
	}
	if(fork() == 0){
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
                if(execvp(*cmd2, cmd2) == -1){
                        perror("incorrect command");
                        exit(1);
                }
	}
	close(fd[0]);
	close(fd[1]);
	wait(NULL);
	wait(NULL);
	memclear(cmd2);
}
*/

void change_dir(char **cmd, char *home){
	char a[100];
	if(cmd[1] == NULL || strcmp(cmd[1], "~") == 0){
		chdir(home);
	}else{
		chdir(cmd[1]);
	}
	printf("%s\n", getcwd(a,100));
}

void handler(int signo){

	puts("received SIGINT");
}

int main(int argc, char **argv){
	char *home = getenv("HOME");
	signal(SIGINT, handler);
	while(1){
//		char *files[2] = {NULL, NULL};
		char ***cmd = get_cmd();
		if(check_end(**cmd)){
			memclear(*cmd);
			break;
                }
		if(strcmp(*cmd[0], "cd") == 0){
			change_dir(*cmd, home);
		}
/*		else{
			if(sign_pipe){
                       	        creatpipe(*cmd, sign);
			}else{
				if(sign){
				forwarding(sign, *cmd);	
				}else{
				launch(*cmd);
				}
			}
		}
		if(sign_conv){
			if(sign_conv > 0){
			//	conveer_and(list);
			}else{
			//	conveer_or(list);
			}
		}
		*/
        	putchar('\n');
		memclear(*cmd);
	 }
        return 0;
}

