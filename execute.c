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

//实现cd 命令
int changeDirectory(char* args[]){
	int find = 0;
	//如果只输入cd，那么cd到HOME 
	if (args[1] == NULL) {
		check_allocation(HOME);
		chdir(HOME);
		return EXIT_SUCCESS;
	}
	else{ 
		//如果cd的参数不包含/
		check_allocation(CDPATH);
		if(!strchr(args[1], '/')){
			char *d=":";
			char *p;
			//分割CDPTAH
			char *tempCDPATH = strdup(CDPATH);
			p=strtok(tempCDPATH,d);
			//依次尝试cd到CDPATH列表的每一项
			while(p)
			{
				char path[100] = "";
				strcat(path, p);
				strcat(path, "/");
				strcat(path, args[1]);  //将CDPTAH和当前输入的CD的参数组合成目录temp
				if(chdir(path) == 0) { //进入到temp成功跳出循环
					find = 1; 
					break;
				} 
				p = strtok(NULL,d);
			}
			if(!find)//如果没找到可以进入的目录
			{
				printf(" %s: no such directory\n", args[1]);
				return EXIT_FAILURE;
			}
			free(tempCDPATH);
		}
		//如果目录包含/,则直接进入
		else if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

//执行shell脚本
int executeShellScript(char* filepath){
	int exitstatus = EXIT_SUCCESS;
	int status;
	int pid;
	if (access(filepath, R_OK) == 0){//如果存在filepath且可读
		pid = fork(); //创建子进程
		if(pid == -1){
			perror("Child process could not be created");
			exitstatus = EXIT_FAILURE;
		}
		else if(pid == 0){ //子进程
			interactive = false;
			FILE *pf =  fopen(filepath, "r");//打开文件
			while(!feof(pf)) {
				CMDTREE	*t = parse_cmdtree(pf);
				if(t != NULL) {
					exitstatus = execute_cmdtree(t); 
					free_cmdtree(t);
				}
			}
			fclose(pf); //关闭文件
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

//执行外部命令
int executeOutterComand(CMDTREE *t){
	int exitstatus = EXIT_FAILURE;
	if(strchr(t->argv[0], '/') != NULL){//如果command的参数包含/
		execv(t->argv[0], t->argv); 
		exitstatus = executeShellScript(t->argv[0]);
	}
	else { //如果command的参数不包含/
		char *d=":";
		char *p;
		//分割PTAH
		check_allocation(PATH);
		char *tempPATH = strdup(PATH);
		strcpy(tempPATH, PATH);
		p=strtok(tempPATH,d);
		//依次尝试搜索PATH列表的每一个目录
		while(p)
		{
			char temp[MAXLENTH] = "";
			strcat(temp, p);
			strcat(temp, "/");
			strcat(temp, t->argv[0]);  //将PATH和当前输入的CD的参数组合成目录temp
			execv(temp, t->argv);
			exitstatus = executeShellScript(temp); 
			if(exitstatus == EXIT_SUCCESS) break;;
			p = strtok(NULL,d);
		}
		free(tempPATH);
	}
	return exitstatus;
}

//检查是否是内部命令
bool checkBuiltin(CMDTREE *t){
	if(strcmp(t->argv[0],"exit") == 0 || strcmp(t->argv[0],"cd") == 0 || strcmp(t->argv[0],"time") == 0
		|| strcmp(t->argv[0],"set") == 0)
		return true;
	return false;
}

//设置环境变量
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

//执行内部命令
int executeBuitlinComand(CMDTREE *t){
	int exitstatus = EXIT_SUCCESS;
	if (strcmp(t->argv[0],"exit") == 0) {//处理exit命令
		if(t->argc > 1)
			exit(atoi(t->argv[1]));
		else exit(exitstatus);
	}
	else if(strcmp(t->argv[0], "cd") == 0){//处理cd命令
		exitstatus = changeDirectory(t->argv);
	}
	else if(strcmp(t->argv[0], "time") == 0){
		if(t->argc > 1){ //如果time后面还有指令
			struct timeval tv_begin, tv_end; //定义时间结构体类型
			char** temp = t->argv; //保存argv指针

			//由于处理了time命令，参数总数减1,argv指针加1
			t->argc--; 
			t->argv++;  

			gettimeofday(&tv_begin, NULL);//获取开始时间
			exitstatus = execute_cmdtree(t); //递归处理命令
			gettimeofday(&tv_end, NULL);//获取结束时间
			double timeUsed =  1000000*(tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec; //时间差，单位微秒
			timeUsed /= 1000; //时间差，单位毫秒
			printf("%.2fmsec\n", timeUsed);

			t->argv = temp; //恢复argv指针
		}
		else printf("0msec\n");
	}
	else if(strcmp(t->argv[0], "set") == 0){//设置环境变量
		exitstatus = setInternalVar(t->argv);
	}
	return exitstatus;
}

//重定向
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

//恢复重定向
void undoRedirection(int fd1, int fd2){
	dup2(fd1, STDIN_FILENO);
	dup2(fd2, STDOUT_FILENO); 
}

//执行管道命令
int pipeHandle(CMDTREE* t){
	int filedes[2];
	int pidA, pidB;
	int statusA, statusB;
	int exitstatus = EXIT_SUCCESS;
	//创建管道和子进程A,B，并将进程A的输出重定向到管道的写端，将进程B的输入重定向到管道的读端
	if ( pipe(filedes) == 0 ){  //如果建立管道成功
		pidA = fork(); //建立子进程A
		if(pidA == -1){
			perror("Child process could not be created");
			exitstatus = EXIT_FAILURE;
		}
		else if (pidA == 0 ) {  //子进程A
			dup2( filedes[1], STDOUT_FILENO );  //把stdout重定向到管道filedes[1]
			close( filedes[0] );   //关掉管道的输入端
			close( filedes[1] );   //关掉管道的输出端
			exitstatus = execute_cmdtree(t->left);//递归处理t->left
			exit(exitstatus);
		}
		else{
			pidB = fork(); //建立子进程B
			if(pidB == -1){
				perror("Child process could not be created");
				exitstatus = EXIT_FAILURE;
			}
			else if(pidB == 0) { //子进程B
				dup2( filedes[0], STDIN_FILENO); //同上进程A
				close( filedes[1] );   
				close( filedes[0] );   
				exitstatus = execute_cmdtree(t->right);
				exit(exitstatus);
			}
			else{ //父进程
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
			executeRedirection(t);//重定向
			if(checkBuiltin(t) == true){ // 如果是内建命令
				exitstatus = executeBuitlinComand(t);
			}
			else{ //如果是外部命令
				//创建子进程
				pid = fork();
				if(pid == -1){ //如果创建子进程失败
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
			undoRedirection(fd1,fd2);//恢复重定向
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
