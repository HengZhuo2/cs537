#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int lineNum(FILE *fstream)
{
	struct stat st;
	stat("p1.in", &st);
	int size = st.st_size;
	int lsize = 1;
	printf("file p1.in size is: %i\n", size);
	char lbuffer[size];
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

	//store lines into buffer
	char buffer[lsize][513];
	int linePos = 0;
	int charPos = 0;
	for(int i=0; i <size; i++)
	{
		buffer[linePos][charPos++] = lbuffer[i];
		printf("value at [%i][%i] assigned is %c\n", linePos, charPos,lbuffer[i]);
		if(lbuffer[i] == '\n')
		{
			charPos = 0;
			linePos++;
		}
	}

	//shuffling
	char str[size];
	linePos = 0;
	charPos = 0;
	for(int i = 0; i < size; i ++)
	{
		str[i] = buffer[linePos][charPos];
		// printf("value at [%i][%i] assigned is %c\n", linePos, charPos,buffer[linePos][charPos]);
		if(buffer[linePos][charPos] =='\n')
		{
			if(linePos > lsize/2)
			{
				linePos = lsize-linePos;
			}
			else
			{
				linePos = lsize-linePos-2;
			}
		}
		else
		{
			charPos++;
		}
	}
	printf("str is %s\n", str);

	FILE *fw;
	fw = fopen("p1.out", "w");
	fwrite(str, size, 1, fw);
	fclose(fw);


	//debug checker:
	//
	// charPos = 0;
	// linePos = 0;
	// for(int i = 0 ; i < size; i++)
	// {
	// 	printf("%c", buffer[linePos][charPos]);
	// 	if(buffer[linePos][charPos] =='\n')
	// 	{
	// 		charPos = 0;
	// 		linePos++;
	// 	}
	// 	else
	// 	{
	// 		charPos++;
	// 	}
	// }


	return lsize;
}

int main()
{
	FILE *fstream;
	fstream = fopen("p1.in", "r");
	int lineSize = lineNum(fstream);
	printf("file p1.in line number is: %i\n", lineSize);

	fclose(fstream);
}