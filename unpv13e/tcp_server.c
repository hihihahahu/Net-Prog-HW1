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
    struct sockaddr_in server, client;
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
            
            if((a = scanf("%s", str)) > 0){
            	//printf("114514\n");
                str[a] = '\0';
                write(client_connection, str, a+1);
            }
            else if(feof(stdin)){
                printf("Shutting down due to EOF.\n");
                close(sock);
                close(client_connection);
                quit = true;
                break;
            }
            else{
                printf("stdin error.\n");
                return 0;
            }
            
        }
        
        
    }
    return 0;
}
