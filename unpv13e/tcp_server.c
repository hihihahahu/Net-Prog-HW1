//
//  tcp_server.c
//  
//
//  Created by borute on 9/26/19.
//

#include <stdio.h>
#include <stdbool.h>
#include "lib/unp.h"



int main(int argc, char** argv){
	int a;
    int sock;

    fd_set fdset;
    struct timeval timeout = {0, 3};

    struct sockaddr_in server, client;
    struct timeval tv;
    
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("Please enter a number (this number + 9877 will be the port which this server listens to): ");
    int port;
    port = atoi(argv[1]);
    port += 9877;
    if(sock == -1){
        printf("Socket creation failed.\n");
        return 0;
    }
    bzero(&server, sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if(bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0){
        printf("Socket binding failed.\n");
        return 0;
    }
    if(listen(sock, 5) < 0){
        printf("Listen failed.\n");
        return 0;
    }
    printf("Listening on port %d.\n", port);
    bool quit = false;

    FD_ZERO(&fdset);
    FD_SET(0, &fdset);

    while(!quit){
        printf("Waiting for connection.\n");
        int client_connection;
        socklen_t clilen = sizeof(client);
        client_connection = accept(sock, (struct sockaddr*) &client, &clilen);
        printf("Accepted connection.\n");
        while(1){
            FD_ZERO(&fdset);
            FD_SET(0, &fdset);
            timeout.tv_sec = 0;
            timeout.tv_usec = 3;
            char* str = calloc(513, sizeof(char));
            char temp[2];
            /*
            temp[0] = '\0';
            temp[1] = 'g';
            //close(client_connection);
            //printf("%ld\n", write(client_connection, str, 2));
            if(write(client_connection, temp, 2) < 2){
                printf("Client disconnected.\n");
                break;
            }
             */
            fflush(stdout);
            char recvline[MAXLINE];
            if (Readline(client_connection, recvline, MAXLINE) == 0){
                printf("str_cli: client disconnected\n");
                break;
            }
            select(1, &fdset, NULL, NULL, &timeout);
            
            if (FD_ISSET(0, &fdset)){
                    //printf("fuck you\n");
                size_t temp = 512;
                fgets(str, temp, stdin);
                //if (str[0] == '\0') {continue;}
                //printf("114514\n");
                a = strlen(str);
                //str[a] = '\0';
                write(client_connection, str, a);
            }
            
            else if(feof(stdin)){
                str[0] = '\0';
                str[1] = '\0';
                write(client_connection, str, 2);
                printf("Shutting down due to EOF.\n");
                close(sock);
                close(client_connection);
                quit = true;
                break;
            }
            
            
        }
    }
    return 0;
}
