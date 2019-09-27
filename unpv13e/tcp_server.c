//
//  tcp_server.c
//  
//
//  Created by borute on 9/26/19.
//

#include <stdio.h>
#include <stdbool.h>
#include "lib/unp.h"

int main(){
	int a;
    int sock;
    fd_set fdset;
    struct sockaddr_in server, client;
    struct timeval tv;
    
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("Please enter a number (this number + 9877 will be the port which this server listens to): ");
    int port;
    scanf("%d", &port);
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
    FD_SET(stdin, &fdset);
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    
    while(!quit){
        printf("Waiting for connection.\n");
        int client_connection;
        socklen_t clilen = sizeof(client);
        client_connection = accept(sock, (struct sockaddr*) &client, &clilen);
        printf("Accepted connection.\n");
        while(1){
            char str[513];
            if(write(client_connection, str, 0) < 0){
                printf("Client disconnected.\n");
                break;
            }
            select(stdin + 1, &fdset, NULL, NULL, &tv);
            if(FD_ISSET(stdin, &fdset)){
            	//printf("114514\n");
                scanf("%s", str);
                a = strlen(str);
                str[a] = '\0';
                //printf("a = %d\n", a);
                write(client_connection, str, a);
            }
            else if(feof(stdin)){
                str[0] = 0;
                str[1] = 0;
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
