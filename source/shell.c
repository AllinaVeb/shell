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
	if(fork() == 0){
		if(execvp(*command, command) == -1){
			perror("incorrect command");
			exit(1);
		}
	}
	wait(NULL);
	
}

char **get_list(int *sign){
	char end = 0, c, **string = NULL;
	int size = 0, bytes;
        while(end != '\n'){
		bytes = (size + 2) * sizeof(char*);
		string = realloc(string, bytes);
		c = getchar();
		while(c == ' ' || c == '\t'){
                	c = getchar();
        	}
        	if(c == '<' || c == '>'){
               		if(c == '>'){
                       		*sign = 1;
                	}else{
                        	*sign = -1;
                	}
                	c = getchar();
       		}
		if(*sign == 0){
			string[size] = get_word(&end, c);
			size++;
		}else{
			break;
		}
        }
	string[size] = NULL;
	return string;
}

void memclear(char **list) {
	int i;
	for(i = 0; list[i] != NULL; i++) {
		free(list[i]);
	}
	free(list);
}

int forwarding(int *sign, char **list){
	int sign2 = 0;
	char **file = get_list(&sign2);
	int fd = open(file[0], O_RDONLY | O_WRONLY | O_CREAT, DEF_MODE);
 	if(fd < 0){
		return 0;
        }
	if(sign > 0){
		int savefd = dup(1);
		dup2(fd, 1);
		launch(list);
		close(fd);
		dup2(savefd, 1);
		close(savefd);
        }
	/* else{
		int savefd = dup(0)
		dup2(0, out file);
		launch(list);
		close(fd);
		dup2(savefd, 1);
		close(savefd);
	}
	*/
	memclear(file);
	return 1;
}

int main(int argc, char **argv){
	 while(1){
		int sign = 0;
		char **list = get_list(&sign);
                if(check_end(*list)){
			memclear(list);
			break;
                }
		if(sign){
			int fileopen = forwarding(&sign, list);	
		}else{
			launch(list);
		}
        	putchar('\n');
		memclear(list);
	 }
        return 0;
}

