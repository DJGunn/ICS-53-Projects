#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//last 2 includes are for fork/wait/pid_t

//add a boolean type
typedef enum {false, true} bool;


////////////// DISCLAIMER: THE isNumber() FUNCTION IS NOT MINE //////////////
//This function is credit to niyasc on stackoverflow, I take no credit for this function (though I wish I'd' thought of it)//
//Everything besides this function is my own unique code//
bool isNumber(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}
////////////// END OF IMPORTED FUNCTION //////////////

//the recursive search using children processes
//according to the assignment, processes that spawn children should not personally check a location in the array
bool recursiveSearch(int myArray[], int myNum, int first, int last, int size)
{
	//declare child variables, will probably move and organize them better later
	pid_t childPID1;
	pid_t childPID2;

	//check array size
	if (size > 1)
	{
		//calculate first and last of A & B
		int firstA, firstB, lastA, lastB, sizeA, sizeB;
		
		//if the size of the array is ODD
		if (size % 2 == 1)
		{
			//array A - first half of our array
			firstA = first;
			lastA = firstA + ((size/2)-1);
			sizeA = (lastA - firstA) + 1;

			//array B - second half of our array, which gets the extra piece
			firstB = lastA + 1;
			lastB = firstB + (size/2);
			sizeB = (lastB - firstB) +1;
		}
		//if the size of the array is EVEN
		else
		{
			//array A - first half of our array
			firstA = first;
			lastA = firstA + ((size/2)-1);
			sizeA = (lastA - firstA) +1;

			//array B - second half of our array
			firstB = lastA + 1;
			lastB = firstB + ((size/2)-1);
			sizeB = (lastB - firstB) + 1;
		}

		//spawn first child
		childPID1 = fork();

		//call function again with child
		if (childPID1 == 0)
		{
			//make our recursive call
			bool found1 = recursiveSearch(myArray, myNum, firstA, lastA, sizeA);

			//if we found a result, exit success, otherwise exit failure
			if (found1 == true) exit(0);
			else exit(1);
		} 
		else if (childPID1 != 0) //have parent spawn a 2nd child
		{
			childPID2 = fork();

			if (childPID2 == 0)
			{
				//have 2nd child make recursive call
				bool found2 = recursiveSearch(myArray, myNum, firstB, lastB, sizeB);

				//if we found a result, exit success, otherwise exit failure
				if (found2 == true) exit(0);
				else exit(1);
			}
		}

		//declare variables for waiting
		pid_t wpid;
		int status = 0;
		int failureCode;

		//our flag for if we found our target
		bool found = false;

		//iterate through children processes
		while ((wpid = wait(&status)) > 0)
		{
			//only while we haven't found the target'
			if (found == false)
			{
				//get the results from each child
				failureCode = WEXITSTATUS(status);

				//if one of the children found it, mark flag to true
				if (failureCode == 0) found = true;
			}

		}

		//return our results to whoever called us
		return found;
	}
	else if (size == 1 && first == last)
	{
		if (myArray[first] == myNum)
		{
			printf("%i ", first); //print the location if found
			return true; //found one!
		}
		else return false; //report didn't find it'
	}
	else
	{
		printf("\nSomehow an error managed to occur and we have an array with size <1.\nPress 'Enter' to quit.\n-1");

		getc(stdin);

		exit(1);
	}
}

int main(int argc, char *argv[])
{
	//check to make sure we have the proper number of args
	if (argc != 3 || !isNumber(argv[2]))
	{
		printf("Proper syntax is \"%s [filename.txt] [# to search for]\"\nExpected # arguments: 2\nYour # arguments: %i\nThe search value must be a number.\nPress enter to exit and try again.\n-1", argv[0], argc - 1);

		//pauses program until enter is pressed
		getc(stdin);

		exit(1);
	}


	//error flag
	bool isError = false;

	//create array to hold the array from the file, we add one additional element to catch any extra EOF or \n characters and have 20 characters to make room for spaces
	int myInts[10];

	//used for determining if the file is empty
	struct stat fileStat;
	stat(argv[1], &fileStat);

    //create an int to hold the size of the array
    int size = 0;

	//Only continue if thefile isn't empty
	if (fileStat.st_size == 0) isError = true;
	else
	{
    	//create a file stream and open the provided file in read only mode
		FILE *arrayFile = fopen(argv[1], "r");
		
		//save the line of text from the file to a formal string
		char myString[1025];

		//get string from file
		fgets(myString, sizeof(myString), arrayFile);

		//create delimiters
		char delimiters[] = " \t\r\n\v\f";
		//parse the first token
		char *token = strtok(myString, delimiters);

		//make sure the token isn't null (ie if the file only contained white space)
		if (token == NULL) isError = true;
		else if (isdigit(*token)) /*check to make sure we're only working with digits*/
		{
			//counter
			int i = 0;

			//parse the rest of the tokens while there are tokens remaining
			while (token)
			{
				//check to make sure our token is a digit
				if (!isdigit(*token) || i > 9)
				{
					//since it's not a delimiter, and not a digit, it's an error
					isError = true;

					break;
				}
				else
				{
					//convert from alphanumeric to integer and store in array
					myInts[i] = atoi(token);

					//increment
					i++;

					//get the next token
					token = strtok(NULL, delimiters);
				}
			}
            //save # of elements
            size = i;

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

		exit(1);
	}

	//call & print our recursive search & result; args[2] holds the target # we're searching for
	bool finalResult = recursiveSearch(myInts, atoi(argv[2]), 0, size-1, size);

	if (finalResult == false) printf("-1");

	//formatting
	printf("\n");

	return 0;
}
