#include <stdlib.h> //gives us malloc
#include <string.h> //gives us string commands
#include <stdio.h> //gives us stdin access
#include <ctype.h> //gives us isdigit()
#include <stdint.h> //gives us uint8_t
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> //heads from here to sys/types.h included on request of instructor

//create our bool type
typedef enum {false, true} bool; 

/*open_clientfd function provided by instructor*/
int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { return -1; } //check errno for cause of error

    //fill in servers IP and port
    if ((hp = gethostbyname(hostname)) == NULL) { return -2; } //check errno for error
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *) hp->h_addr_list[0], (char *) &serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    //establish connection with server
    if(connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {return -1;}

    return clientfd;
}


int main(int argc, char *argv[])
{
    //print out initial instructions for use
    printf("==================================================================\n\n");
    printf("Client/Server Address Book: Client\n\n");
    printf("==================================================================\n\n");
    printf("Usage:\n\nWhen launching the program, include a server domain and port number as space delineated arguments.");
    printf("Enter an e-mail address to look up. If that address is found in the server's database, the name associated with that e-mail will be returned.\n");
    printf("\nThe proper format is: [address]@[domain].[extension]\n");
    printf("\n\"+++\" - quits the program\n\n");
    printf("==================================================================\n\n");
    printf("Please note:\n-User input is limited to 255 characters.\n");
    printf("Thank you and enjoy your stay!\n\n");
    printf("==================================================================\n");
    
    //create file descriptor for server connection, and store server host name as well as port
    int network_fd, port;
    char* hostName = argv[1];
    port = atoi(argv[2]);

    //create 256 char buffer for payload and response
    char payload[256];
    char response[256];
    char printBuf[255];

    //connect
    network_fd = open_clientfd(hostName, port);

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
            if (strcmp(token, "+++") == 0) { quit = true; }
            
            //set byte 0 to the size of the String
            payload[0] = (unsigned)strlen(token);
            payload[1] = '\0';

            //concat the message into the payload
            strcat(payload, token);

            //send the message
            write(network_fd, payload, sizeof(payload));

            if (!quit)
            {
                //read response
                read(network_fd, response, sizeof(response));

                int i = 0;

                for (;i < (int) response[0]; i++)
                { 
                    printBuf[i] = response[i+1];    
                }

                printBuf[i] = '\0';

                //print response
                printf("%s\n", printBuf);
            }
            
        } //error catching
        else printf("Error: Unable to parse text, please try again.\n");        
    }
    

    //exit
    return 0;
}