#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	printf("welcome to teatppid\n");

	int pidp = getpid();

	int fktry = fork();

	if(fktry <0 )
	{
		fprintf(stderr, "failed\n");
		exit(1);
	}
	else if(fktry == 0)
	{
		if(pidp != (int)getpid())
		{
			printf("success from child (pid:%d)\n", (int)getpid());
		}
	}
	else
	{
		int wtry = wait(NULL);
	}

	return 0;
}