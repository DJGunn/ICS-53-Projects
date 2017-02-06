#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//last 2 includes are for fork/wait/pid_t

//add a boolean type
typedef enum {false, true} bool;

//the recursive search using children processes
//according to the assignment, only the last layer of children will actually check any numbers
//Won't this create a logrithmic # of processes? It seems inefficient...
void recursiveSearch(int myArray[], int myNum, int head, int tail)
{
	//check array size
	if (sizeof(myArray) > 1)
	{
		//calculate head and tail of A & B
		int headA, headB, tailA, tailB, sizeA, sizeB;
		
		//head & tail of first half of array
		headA = head;
		tailA = floor((sizeof(myArray) - 1) / 2);
		sizeA = (tailA - headA)+1;

		//head & tail of second half of array
		headB = ceil((sizeof(myArray)) / 2);
		tailB = tail;
		sizeB = (tailB - headB)+1;

		//manually assign memory space for our arrays
		int *frontHalf = malloc(sizeA * sizeof(int));
		int *backHalf = malloc(sizeB * sizeof(int));

		//error check
		if (!frontHalf || !backHalf)
		{
			printf("\nError: Unable to allocate memory for array chunks.\nPress 'Enter' to quit.\n-1");

			getc(stdin);

			exit(1);
		}

		//copy first and second half of original array into the newly allocated space
		memcpy(frontHalf, myArray, sizeA * sizeof(int));
		memcpy(backHalf, myArray + sizeA, sizeB * sizeof(int));

		//spawn first child
		int status = 0;
		pid_t wpid; //for waiting

		pid_t childPID1 = fork();
		pid_t childPID2;

		//call function again with child
		if (childPID1 == 0) recursiveSearch(frontHalf, myNum, headA, tailA);
		else if (childPID1 != 0) //have parent spawn a 2nd child
		{
			childPID2 = fork();

			if (childPID2 == 0) recursiveSearch(backHalf, myNum, headB, tailB); //have 2nd child make recursive call
		}

		//theoretically this will have each parent that spawned children, wait for the children they spawned
		while ((wpid = wait(&status)) > 0) {}
	}
	else if (sizeof(myArray) == 1 && head == tail) printf("%i ", head); //print the location if found
	else
	{
		printf("\nSomehow an error managed to occur and we have an array with size <1.\nPress 'Enter' to quit.\n-1");

		getc(stdin);

		exit(1);
	}

	//wait on the children
}

int main(int argc, char *argv[])
{
	//check to make sure arguments are entered properly
	if (argc != 3 || !isdigit(argv[3]))
	{
		printf("Proper syntax is %s [filename.txt] [# to search for]\nYou entered %i arguments instead of 2 arguments.\nPress enter to exit and try again.\n-1", argv[0], argc - 1);

		//pauses program until enter is pressed
		getc(stdin);

		exit(0);
	}

	//error flag
	bool isError = false;

	//create array to hold the array from the file, we add one additional element to catch any extra EOF or \n characters and have 20 characters to make room for spaces
	int myInts[10];

	//command prompt
	printf("$ ");

	//used for determining if the file is empty
	struct stat fileStat;
	stat("C:\\Users\\Flandre Scarlet\\Documents\\Visual Studio 2015\\Projects\\Lab 2\\Debug\\array.txt", &fileStat);

	//Only continue if thefile isn't empty
	if (fileStat.st_size == 0) isError = true;
	else
	{
		//create a file stream and open the provided file in read only mode
		//FILE *arrayFile = fopen(argv[1], "r");
		//temp
		FILE *arrayFile = fopen("C:\\Users\\Flandre Scarlet\\Documents\\Visual Studio 2015\\Projects\\Lab 2\\Debug\\array.txt", "r");

		//save the line of text from the file to a formal string
		char myString[1025];

		fgets(myString, sizeof(myString), arrayFile);

		//parse the first token
		char *token = strtok(myString, " ");

		//make sure the token isn't null (ie if the file only contained white space)
		if (token == NULL) isError = true;
		else if (isdigit(*token)) /*check to make sure we're only working with digits*/
		{
			//counter
			int i = 0;

			//parse the rest of the tokens while there are tokens remaining
			while (token)
			{
				if (!isdigit(*token) || i > 9)
				{
					isError = true;

					break;
				}

				//convert from alphanumeric to integer and store in array
				myInts[i] = atoi(token);

				//increment
				i++;

				//get the next token
				token = strtok(NULL, " ");
			}
		}
		else isError = true;
	}

	if (isError)
	{
		printf("Error: The loaded array file must abide by the following restrictions:\n");
		printf("- All characters in file must be integers separated by white space.\n- The file cannot be empty or only contain whitespace.\n");
		printf("- The file must not contain more than 10 numbers.\n- The total filesize must be less than 1025 characters.\nPress 'Enter' to exit and try again.\n-1");

		//pauses program until enter is pressed
		getc(stdin);

		exit(0);
	}

	//call & print our recursive search & result; args[2] holds the target # we're searching for
	recursiveSearch(myInts, atoi(argv[2]), 0, (sizeof(myInts) - 1));

	return 0;
}