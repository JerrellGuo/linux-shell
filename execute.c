#include "mysh.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAXLENTH 256

/*
CITS2002 Project 2 2015
Name(s):		student-name1 (, student-name2)
Student number(s):	student-number-1 (, student-number-2)
Date:		date-of-submission
*/

//ʵ��cd ����
int changeDirectory(char* args[]){
	int find = 0;
	//���ֻ����cd����ôcd��HOME 
	if (args[1] == NULL) {
		check_allocation(HOME);
		chdir(HOME);
		return EXIT_SUCCESS;
	}
	else{ 
		//���cd�Ĳ���������/
		check_allocation(CDPATH);
		if(!strchr(args[1], '/')){
			char *d=":";
			char *p;
			//�ָ�CDPTAH
			char *tempCDPATH = strdup(CDPATH);
			p=strtok(tempCDPATH,d);
			//���γ���cd��CDPATH�б��ÿһ��
			while(p)
			{
				char path[100] = "";
				strcat(path, p);
				strcat(path, "/");
				strcat(path, args[1]);  //��CDPTAH�͵�ǰ�����CD�Ĳ�����ϳ�Ŀ¼temp
				if(chdir(path) == 0) { //���뵽temp�ɹ�����ѭ��
					find = 1; 
					break;
				} 
				p = strtok(NULL,d);
			}
			if(!find)//���û�ҵ����Խ����Ŀ¼
			{
				printf(" %s: no such directory\n", args[1]);
				return EXIT_FAILURE;
			}
			free(tempCDPATH);
		}
		//���Ŀ¼����/,��ֱ�ӽ���
		else if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

//ִ��shell�ű�
int executeShellScript(char* filepath){
	int exitstatus = EXIT_SUCCESS;
	int status;
	int pid;
	if (access(filepath, R_OK) == 0){//�������filepath�ҿɶ�
		pid = fork(); //�����ӽ���
		if(pid == -1){
			perror("Child process could not be created");
			exitstatus = EXIT_FAILURE;
		}
		else if(pid == 0){ //�ӽ���
			interactive = false;
			FILE *pf =  fopen(filepath, "r");//���ļ�
			while(!feof(pf)) {
				CMDTREE	*t = parse_cmdtree(pf);
				if(t != NULL) {
					exitstatus = execute_cmdtree(t); 
					free_cmdtree(t);
				}
			}
			fclose(pf); //�ر��ļ�
			exit(exitstatus);
		}
		else{
			wait(&status);
			if(status != 0)
				exitstatus = EXIT_FAILURE;
		}
	}
	else exitstatus = EXIT_FAILURE;
	return exitstatus;
}

//ִ���ⲿ����
int executeOutterComand(CMDTREE *t){
	int exitstatus = EXIT_FAILURE;
	if(strchr(t->argv[0], '/') != NULL){//���command�Ĳ�������/
		execv(t->argv[0], t->argv); 
		exitstatus = executeShellScript(t->argv[0]);
	}
	else { //���command�Ĳ���������/
		char *d=":";
		char *p;
		//�ָ�PTAH
		check_allocation(PATH);
		char *tempPATH = strdup(PATH);
		strcpy(tempPATH, PATH);
		p=strtok(tempPATH,d);
		//���γ�������PATH�б��ÿһ��Ŀ¼
		while(p)
		{
			char temp[MAXLENTH] = "";
			strcat(temp, p);
			strcat(temp, "/");
			strcat(temp, t->argv[0]);  //��PATH�͵�ǰ�����CD�Ĳ�����ϳ�Ŀ¼temp
			execv(temp, t->argv);
			exitstatus = executeShellScript(temp); 
			if(exitstatus == EXIT_SUCCESS) break;;
			p = strtok(NULL,d);
		}
		free(tempPATH);
	}
	return exitstatus;
}

//����Ƿ����ڲ�����
bool checkBuiltin(CMDTREE *t){
	if(strcmp(t->argv[0],"exit") == 0 || strcmp(t->argv[0],"cd") == 0 || strcmp(t->argv[0],"time") == 0
		|| strcmp(t->argv[0],"set") == 0)
		return true;
	return false;
}

//���û�������
int setInternalVar(char * args[]){
	int exitstatus = EXIT_SUCCESS;
	if(args[1] == NULL) return EXIT_FAILURE;

	if(args[2] == NULL) args[2] = "";

	if(strcmp(args[1], "HOME") == 0){
		free(HOME);
		HOME = strdup(args[2]);
	}
	else if(strcmp(args[1], "CDPATH") == 0){
		free(CDPATH);
		CDPATH = strdup(args[2]);
	}
	else if(strcmp(args[1], "PATH") == 0){
		free(PATH);
		PATH = strdup(args[2]);
	}
	else{
		exitstatus = EXIT_FAILURE;
	}
	return exitstatus;
}

//ִ���ڲ�����
int executeBuitlinComand(CMDTREE *t){
	int exitstatus = EXIT_SUCCESS;
	if (strcmp(t->argv[0],"exit") == 0) {//����exit����
		if(t->argc > 1)
			exit(atoi(t->argv[1]));
		else exit(exitstatus);
	}
	else if(strcmp(t->argv[0], "cd") == 0){//����cd����
		exitstatus = changeDirectory(t->argv);
	}
	else if(strcmp(t->argv[0], "time") == 0){
		if(t->argc > 1){ //���time���滹��ָ��
			struct timeval tv_begin, tv_end; //����ʱ��ṹ������
			char** temp = t->argv; //����argvָ��

			//���ڴ�����time�������������1,argvָ���1
			t->argc--; 
			t->argv++;  

			gettimeofday(&tv_begin, NULL);//��ȡ��ʼʱ��
			exitstatus = execute_cmdtree(t); //�ݹ鴦������
			gettimeofday(&tv_end, NULL);//��ȡ����ʱ��
			double timeUsed =  1000000*(tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec; //ʱ����λ΢��
			timeUsed /= 1000; //ʱ����λ����
			printf("%.2fmsec\n", timeUsed);

			t->argv = temp; //�ָ�argvָ��
		}
		else printf("0msec\n");
	}
	else if(strcmp(t->argv[0], "set") == 0){//���û�������
		exitstatus = setInternalVar(t->argv);
	}
	return exitstatus;
}

//�ض���
void executeRedirection(CMDTREE *t){
	int fileDescriptor; // between 0 and 19, describing the output or input file

	if(t->infile != NULL){
		fileDescriptor = open(t->infile, O_RDONLY, 0600);  
		if(fileDescriptor == -1){
			printf(" %s: no such file\n", t->infile);
			exit(EXIT_FAILURE);
		}
		dup2(fileDescriptor, STDIN_FILENO);
		close(fileDescriptor);
	}

	if(t->outfile != NULL && t->append == false) {
		fileDescriptor = open(t->outfile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
		if(fileDescriptor == -1){
			printf(" %s: open fail\n", t->outfile);
			exit(EXIT_FAILURE);
		}
		dup2(fileDescriptor, STDOUT_FILENO); 
		close(fileDescriptor);
	}

	else if(t->outfile != NULL){
		fileDescriptor = open(t->outfile, O_CREAT | O_APPEND  | O_WRONLY, 0600); 
		if(fileDescriptor == -1){
			printf(" %s: open fail\n", t->outfile);
			exit(EXIT_FAILURE);
		}
		dup2(fileDescriptor, STDOUT_FILENO); 
		close(fileDescriptor);
	}
}

//�ָ��ض���
void undoRedirection(int fd1, int fd2){
	dup2(fd1, STDIN_FILENO);
	dup2(fd2, STDOUT_FILENO); 
}

//ִ�йܵ�����
int pipeHandle(CMDTREE* t){
	int filedes[2];
	int pidA, pidB;
	int statusA, statusB;
	int exitstatus = EXIT_SUCCESS;
	//�����ܵ����ӽ���A,B����������A������ض��򵽹ܵ���д�ˣ�������B�������ض��򵽹ܵ��Ķ���
	if ( pipe(filedes) == 0 ){  //��������ܵ��ɹ�
		pidA = fork(); //�����ӽ���A
		if(pidA == -1){
			perror("Child process could not be created");
			exitstatus = EXIT_FAILURE;
		}
		else if (pidA == 0 ) {  //�ӽ���A
			dup2( filedes[1], STDOUT_FILENO );  //��stdout�ض��򵽹ܵ�filedes[1]
			close( filedes[0] );   //�ص��ܵ��������
			close( filedes[1] );   //�ص��ܵ��������
			exitstatus = execute_cmdtree(t->left);//�ݹ鴦��t->left
			exit(exitstatus);
		}
		else{
			pidB = fork(); //�����ӽ���B
			if(pidB == -1){
				perror("Child process could not be created");
				exitstatus = EXIT_FAILURE;
			}
			else if(pidB == 0) { //�ӽ���B
				dup2( filedes[0], STDIN_FILENO); //ͬ�Ͻ���A
				close( filedes[1] );   
				close( filedes[0] );   
				exitstatus = execute_cmdtree(t->right);
				exit(exitstatus);
			}
			else{ //������
				close(filedes[1]);
				close(filedes[0]);
				waitpid(pidA, &statusA, 0);
				waitpid(pidB, &statusB, 0);
				if(statusA != 0 || statusB != 0)
					exitstatus = EXIT_FAILURE;
			}
		}
	}
	return exitstatus;
}

// -------------------------------------------------------------------
//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t)
{
	int  exitstatus;
	int status;
	int pid;
	int fd1, fd2;
	fd1 =dup(STDIN_FILENO); 
	fd2 =dup(STDOUT_FILENO); 

	if(t == NULL) {			// hmmmm, that's a problem
		exitstatus	= EXIT_FAILURE;
	}
	else {				// normal, exit commands
		exitstatus	= EXIT_SUCCESS;
		switch (t->type) {
		case N_COMMAND :
			executeRedirection(t);//�ض���
			if(checkBuiltin(t) == true){ // ������ڽ�����
				exitstatus = executeBuitlinComand(t);
			}
			else{ //������ⲿ����
				//�����ӽ���
				pid = fork();
				if(pid == -1){ //��������ӽ���ʧ��
					perror("Child process could not be created");
					exitstatus = EXIT_FAILURE;
				}
				else if(pid != 0){
					wait(&status);
					if(status != 0)
						exitstatus = EXIT_FAILURE;
				}
				else{
					exitstatus = executeOutterComand(t);
					if(exitstatus == EXIT_FAILURE)
					{
						perror("Command execute fail");
						exit(EXIT_FAILURE);
					}
					exit(EXIT_SUCCESS);
				}
			}
			undoRedirection(fd1,fd2);//�ָ��ض���
			break;

		case N_SUBSHELL :
			pid = fork();
			if(pid == -1){
				perror("Child process could not be created");
				exitstatus = EXIT_FAILURE;
			}
			else if(pid != 0){
				wait(&status);
				if(status != 0)
					exitstatus = EXIT_FAILURE;
			}
			else{
				executeRedirection(t);
				exitstatus = execute_cmdtree(t->left);
				exit(exitstatus);
			}

			break;

		case N_AND :
			exitstatus = execute_cmdtree(t->left);
			if(exitstatus == EXIT_SUCCESS)
				exitstatus = execute_cmdtree(t->right);
			break;

		case N_OR :
			exitstatus = execute_cmdtree(t->left);
			if(exitstatus == EXIT_FAILURE)
				exitstatus = execute_cmdtree(t->right);
			break;

		case N_PIPE :
			if(t->left->outfile != NULL || t->right->infile != NULL)
				exitstatus = EXIT_FAILURE;
			else exitstatus = pipeHandle(t);
			break;

		case N_SEMICOLON :
			execute_cmdtree(t->left);
			exitstatus = execute_cmdtree(t->right);
			break;

		case N_BACKGROUND :
			pid = fork();
			if(pid == -1){
				perror("Child process could not be created");
				exitstatus = EXIT_FAILURE;
			}
			else if(pid != 0){
				execute_cmdtree(t->left);
				wait(NULL);
			}
			else{
				exitstatus = execute_cmdtree(t->right);
				exit(exitstatus);
			}
			break;

		default :
			fprintf(stderr, "%s: invalid NODETYPE in print_cmdtree0()\n", argv0);
			exit(1);
			break;
		}
	}
	return exitstatus;
}
