#include <stdlib.h> //gives us malloc
#include <string.h> //gives us string commands
#include <stdio.h> //gives us stdin access
#include <ctype.h> //gives us isdigit()
#include <stdint.h> //gives us uint8_t


//create our bool type
typedef enum {false, true} bool; 

////////////// DISCLAIMER: THE isNumber() FUNCTION IS NOT MINE //////////////
//This function is credit to niyasc on stackoverflow, I take no credit for this function (though I wish I'd thought of it)//
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

//global heap pointer
uint8_t *heap;
//global numBlocks
uint8_t numBlocks = 0;

//we have 6 fuctnions
//allocates a given # of bytes and prints the block number of the new block
void allocate(int numBytes)
{
    //pointer for traversing the heap
    uint8_t *ptr = heap;

    //need to really scrutinize this algorithm
    while ((ptr < heap + 126) /* while our ptr is before heap + 126, which is heap[126], which is space #127 out of 128 spaces,
    which means as long as there are 3 spaces left at least */

    && /* and */
    
    ((*(ptr+1) & 1) /* block is allocated */
    
    || /* or */
    
    (((*(ptr+1) >> 1) + 1) < numBytes + 2))) /* size of currnet block is less than the size we're looking for + header*/
    
    ptr = ptr + (*(ptr+1) >> 1) + 1; /* increment by size + 1, to get next block's header byte 1 */ //2-complement mask out allocation bit

    //check to see if we went beyond search range, if so no block was found, otherwise we found a block
    if (ptr > heap + 125) //need at least 3 blocks for an allocation, ptr[125] is the last viable new block position, with payload 1
    {
        printf("Reached end of heap, not enough bytes available.\n");
        return;
    }
    //otherwise we found enough space!
    //now is it the right size? or is it too big?
    //if its too big, then split it
    if (numBytes + 2 > (*(ptr+1) >> 1) +1 )
    {
        printf("\nSomehow an error occured and we don't have enough space despite attempting to allocate.");
        return;
    }
    else if (numBytes + 2 < (*(ptr+1) >> 1) +1 )
    {
        //the space we found is larger than our need, we need to split it, but if the extra piece is 2 or smaller, give it to this block
        int originalSize = (*(ptr+1) >> 1) + 1;
        int newSize = numBytes + 2;

        //if there's less than 3 spaces remaining after this allocation, give those spaces to this allocation
        if (originalSize - newSize < 3)
        {
            newSize = originalSize;

            //no splitting required now
            //Increment and assign the block number into header byte 1
            *ptr = ++numBlocks;

            //assign the blocksize newSize - 1, shift left, and set the allocated bit into header byte 2
            *(ptr + 1) = ((newSize - 1) << 1) | 1;
        }
        else //still need to perform splitting
        {
            int remainderSize = originalSize - newSize;

            //Increment and assign the block number into header byte 1
            *ptr = ++numBlocks;

            //assign the blocksize newSize - 1, shift left, and set the allocated bit into header byte 2
            *(ptr + 1) = ((newSize - 1) << 1) | 1;

            //assign the blocksize remainderSize -1, shift left, set allocated to 0, and store in the size of the newly split remainder blockList
            *(ptr+1+newSize) = ((remainderSize - 1) << 1) & -2;
        }
    }
    else //the space we're looking at is exactly the size we need!
    {
        //Increment and assign the block number into header byte 1
        *ptr = ++numBlocks;

        //assign the blocksize (request size + 2) - 1 and the allocated bit into header byte 2
        *(ptr + 1) = ((numBytes + 1) << 1) | 1;
    }
    
    //print the block number that we just allocated
    printf("%i\n", numBlocks);
}

//frees a given block number
void freeBlock(uint8_t blockNum)
{
    //pointer for traversing the heap
    uint8_t *ptr = heap;

    while ((ptr < heap + 126) && (*ptr != blockNum)) { ptr = ptr + (*(ptr+1) >> 1) + 1; } /* increment by size + 1, to get next block's header byte 1 */

    //make sure we didn't overextend, if we did, then we didn't find anything
    if (ptr > heap + 125)
    {
        printf("Reached end of heap, block number not found.\n");
        return;
    }

    //if we're here, then we successfully found the target!
    *(ptr+1) &= -2; //bitwise and with -2 (1111 1110) to keep size, but set allocation bit to 0
}

//prints information about all blocks in heap
//size, allocation status, start address, end address; addresses in hex
void blockList()
{
    //pointer for traversing the heap
    uint8_t *ptr = heap;

    //print initial formatting text
    printf("Size    Allocated  Start          End\n");

    //variables for holding block info
    int size, allocated;

    //travel the entire heap
    while (ptr < heap + 126)
    {
        size = (*(ptr+1) >> 1) + 1;

        if (*(ptr+1) & 1) allocated = 1;
        else allocated = 0;

        //print current block's info
        printf("%-7i %-10s %-14p %p\n", size, (allocated == 1) ? "Yes" : "No", ptr, ptr+size-1);

        //move to next block
        ptr = ptr + (*(ptr+1) >> 1) + 1;
    }
}

//prints the given character numWrites times into the block specified by blockNum
void writeHeap(uint8_t blockNum, char character, uint8_t numWrites)
{
    //pointer for traversing the heap
    uint8_t *ptr = heap;

    while ((ptr < heap + 126) && (*ptr != blockNum)) { ptr = ptr + (*(ptr+1) >> 1) + 1; } /* increment by size + 1, to get next block's header byte 1 */

    //make sure we didn't overextend, if we did, then we didn't find anything
    if (ptr > heap + 125)
    {
        printf("Reached end of heap, block number not found.\n");
        return;
    }
    else
    {
        //if we're here, we found the block in question, so take its size
        uint8_t blockSize = (*(ptr+1) >> 1) + 1;

        //make sure the block payload is big enough to hold the requested writes
        if (blockSize - 2 < numWrites)
        {
            printf("Write too big\n");
            return;
        }
        else
        {
            uint8_t i = 0;

            for (;i < numWrites; i++)
            {
                //write the character in each spot of the payload, numWrites times, adjusting for header
                *(ptr+2+i) = (uint8_t) character;
            }
        }
    }
}

//Prints numBytes from block specified by blockNum
void printHeap(uint8_t blockNum, uint8_t numBytes)
{
    //pointer for traversing the heap
    uint8_t *ptr = heap;

    while ((ptr < heap + 126) && (*ptr != blockNum)) { ptr = ptr + (*(ptr+1) >> 1) + 1; } /* increment by size + 1, to get next block's header byte 1 */

    //make sure we didn't overextend, if we did, then we didn't find anything
    if (ptr > heap + 125)
    {
        printf("Reached end of heap, block number not found.\n");
        return;
    }
    else
    {
        //I'd normally ensure the memory being accessed is only our block, but the assignment explicitly says not to check
        uint8_t i = 0;

        for (;i < numBytes; i++)
        {
            printf("%c", (char)(*(ptr+2+i)));
        }

        //end with newline
        printf("\n");
    }

}

//prints the 2-byte header of block given by blockNum
void printHeader(uint8_t blockNum)
{
    //pointer for traversing the heap
    uint8_t *ptr = heap;

    while ((ptr < heap + 126) && (*ptr != blockNum)) { ptr = ptr + (*(ptr+1) >> 1) + 1; } /* increment by size + 1, to get next block's header byte 1 */

    //make sure we didn't overextend, if we did, then we didn't find anything
    if (ptr > heap + 125)
    {
        printf("Reached end of heap, block number not found.\n");
        return;
    }
    else
    {
        //print the header in hex
        printf("0x%02X 0x%02X\n",*ptr, *(ptr+1));
    }

}

int main(int argc, char *argv[])
{
    //print out initial instructions for use
    printf("==================================================================\n\n");
    printf("This application allocates 128 bytes (126 usable) of heap memory space that can be accessed and manipulated by the user using the below commands.\n\n");
    printf("==================================================================\n\n");
    printf("Text Commands:\n\nallocate [# of Bytes] - Allocates a block on our heap of [# of Bytes] size and returns the block number associated with it\n");
    printf("\nfree [Block #] - Frees the block of memory declared by [Block #]\n");
    printf("\nblocklist - This command takes no arguments, and prints information about every currently allocated block\n");
    printf("\nwriteheap [Block #] [Character to Write] [# of writes] - This writes [# of writes] copies of [Character to write] into block [Block #]\n");
    printf("\nprintheap [Block #] [# bytes to print] - Prints [# bytes to print] bytes from block [Block #]\n");
    printf("\nprintheader [Block #] - prints the 2 byte header associated with block [Block #]\n");
    printf("\nquit - quits the program\n\n");
    printf("==================================================================\n\n");
    printf("IMPORTANT: Number inputs above 255 will have unexpected results and not function properly. Use 0-255 only!\n\n");
    printf("Please note:\nUser input is limited to 255 characters and the commands are case sensitive.\n");
    printf("Please remember that you may 'quit' the program at any time.\n\n");
    printf("Thank you and enjoy your stay!\n\n");
    printf("==================================================================\n");
    
    //malloc the original heap
    heap = (uint8_t*) malloc(128 * sizeof(uint8_t)); //128 bytes

    //error check
    if (heap == NULL)
    {
        perror("Malloc Error");
        exit(1);
    }

    //build our initial header
    *heap = numBlocks; //block 0 = 0000 0000
    *(heap + 1) = 254; //block 1 = 1111 1110, which is 127 when you shift right one, which is 128 spots (0-127)


    //our loop flag
    bool quit = false;

    //string to hold our raw input
    char input[256];

    //create a token to hold parsed input
    char *token;

    //create our delimiters
    char delimiters[] = " \t\r\n\v\f";

    //program loop
    while (!quit)
    {
        //print out command prompt
        printf(">");

        //get the raw input
        fgets(input, 256, stdin);


        //parse the first token
        token = strtok(input, delimiters);

        //if token isn't NULL then proceed'
        if (token != NULL)
        {
            if (strcmp(token, "allocate") == 0)
            {
                //get argument and ignore anything else
                token = strtok(NULL, delimiters);

                //error checking, call allocate if successful input
                if (token == NULL) { printf("Could not parse argument, please try again.\n"); }
                else if (isNumber(token)) { allocate(atoi(token)); }
                else { printf("Expected an integer as command argument, please try again.\n"); }
            }
            else if (strcmp(token, "free") == 0)
            {
                //get argument and ignore anything else
                token = strtok(NULL, delimiters);

                //error checking, call allocate if successful input
                if (token == NULL) { printf("Could not parse argument, please try again.\n"); }
                else if (isNumber(token)) { freeBlock(atoi(token)); }
                else { printf("Expected an integer as command argument, please try again.\n"); }
            }
            else if (strcmp(token, "blocklist") == 0) { blockList(); }
            else if (strcmp(token, "writeheap") == 0)
            {
                //vars to hold arguments
                int blockNum, writeNum;
                char writeChar;

                //get 1st argument
                token = strtok(NULL, delimiters);

                //error checking, call allocate if successful input
                if (token == NULL) { printf("Could not parse 1st argument, please try again.\n"); }
                else if (!isNumber(token)) { printf("Expected an integer as 1st command argument, please try again.\n"); }
                else 
                {
                    //store first arg
                    blockNum = atoi(token);

                    //get 2nd argument
                    token = strtok(NULL, delimiters);

                    if (token == NULL) { printf("Could not parse 2nd argument, please try again.\n"); }
                    else if (isNumber(token)) { printf("Expected a character as 2nd command argument, please try again.\n"); }
                    else
                    {
                        //store second arg
                        writeChar = *token;

                        //get 3rd argument
                        token = strtok(NULL, delimiters);

                        if (token == NULL) { printf("Could not parse 3rd argument, please try again.\n"); }
                        else if (!isNumber(token)) { printf("Expected an integer as 3rd command argument, please try again.\n"); }
                        else
                        {
                            //store third arg
                            writeNum = atoi(token);

                            //call writeHeap
                            writeHeap(blockNum, writeChar, writeNum);
                        }
                    }
                }
            }
            else if (strcmp(token, "printheap") == 0)
            {
                //vars to hold arguments
                int blockNum, printNum;

                //get 1st argument
                token = strtok(NULL, delimiters);

                //error checking, call allocate if successful input
                if (token == NULL) { printf("Could not parse 1st argument, please try again.\n"); }
                else if (!isNumber(token)) { printf("Expected an integer as 1st command argument, please try again.\n"); }
                else 
                {
                    //store 1st arg
                    blockNum = atoi(token);

                    //get 2nd argument
                    token = strtok(NULL, delimiters);

                    //error checking, call allocate if successful input
                    if (token == NULL) { printf("Could not parse 2nd argument, please try again.\n"); }
                    else if (!isNumber(token)) { printf("Expected an integer as 2nd command argument, please try again.\n"); }
                    else
                    {
                        //store 2nd arg
                        printNum = atoi(token);

                        //call printHeap
                        printHeap(blockNum, printNum);
                    }

                }

            }
            else if (strcmp(token, "printheader") == 0)
            {
                //get argument and ignore anything else
                token = strtok(NULL, delimiters);

                //error checking, call allocate if successful input
                if (token == NULL) { printf("Could not parse argument, please try again.\n"); }
                else if (isNumber(token)) { printHeader(atoi(token)); }
                else { printf("Expected an integer as command argument, please try again.\n"); }
            }
            else if (strcmp(token, "quit") == 0)
            {
                quit = true;
            }
            else
            {
                printf("Error: That wasn't a valid command, please check your syntax and try again.\n");
            }

        } //error catching
        else printf("Error: Unable to parse text, please try again.\n");        
    }
    
    //clean up our heap
    free(heap);

    //exit
    return 0;
}