#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>  
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <signal.h>

#include "parse.h"

//vars
extern char **environ;
int old_stdin, old_stdout, old_stderr;
int sigint;
int initflag;
int pipeflag;



//funcs
void execute();

int isBuiltin(Cmd c) {
	char *builtin_cmds[] = {"cd","echo","logout","nice","pwd","setenv","unsetenv","where"};
	int i;
	for(i=0; i<8; i++){
		if(strcmp(c->args[0],builtin_cmds[i])==0){
			return i;
		}
	}
	return -1;
}

void cdexe(Cmd c) {
	//vars
	char *home = getenv("HOME");
	//char path[512];
	//get directory
	int flag=0;
	char *dir = c->args[1];
	if(dir == NULL){
		dir = malloc(strlen(home));
		strcpy(dir,home);
		flag = 1;
		//printf("%s\n",dir);
	}
	if(c->nargs > 2){
		fprintf(stderr,"too many arguments\n");
		exit(1);
	} else {
	//change dir
	if(opendir(dir) != NULL) {
		//char *path;
		//chdir("/");
		//path = strtok(dir, "/");
		chdir(dir);
		//while((path = strtok(NULL, "/"))){
			//printf("%s\n",path);
			//chdir(path);
		//}
	}else{
		fprintf(stderr,"Dir does not exist\n");
		// exit(1);
	}
	}
	if(flag){
		free(dir);
		dir = NULL;
	}
/*
	//printf("%s\n",dir);
	//dir is NULL
	
	//dir start with ~
	if(dir[0]=='~'){
		if(strlen(dir)!=1){
			
			char *temp = strtok(dir, "~");
			dir = (char *)malloc(strlen(home)+strlen(temp));
			strcpy(dir, home);
			strcat(dir, temp);
			//printf("%s\n",dir);
		}else{
			strcpy(dir, home);
		}
	}

*/

	

}

void echoexe(Cmd c) {
	int i = 1;
	
	char **s = c->args;
	//exception handling
	int n = c->nargs;
	if(n == 1)
		return;
	for(i = 1; i < n; i++){
		printf("%s ",s[i]);
	}
	printf("\n");
}

void logoutexe() {
	exit(0);
}

int check_first(Cmd c){
	int Ret_Integer = 0;  
	int Integer_sign = 1;  

	char *pstr = c->args[1];
	
	//while(isspace(*pstr) == 0)
		//pstr++;
			//printf("d\n");
	//check sign
	if(*pstr =='-' || *pstr =='+')
		pstr++; 
      
	if(*pstr < '0' || *pstr > '9')
		return 0;

	else return 1; 
}


void niceexe(Cmd c) {
	//exception handling
	
	int n = c->nargs;
	int i = 0;
	if(n == 1)
		return;
	//set priority
	int result, old = getpriority (PRIO_PROCESS, 0);
	//have both number and argument
	if(n >= 3){
		//printf("%d\n", check_first(c));
		if(check_first(c)){

			int increment = atoi(c->args[1]);
			if(increment>19) increment = 19;
			if(increment<-20) increment = -20;
			//c->args[1] = NULL;
			//printf("%d\n",increment);
			result = setpriority(PRIO_PROCESS, 0, increment);
			for(i=0; i<(n-2); i++){
				strcpy(c->args[i], c->args[i+2]);

				//printf("%s\n", (c->args)[i+2]);
			}
			c->args[i] = NULL;
			c->args[i+1] = NULL;
			//printf("%s\n", (c->args)[i+1]);
			(c->nargs) = (c->nargs) - 2;
			execute(c);
		} else {
			result = setpriority(PRIO_PROCESS, 0, 4);
			for(i=0; i<(n-1); i++){
				strcpy(c->args[i], c->args[i+1]);

				//printf("%s\n", (c->args)[i+2]);
			}
			c->args[i] = NULL;
			//printf("%s\n", (c->args)[i+1]);
			(c->nargs) = (c->nargs) - 1;
			execute(c);
		}
	}
	//only have argument
	else if (n == 2){
		if(check_first(c)){
			int increment = atoi(c->args[1]);
			if(increment>19) increment = 19;
			if(increment<-20) increment = -20;
			result = setpriority(PRIO_PROCESS, 0, increment);
		// printf("in nice: %d\n", increment);
			
			return;
		}
		result = setpriority(PRIO_PROCESS, 0, 4);
		strcpy(c->args[0], c->args[1]);
		c->args[1] = NULL;
		c->nargs = c->nargs -1;
		execute(c);
	}

	//nice failed
	if(result == -1){
		// fprintf(stderr,"nice fail\n");
		// exit(1);
	}
}

void pwdexe(Cmd c) {
	char *dir = getcwd(NULL, 0);
	printf("%s\n", dir);
	free(dir);
	dir = NULL;
}

void setenvexe(Cmd c) {
	//vars
	char *var = c->args[1];
	char *word = c->args[2];

	//withou argument
	int n = c->nargs;

	if(n == 1) {
		char** env = environ;
 		while(*env) {
   			printf("%s\n", *env);
			env++; 
  		}
	}
	//given VAR no word
	else if(n == 2) {
		setenv(var,"",1);
	}
	//given VAR and word
	else if(n==3)
		setenv(var, word, 1);
}

void unsetenvexe(Cmd c) {
	//vars
	char *var = c->args[1];
	//withou argument
	int n = c->nargs;
	if(n == 1)
		return;
	//delete env
	if(n == 2) {
		unsetenv(var);	
	}
}

void whereexe(Cmd c) {
	//vars
	char *path = getenv("PATH");
	char *temp_path;
	temp_path = (char *)malloc(strlen(path));
	strcpy(temp_path, path);
	//printf("PATH is %s\n",path);
	int n = c->nargs;
	char *cur_path;
	char *command = c->args[1];
	char *env_path = strtok(temp_path, ":");
	char *builtin_cmds[] = {"cd","echo","logout","nice","pwd","setenv","unsetenv","where"};
	int i;
	
	//eligle structure
	if(n == 2){
		for(i=0; i<8; i++){
			if(strcmp(command,builtin_cmds[i])==0){
				printf("%s\n",builtin_cmds[i]);
			}
		}

		while(env_path != NULL) {
			cur_path = (char *)malloc(strlen(env_path)+strlen(command)+4);
			strcpy(cur_path, env_path);
			strcat(cur_path, "/");
			strcat(cur_path, command);
			if(access(cur_path,F_OK) == 0)
				printf("%s\n",env_path);
			free(cur_path);
			cur_path = NULL;
			env_path = strtok(NULL, ":");
		}
	}

	//free(temp_path);
	temp_path = NULL;
}

setupRedirect(Cmd c)
{
	int fd;

	//input redirection
	if(c->in == Tin){
		if((fd = open(c->infile, O_RDONLY)) == -1){
			fprintf(stderr, "open failed\n");
			// exit(1);
		}
		else{
			if(dup2(fd, 0) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
		close(fd); 
		}
	}

	//output redirection
	if(c->out == Tout){
		if((fd = open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))== -1){
			fprintf(stderr, "open failed\n");
			// exit(1);
		}else{
			if(dup2(fd,fileno(stdout)) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
		close(fd);
		}
	}


	if(c->out == ToutErr){
		if((fd = open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))== -1){
			fprintf(stderr, "open failed\n");
			// exit(1);
		}else{
			if(dup2(fd,fileno(stderr)) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
			if(dup2(fd,fileno(stdout)) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
		close(fd);
		}
	}

	if(c->out == Tapp){
		if((fd = open(c->outfile,O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))== -1){
			fprintf(stderr, "open failed\n");
			// exit(1);
		}else{
			if(dup2(fd,fileno(stdout)) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
		close(fd);
		}
	}

	if(c->out == TappErr){
		if((fd = open(c->outfile,O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IRGRP|S_IWGRP|S_IWUSR))== -1){
			fprintf(stderr, "open failed\n");
			// exit(1);
		}else{
			if(dup2(fd,fileno(stderr)) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
			if(dup2(fd,fileno(stdout)) == -1){
				fprintf(stderr, "dup2 failed\n");
				exit(1);
			}
		close(fd);
		}
	}

}

void savestd()
{
	old_stdin = dup(0);
	old_stdout = dup(1);
	old_stderr = dup(2);
	if(old_stdin == -1 || old_stdout == -1 || old_stderr == -1){
		fprintf(stderr,"dup failed\n");
		exit(1);
	}
}

void prompt();

void execute(Cmd c) {
	//set IO redirect
	setupRedirect(c);
	//check if built-in
	int cmdCase = isBuiltin(c);
	//if built-in
	//printf("%s\n",c->args[0]);
	if(cmdCase != -1){
		switch(cmdCase) {
			case 0:
				cdexe(c);
				break;
			case 1:
				echoexe(c);
				break;
			case 2:
				logoutexe();
				break;
			case 3:
				niceexe(c);
				break;
			case 4:
				pwdexe(c);
				break;
			case 5:
				setenvexe(c);
				break;
			case 6:
				unsetenvexe(c);
				break;
			case 7:
				whereexe(c);
				break;
		}
	} 
	else if(initflag == 0 && !strcmp(c->args[0], "end")){
		// logoutexe();
		printf("\n");
		// prompt();
	}
	//if not built in function	
	else {
		//vars
		char *input = c->args[0];
		DIR *dirptr = NULL;


		//fork and execute
		int pid = fork();
		if(pid < 0)
			fprintf(stderr, "error is fork\n");
		if(pid == 0) {
		//contains /
		//start with /
		//no /
		if(strstr(input, "/") == NULL) {
			char *path = getenv("PATH");

			char *temp_path = (char *)malloc(strlen(path));
			strcpy(temp_path, path);
			char *env_path = strtok(temp_path, ":");
			while(env_path != NULL) {
				char *cur_path = (char *)malloc(strlen(env_path)+strlen(input)+4);
				strcpy(cur_path, env_path);
				strcat(cur_path, "/");
				strcat(cur_path, input);
				if(access(cur_path,F_OK) == 0){
					strcpy(input, cur_path);
					free(cur_path);
					cur_path = NULL;
					break;
				}
				free(cur_path);
				cur_path = NULL;
				env_path = strtok(NULL, ":");
			}
		}

		//check path
		//printf("%s\n",input);
		if(access(input, F_OK) != 0){
			fprintf(stderr, "command not found\n");
			exit(1);
		}
		if(access(input, R_OK|X_OK) != 0 || (dirptr=opendir(input)) != NULL){
			fprintf(stderr, "permission denied\n");
			exit(1);			
		}
		execve(input, c->args,environ);
		exit(0);
		}
		else {
		int result;
			if(pipeflag){
				wait(&result);
				if(WEXITSTATUS(result) == 1)
					exit(1);
			}
			else
				wait(0);
		}
	
	}
		
}

void prompt() {
	//vars
	char hostname[64];
	//reset standard stream
	if(dup2(old_stdin,fileno(stdin)) == -1)
	{
		fprintf(stderr,"dup reset stdin failed\n");
		exit(1);
	}
	if(dup2(old_stdout,fileno(stdout)) == -1)
	{
		fprintf(stderr,"dup reset stdout failed\n");
		exit(1);
	}
	if(dup2(old_stderr,fileno(stderr)) == -1)
	{
		fprintf(stderr,"dup reset stderr failed\n");
		exit(1);
	}

	//print prompt
	if( gethostname(hostname,sizeof(hostname)) ){
        	fprintf(stderr,"gethostname calling error\n");
        	return;
    	}
	if(sigint == 1)
		printf("\n%s%% ",hostname);
	else 
		printf("%s%% ",hostname);
	fflush(stdout);
}




void setuppipe(Pipe pipeline) {
	//vars
	Cmd first = pipeline->head;
	Cmd temp = first;
	Cmd end = NULL;
	int n = 0, m = 0;
	int i,j;
	int pid, ref;
	int eflag = 0;
	int pipefd[2];
	int p1,p2;
	int subshell_pid;
	int result;
	pipeflag = 0;
	//count number of pipes
	while(temp->next != NULL){
		temp = temp->next;
		n++;
	}
	//if no pipeline
	if(n==0)
		execute(first);
	else{
	pipeflag = 1;
	//for(i=0;i<1;i++){
	if (pipe(pipefd) == -1){
		fprintf(stderr,"Pipe creation failed\n");
		exit(1);
	}
	p1 = -1;	
	for(i = 0; i <=n; i++){	
		pid = fork();
		if(pid<0){ //fork failed
			fprintf(stderr,"Pipe redirect failed\n");
			exit(1);
		}
		else if(pid){ //parent
			close(p1);
			p1=pipefd[0];
			close(pipefd[1]);
			//printf("%d\n",p1);
			if (pipe(pipefd) == -1){
				fprintf(stderr,"Pipe creation failed\n");
				exit(1);
			}
			wait(&result);
			if(WEXITSTATUS(result) == 1)
				break;
		}else if(pid == 0){ //child
			//printf("%d, %d %s\n",p1,i, first->args[0]);
			if(p1==-1){
				//close(pipefd[0]);
				if(dup2(pipefd[1],1)==-1){
					fprintf(stderr,"Pipe redirect failed\n");
					exit(1);
				}
				if(first->out == TpipeErr){
					if(dup2(pipefd[1],2)==-1){
						fprintf(stderr,"Pipe redirect failed\n");
						exit(1);
					}
				}
				execute(first);	
				exit(0);
			} else if(i==n) {//printf("%d, %d %s\n",p1,i, first->args[0]);
				if(dup2(p1,0)==-1){
					fprintf(stderr,"Pipe redirect failed\n");
					exit(1);
				}
				//close(p2);
				execute(first);	
				exit(0);
			}
			else{
				if(dup2(p1,0)==-1){
					fprintf(stderr,"Pipe redirect failed\n");
					exit(1);
				}
				//close(p2);
				if(dup2(pipefd[1],1)==-1){
					fprintf(stderr,"Pipe redirect failed\n");
					exit(1);
				}
				if(first->out == TpipeErr){
					if(dup2(pipefd[1],2)==-1){
						fprintf(stderr,"Pipe redirect failed\n");
						exit(1);
					}
				}
				//close(pipefd[0]);
				execute(first);	
				exit(0);

			}
		}
		first = first -> next;
	}

	}	

}

void run(){
	//variables
	Pipe pipeline;
	int pipefd[2];
  	int pid;
	Cmd first, last;
	char hostname[64];
	//savestd();
	//save standard stream
	//savestd();
		// int flag = 1;
	while(1) {
		//prompt

		// printf("tst");
		// if (isatty(fileno(stdin))){

		prompt();
		// }
		//parsing

		pipeline = parse();
		// printf("%s\n", pipeline);

		//no input
			// printf("something");
		if(pipeline==NULL){
			// flag = 0;
			continue;
		}
		//execution
		if(strcmp(pipeline->head->args[0],"end")==0){
		// if(strcmp(pipeline->head->args[0]," ")==0){
			// printf("end\n");
			break;
		}
		while(pipeline != NULL) {
			// flag = 1;
			setuppipe(pipeline);
			// printf("the other thing\n");

			pipeline = pipeline->next;
		}	
		// break;

	}

}


//extern 
void init() {
	//vars
	int fd;	
	Pipe pipeline;
	initflag = 1;
	//open file 
	char *home = getenv("HOME");
	char *path = (char *)malloc(strlen(home)+strlen("/.ushrc")+1);
	strcpy(path, home);
	strcat(path, "/.ushrc");
	//printf("%s\n",path);	
	if((fd = open(path, O_RDONLY)) == -1) {
		//fprintf(stderr, "open ushrc failed\n");
		//exit(1);
		initflag = 0;
		return;
	}else{
	//redirect to stdin
	if(dup2(fd, 0) == -1) {
		fprintf(stderr, "dup2 ushrc failed\n");
		exit(1);
	}
	close(fd);
	//run parse
	while(1){
		pipeline = parse();
		// printf("output: %s", pipeline->head->args[0]);
		if(pipeline == NULL) 
			continue;
		if(strcmp(pipeline->head->args[0],"end")==0){
		// if(strcmp(pipeline->head->args[0]," ")==0){
			break;
		}
	
		while(pipeline != NULL) {
			setuppipe(pipeline);
			pipeline = pipeline->next;	
		}
	}
	}
	initflag = 0;
}

// void handleTerm(){}

void handleInt() {
	// sigint = 1;
	// prompt();
	// sigint = 0;
	logoutexe();
}

void handleStp() {}

void main(){
	savestd();
	//initialization
	signal(SIGQUIT,SIG_IGN);
	// signal(SIGTERM, handleTerm);
	signal(SIGINT, handleInt);
	signal(SIGTSTP, handleStp);
	init();
	//printf("%d\n",initflag);
	//interactive operation
	run();
}
