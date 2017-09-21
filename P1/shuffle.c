#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	FILE *fstream;
	fstream = fopen("p1.in", "r");
	if (fstream == NULL)
	{
		fprintf(stderr, "Error: Cannot open file foo\n");
		exit(1);
	}

	struct stat st;
	stat("p1.in", &st);
	int size = st.st_size;
	// printf("file p1.in size is: %i\n", size);


	//find out number of lines
	char lbuffer[size];
	int lsize = 1;
	if(fread(lbuffer, size, 1, fstream)!= 0){
		for(int i=0; i <size; i++)
		{
			// printf("checking: %c\n", lbuffer[i]);
			if(lbuffer[i] == '\n')
			{
				lsize++;
			}
		}
	}
	else
	{
		lsize = 0;
	}
	// printf("number of lines is : %i\n", lsize);
	fclose(fstream);

	//create new strin for incoming string
	char *strin;
	strin = (char *) malloc(lsize*512);

	//read in chars
	fstream = fopen("p1.in", "r");
	int linePos = 0;
	int charPos = 0;

	if(fread(lbuffer, size, 1, fstream)!= 0)
	{
		for(int i=0; i <size; i++)
		{
			*(strin+(linePos*512+charPos)) = (char)lbuffer[i];
			// printf("value at [%i][%i] assigned is %c\n", linePos, charPos,*(strin+(linePos*512+charPos)));
			// printf("data write at[%i]: %s\n", linePos, strin+linePos*512);
			if(lbuffer[i] == '\n')
			{
				charPos = 0;
				linePos++;
			}
			else
			{
				charPos++;
			}
		}

		*(strin+(linePos*512+charPos)) = (char)'\n';
	}
	//add extra \n
	// *(strin+(lineNum*512+charPos)+1) = (char)"\n";

	//write output into file
	FILE *fw;
	fw = fopen("p1.out", "w");
	// fwrite(strin, size, 1, fw);

	linePos = 0;
	charPos = 0;
	for(int i=0; i <size+1; i++)
	{
		// printf("data write at[%i][%i] %c\n", linePos, charPos, *(strin+(linePos*512+charPos)));
		fwrite(strin+(linePos*512+charPos), 1, 1, fw);

		if(*(strin+(linePos*512+charPos)) =='\n')
		{
			// printf("int i is now %i, %s", i, (strin+(linePos*512)));
			// printf("line number is now: %i\n", linePos);
			charPos = 0;

			if(linePos > lsize/2)
			{
				// printf("line number is now: %i\n", linePos);
				linePos = lsize-linePos;
				// printf("line number is now: %i\n", linePos);
			}
			else
			{
				// printf("line number is now: %i\n", linePos);
				linePos = lsize-linePos-1;
				// printf("line number is now: %i\n", linePos);
			}

		}
		else
		{
			charPos++;
		}
	}



	fclose(fw);


	// int lineSize = lineNum(fstream);
	// printf("file p1.in line number is: %i\n", lineSize);

	fclose(fstream);
	free(strin);
	exit(0);
}













