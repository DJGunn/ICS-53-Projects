#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct room {
	//room number, and the numbers of adjacent rooms, -1 if no adjacent room
	int north;
	int south;
	int east;
	int west;

	//character array to hold the room's description
	char desc[1025];
};

//going to need to make a struct, and an array of pointers to the struct
typedef enum {false, true} bool;
//global to keep track of the currently loaded room
int currentRoomNum = 1;
//global array of rooms
struct room rooms[100];

//this function displays the current room
//for the sake of this assignment, we're assuming that there aren't any problems in the map file
void loadRoom(int roomID)
{
	//check proper roomID
	if (roomID != -1)
	{
		//sets the current room#
		currentRoomNum = roomID;
	}
	//roomID is a direction they can't go, print error and return
	else
	{
		printf("You can't go there.\n");
		return;
	}

	//print room description, the rooms array starts at 0, so we need to offset our room#
	printf("==================================================================\n");
	printf("%s\n", rooms[currentRoomNum - 1].desc);
	printf("==================================================================\n");
	printf("The available directions to you are: ");

	if (rooms[currentRoomNum - 1].north != -1) printf("north ");
	if (rooms[currentRoomNum - 1].south != -1) printf("south ");
	if (rooms[currentRoomNum - 1].east != -1) printf("east ");
	if (rooms[currentRoomNum - 1].west != -1) printf("west");

	printf("\n");
}

//load dungeon function
bool loaddungeon(char string[])
{
	//open the dungeon file
	FILE *dungeonFile = fopen(string, "r");
	
	//error check
	if (!dungeonFile)
	{
		printf("Error opening file. Check your spelling/dungeon file and try again.\n");
		return false;
	}

	//this is the buffer that will hold each line of the file for tokenization
	//each dungeon line is limited to 1024 bytes/characters, extra byte for null char
	char dungeonFileLine[1025];

	//if succeed then iterate through the file and populate an array of rooms with the data
	char *token; //this is our token

	//holds the position at which we are loading each line into our struct array
	int i = 0;

	//Iterate through the dungeon file
	while (fgets(dungeonFileLine, sizeof(dungeonFileLine), dungeonFile) != NULL)
	{
		//get the first token
		token = strtok(dungeonFileLine, " ");

		//procedurally iterate through the loaded dungeon file line
		//collect the description
		token = strtok(NULL, "$");

		//store the room description
		strcpy(rooms[i].desc, token);

		//parse the north room
		token = strtok(NULL, " ");

		//store the north room
		rooms[i].north = atoi(token);

		//parse the south room
		token = strtok(NULL, " ");

		//store the south room
		rooms[i].south = atoi(token);

		//parse the east room
		token = strtok(NULL, " ");

		//store the east room
		rooms[i].east = atoi(token);

		//parse the west room
		token = strtok(NULL, "\n");

		//store the west room
		rooms[i].west = atoi(token);

		//increment counter
		i++;
	}

	//close the file
	fclose(dungeonFile);

	printf("Load dungeon run successfully.\n");

	//print the first room
	loadRoom(currentRoomNum);

	//return
	return true;
}

int main()
{
	//this flag will switch to true when we want to exit the program
	bool quit = false;
	bool fileLoaded = false;

	//the player's entire line input
	char input[256];
	//token array to hold tokens parsed from input
	char *token;

	printf("==================================================================\n");
	printf("Text Commands:\n\nloaddungeon [filename] - loads the dungeon specified by [filename]\nquit - quits the game\n\n");
	printf("Once a dungeon is loaded, the following commands become available:\n");
	printf("north - travel north\nsouth - travel south\neast - travel east\nwest - travel west\n\n");
	printf("Please note:\n-User input is limited to 255 characters.\n");
	printf("-Dungeon size is limited to 100 rooms.\n");
	printf("-Room size is limited to 1023 characters.\n\n");
	printf("Please remember that you may 'quit' the program at any time.\n\n");
	printf("Thank you and enjoy your stay!\n");
	printf("==================================================================\n");

	//game loop
	while (!quit)
	{
		//command prompt
		printf("$ ");

		//read player's input
		fgets(input, 256, stdin);

		//get the first token
		token = strtok(input," ");

		//react to input
		if (strcmp(token, "quit\n") == 0) quit = true;
		else if (strcmp(token, "loaddungeon") == 0 || strcmp(token, "loaddungeon\n") == 0)
		{
			//we use NULL to tell the tokenizer to continue working on the pre-existing string
			if (token = strtok(NULL, "\n"))
			{
				//finally load the dungeon file
				fileLoaded = loaddungeon(token);
			}
			else printf("Syntax is: 'loaddungeon [filename]', please try again.\n");
		}
		else if (fileLoaded) //these commands are only available if a file is loaded
		{
			if (strcmp(token, "north\n") == 0) loadRoom(rooms[currentRoomNum - 1].north);
			else if (strcmp(token, "south\n") == 0) loadRoom(rooms[currentRoomNum - 1].south);
			else if (strcmp(token, "east\n") == 0) loadRoom(rooms[currentRoomNum - 1].east);
			else if (strcmp(token, "west\n") == 0) loadRoom(rooms[currentRoomNum - 1].west);
		}
		else printf("That command was not recognized, please try again.\n");
	}

	return 0;
}