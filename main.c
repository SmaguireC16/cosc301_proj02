#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>

struct node {
	char * word;
	struct node *next;

};

void removeWhiteSpace(char *str) {
	int i = 0;
	int j = 0;
	while (i < strlen(str)) {
		if (!isspace(str[i])) {
			str[j] = str[i];
			i++; 	
			j++; 	
		}
		else {i++;}
	}
	str[j] = '\0';
}


char** tokenify(const char *s) {
	char *str = strdup(s);
	int count =0;
	char *token= strtok(str, " \n\t");
	while (token != NULL){
		count++;
		token = strtok(NULL, " \n\t");
		}
	count++;
	char **rv = malloc(count*sizeof(char*));
	
	char*str1=strdup(s);

	char *token1=strtok(str1, " \n\t");
	int count1=0;

	while (token1!=NULL){
		char *p= strdup(token1);
		rv[count1]=p;
		count1++;
		token1=strtok(NULL, " \n\t");
		}

	rv[count-1]=NULL;
	return rv;
}
char** tokenify_commands(char input []) {
	char *pinput = input;
	char *pstring = strdup(pinput); 
	char *ptoken; 
	ptoken = strtok(pstring, ";");
	char **array = malloc(sizeof(char*)*((strlen(pinput)/2)+2));
	int i = 0;
	while (ptoken != NULL) {
		char *ptrtok = strdup(ptoken);
		array[i] = ptrtok;
		ptoken = strtok(NULL, ";"); 
		i++;	
	}
	array[i]=NULL;
	free(pstring);
	return array;
}

void free_tokens(char **tokens) {
    int i = 0;
	char ** temp = tokens;
    while (tokens[i] != NULL) {
        free(tokens[i]);
        i++;
    }
    free(temp); 
}

void list_append(char* cmd, struct node **head) {
	
	struct node *newNode = malloc(sizeof(struct node));
	newNode->word = cmd;
	newNode->next = NULL;
	if (*head == NULL) { 
		*head = newNode; 
		return;
	}
	struct node *temp = *head;
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = newNode;
	return;
}

void free_LL(struct node *head) { 
    while (head != NULL) {
		struct node *temp = head;
		struct node *next = head->next;
        free(temp);
        head = next;
    }
	free(head);
    return;
}
bool check_mode(char* input) {
	char * mode = "mode";

	removeWhiteSpace(input);
	if ((strcmp(mode, input))==0) {
		return true;
	}
	return false;

};

int main(int argc, char **argv) {

	int mode = 0; //0=seq, 1=parallel
	int * pmode = &mode;
	int exitDelay = 0; 
	int new_mode = 0; 
	char ** ptrs;
	bool done = false;
	char *nstring = malloc(sizeof(char)*1024);
	struct node *head = malloc(sizeof(struct node));
	const char * mode_literal = "mode";
	const char * exit_literal = "exit";//may be a poor variable name choice
	const char * seq = "s";
	const char * par = "p";
	const char * sequential = "sequential";
	const char * parallel = "parallel";
	char** command;
	FILE *fp = NULL;
	fp = fopen ("shell-config", "r");
	char line[1024];
	char * pline = line;
	char * pnew;
	char newstr[1024];
	char *newstring=newstr;

	while(fgets(line, 128, fp) != NULL) {
		removeWhiteSpace(pline);
		pnew = malloc(sizeof(char)*1024);
		strcpy(pnew, pline);
		char slash[] = "/";
		char * pslash = slash; 
		pnew=strcat(pnew, pslash);// now suitable for shell
		list_append(pnew, &head);
  		}

	struct node *tempnode = head->next;
	while (!done) {
		char input[1024];
		char * input2 = input;
		printf("prompt: ");
		fflush(stdout);
		if (fgets(input, 1024, stdin)!= NULL) {
			input[strlen(input)-1] = '\0';
			for (int i = 0; i<strlen(input); i++) {
				if (input[i] == '#') input[i] = '\0';
			}//removed comments
			ptrs = tokenify_commands(input); 
			if (strstr(input, mode_literal)!=NULL) {
				if (check_mode(input)==true) {
					if (mode ==0) printf("Sequential Mode\n");
					else printf("Parallel Mode\n"); 
				}
			}
			if (mode ==0) {		//Sequential Mode: 
				char** temp1= ptrs;
				int i =0;

				pid_t child_pid;
				while (temp1[i]!=NULL) {

					if (strstr(temp1[i], mode_literal) !=NULL) {

						if (strstr(temp1[i],seq)!=NULL) {new_mode=0;}
						if (strstr(temp1[i],par)!=NULL) {new_mode=1;}
						i++;
						continue;
						}
					const char *EXIT = "exit";
					if (strstr(temp1[i], EXIT) !=NULL) {
						exit(0);
					}

					child_pid = fork();
					if (child_pid ==0) {  // if in child
						command = tokenify(temp1[i]);
						
						if (command[0] == NULL) {
							i++;
							continue;
						}
						if (execv(command[0], command) < 0) {
		    						printf("execv failed\n");
						}
						free_tokens(command);
						exit(0);
					}
					else {
						//if parent wait
						int return_status;
						waitpid(child_pid, &return_status, 0); 
						i++;	
					}
				}
				if (new_mode==0) {*pmode=0;}
				if (new_mode==1) {*pmode=1;}
			}
			else if (mode ==1) {	//Parallel mode
				char ** temp = ptrs;
				int i = 0;
				while (temp[i] != NULL) {
					if (strstr(temp[i], mode_literal) != NULL) {
						if (strstr(temp[i], seq)||strstr(temp[i],sequential)) new_mode = 0;
						else if (strstr(temp[i], par)||strstr(temp[i], parallel)) new_mode = 1;
						
					}
					
					if (strstr(temp[i], exit_literal)!=NULL) {
						exitDelay = 1;	
					}
					i++;		
				}
				i = 0;
				while (ptrs[i] !=NULL) {
					// if it's NOT mode or exit, then...
					if (strstr(ptrs[i], mode_literal)==NULL && strstr(ptrs[i], exit_literal)==NULL) {
						pid_t child = fork();						

						if (child ==0) {

							pid_t child2 = fork();
							if (child2 == 0) {
								command = tokenify(temp[i]);

								if (command[0] == NULL) {
									i++;
									continue;
								}
								if (execv(command[0], command) < 0) {
									printf("execv failed\n");
								}
								free_tokens(command);
								exit(0);
							}	
							else {
								int return_status;
								waitpid(child2, &return_status, 0);
								printf("Process %i has completed", child2);
								exit(0);
							}						
						}
						i++;
				}
				if (new_mode==0) {*pmode=0;}
				if (new_mode==1) {*pmode=1;}
				}
				if (exitDelay == 1) {
					exit(0);
				}
			}
		
		}
		if (feof(stdin)) exit(0); 
		
		
	
	fclose(fp);
	free(head);
	free(command);		
	return 0;
	}
}

