#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

//global list of jobs running at background
int jlist[20];
int pos;//position in the jlist, pointing to aviable slot

char error_message[30] = "An error has occurred\n";


int checkjob()
{
	for(int k=0; k< pos; k++)
	{
		if(waitpid(jlist[k],NULL,WNOHANG) != 0)
		{
			//this process finished, clear the pos
			for(int p=k; p< pos-1; p++)
			{
				//move all jobs forward
				jlist[p] = jlist[p+1]; 
			}

		}
	}
	return 1;
}

int killjobs()
{
	for(int k=0; k< pos; k++)
	{
		if(waitpid(jlist[k],NULL,WNOHANG) != 0)
		{
			kill(jlist[k],0);
		}
	}
	return 1;
}




char **parse(char *cmd)
{	
	int size = 128;
	char **tokens = malloc(size * sizeof(char*));
	char *token;
	char *svptr;

	if(!tokens)
	{
		fprintf(stderr,"allocation error\n");
		exit(1);
	}

	token = strtok_r(cmd, " \n\t\r\a", &svptr);
	int pos = 0;//position tracker

	while(token != NULL)
	{
		tokens[pos] = token;//put token into tokens
		pos++;
		//initial set memory not enough
		if(pos >= size)
		{
			size += size;//double the size
			tokens = realloc(tokens, size* sizeof(char*));
			if(!tokens)
			{
				//reallocate failed
				fprintf(stderr, "reallocation error\n");
				exit(1);
			}
		}
		token = strtok_r(NULL," \n\t\r\a", &svptr);
	}

	tokens[pos] = NULL;//last arg should be NULL
	return tokens;
}

//build in functions
int mysh_cd(char **args);
int mysh_pwd(char **args);
int mysh_exit(char **args);

char *buildin_str[] = 
{
	"cd",
	"pwd",
	"exit"
};

int (*buildin[]) (char **)=
{
	&mysh_cd,
	&mysh_pwd,
	&mysh_exit
};

int mysh_num_buildins()
{
	return sizeof(buildin_str) / sizeof(char *);
}

//implementation
int mysh_cd(char **args)
{
	if(args[1] == NULL)
	{
		//change dir to home
		if(chdir(getenv("HOME")) != 0)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
	}
	else
	{
		if(chdir(args[1]) != 0)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
	}
	return 1;
}

int mysh_pwd(char **args)
{
	if(args[1] != NULL)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else
	{
		if(strcmp(args[0], "pwd") == 0)
		{
			char path[128];
			getcwd(path, 128);
			printf("%s\n", path);
		}
	}
	return 1;
}

int mysh_exit(char **args)
{
  return 4;
}

int execute(char **args)
{
	int in,out;
	//check redicretion, pipeline, background
	int redicin = 0;
	int redicout = 0;
	int pipeflag = 0;
	int backflag = 0;
	char **cmd1;
	char **cmd2;

	for(int i = 0; args[i] != NULL; i++)
	{
		if(strcmp(args[i],"<" ) == 0)
		{
			redicin = 1;
			in = open(args[i+1],O_RDONLY);
			args[i] = NULL;
		}
		else if(strcmp(args[i],">" ) == 0)
		{
			redicout = 1;
			out = open(args[i+1], O_WRONLY | O_TRUNC | O_CREAT,S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			args[i] = NULL;
		}
		else if(strcmp(args[i], "|") == 0)
		{
			pipeflag = 1;

			cmd1 = (char **) malloc(5*sizeof(char*));
			cmd2 = (char **) malloc(5*sizeof(char*));

			args[i] = NULL; 
			cmd1 = args;
			cmd2 = &args[i+1];
		}
		else if(strcmp(args[i], "&") == 0)
		{
			backflag = 1;
			args[i] = NULL;
		}
	}

	int status;

	if(pipeflag)
	{
		//pipe operation	
		int pipeA[2];//create new pipe
		int pid = 0;
		//check pipe creation sucessful
		if (pipe(pipeA) == -1) {
       	 	write(STDERR_FILENO, error_message, strlen(error_message));
        	exit(EXIT_FAILURE);
    	}
    	fflush(stdout);
		pid = fork();
		if (pid == 0)
	    {
	    	//first cmd, close read end, connect write end
		    close(pipeA[0]);
	      	dup2(pipeA[1], STDOUT_FILENO);
	      	execvp(cmd1[0], cmd1);
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(0);
	    }
	  	else
	    {
	    	int pid2 = 0;
	    	fflush(stdout);
	    	pid2 = fork();
	    	if(pid2 == 0)
	    	{
	    		//second cmd, close write end, connect read end
	    		close(pipeA[1]);
	    		dup2(pipeA[0], STDIN_FILENO);
				execvp(cmd2[0], cmd2);
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(0);
	    	}
	    	else
	    	{
				close(pipeA[1]);
		    	close(pipeA[0]);
		    	//wait two childs finish
	    		do
				{
					waitpid(pid,&status,WUNTRACED);
					waitpid(pid2,&status,WUNTRACED);
				}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	    	}
	    }
	}
	//not pipeline
	else
	{
		fflush(stdout);
		int pid = fork();
		if(pid == 0)
		{
			// child
			if(redicin)
			{
				dup2(in, 0);
			}	
			if(redicout)
			{
				dup2(out, 1);
			}
			// sleep(10);
			execvp(args[0], args);
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(0);//when successful, never return
		}
		else if(pid >0)
		{
			//parent

			//check do we have backflag
			//if so, run backgound, dont wait, if not, wait
			if(!backflag)
			{
				do
				{
					waitpid(pid,&status,WUNTRACED);
				}while(!WIFEXITED(status) && !WIFSIGNALED(status));
			}
			else
			{
				//dont wait, but need to keep track of childs
				jlist[pos] = pid;
			}
			// printf("im a parent\n");
		}
		else
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}


		if(redicin)
		{
			close(in);
		}
		else if(redicout)
		{
			close(out);
		}
	}

	return 1;
}

int launch(char **args)
{
	int i;

	if(args[0] == NULL)
	{
		return 0;
	}

	//check weather if its buildin
	for(i=0; i< mysh_num_buildins();i++)
	{
		if(strcmp(args[0], buildin_str[i]) == 0)
		{
		  
			return(*buildin[i])(args);
		}
	}

	//not buildin function, go execute
	return execute(args);
}


int main(int argc, char *argv[])
{	
	//argument check
	if(argc != 1)
	{
		//wrong
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	//else, args correct

	int run = 1;
	int sid = 1;
	pos = 0;

	char *cmd = (char*) malloc(130*sizeof(char));
	char **args;
	// char error_message[30] = "An error has occurred\n";
	char c, h;
	
	while(run)
	{
		//waiting for command
		printf("mysh (%d)> ", sid);

		int cpos = 0;
		int toolong = 0;

		fgets(cmd,130,stdin);

		while (  ((c = cmd[cpos]) != '\n') && (c != EOF) && (!toolong)) 
		{
			// printf("c is now%d\n", c);
			if(cpos > 128)
			{
				write(STDERR_FILENO, error_message, strlen(error_message));
				toolong = 1;
				break;
			}
			cpos++;
		}

		// printf("here\n");

		if(toolong)
		{
			while( (h = getchar()) != '\n' && c != EOF)
				{
					//flush all stdin
				}
			sid ++;
			continue;
		}


		// if( strcmp(cmd,"\n") == 0)
		// {
		// 	// printf("nothing happened\n");
		// 	continue;
		// }

		// printf("cmd is %s", cmd);
		//parse command
		args = parse(cmd);

		if(args[0] == NULL)
		{
			continue;
		}

		//check jobs list
		checkjob();
		//launch cmd(s)
		run = launch(args);

		// printf("run is: %d\n", run);
		//this one finished, increment sid, go to next one
		if(run == 2)
		{
		 	write(STDERR_FILENO, error_message, strlen(error_message));
		 	// printf("An error has occurred\n");
		  	sid ++;
		  	// nextcmd = 1;
		}
		else if(run == 4)
		{
			run = 0;
			// printf("exit here1\n");
			// printf("exit here2\n");
			killjobs();
		  	exit(0);
		  	// break;
		  	// printf("exit here2\n");
		}
		else if(run)
		{
		  	sid ++;
		  	// nextcmd = 1;
		}
	}

	//exit cmd called
	//check all process done
	free(args);
	free(cmd);
	return 0;
}
