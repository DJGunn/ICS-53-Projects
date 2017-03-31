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

/*function provided by professor as part of assignment*/
int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /*create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {return -1;}

    /*eliminates 'address already in use' error from bind*/
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int)) < 0) {return -1;}

    /*listenfd will be an endpoint for all requests to port on any IP address for this host*/
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) port);

    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {return -1;}

    /* make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 10) < 0) {return -1;} //make it so we can queue 10 connections

    return listenfd;
}


int main(int argc, char *argv[])
{
    //print out initial instructions for use
    printf("==================================================================\n\n");
    printf("Client/Server Address Book: Server\n\n");
    printf("==================================================================\n\n");
    printf("Usage:\n\nWhen launching the program, include a port number as one of the arguments.");
    printf("The server is automated. It compares strings sent to it against an address book, and replies with names associated with the entry.\n");
    printf("\nPressing Ctrl + C will quit the program.\n\n");
    printf("==================================================================\n\n");
    printf("Please note:\n-User input is limited to 255 characters and the server can only queue up to 10 connections.\n");
    printf("Thank you and enjoy your stay!\n\n");
    printf("==================================================================\n");
    printf("Address server started\n");

    char payload[256]; //buffer to hold payload received and sent
    char buffer[255]; //buffer to hold address
    
    /*code below provided by professor as part of assignment*/

    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    unsigned short client_port;

    port = atoi(argv[1]);

    listenfd = open_listenfd(port);

    while (true)
    {

        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        hp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);

        haddrp = inet_ntoa(clientaddr.sin_addr);
        client_port = ntohs(clientaddr.sin_port);

        printf("server connected to %s (%s), port %u\n", hp->h_name, haddrp, client_port);

        /*end professor's code*/
        while (read(connfd, payload, 255) > 0) //for as long as the connection is alive
        {
            int i = 0;

            for (;i < (int) payload[0]; i++)
            { 
                buffer[i] = payload[i+1];    
            }

            buffer[i] = '\0';

            printf("%s\n", buffer); //print received payload

            //compare received string vs our dictionary
            //if we get a "+++" this means client has quit, so drop connection
            if (strcmp(buffer, "harris@ics.uci.edu") == 0) 
            {
                payload[0] = (unsigned) strlen("Ian G. Harris");
                strcpy(buffer, "Ian G. Harris");
            }
            else if (strcmp(buffer, "joe@cnn.com") == 0)
            {
                payload[0] = (unsigned) strlen("Joe Smith");
                strcpy(buffer, "Joe Smith");
            }
            else if (strcmp(buffer, "jane@slashdot.org") == 0)
            {
                payload[0] = (unsigned) strlen("Jane Smith");
                strcpy(buffer, "Jane Smith");
            }
            else if (strcmp(buffer, "+++") == 0) break;
            else
            {
                payload[0] = (unsigned) strlen("unknown");
                strcpy(buffer, "unknown");
            }

            payload[1] = '\0';

            strcat(payload, buffer);

            write(connfd, payload, strlen(payload) * sizeof(char)); //send back the name found in address book, or unknown

            read(connfd, payload, 1); // get last byte
        }


        close(connfd); //last line of professor's code
    }    

    //exit
    return 0;
}