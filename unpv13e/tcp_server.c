//
//  tcp_server.c
//  
//
//  Created by borute on 9/26/19.
//

#include <stdio.h>
#include "lib/unp.h"

int main(){
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
    while(1){
        printf("Waiting for connection.");
        int client_connection;
        socklen_t clilen = sizeof(client);
        client_connection = accept(sock, (struct sockaddr*) &client, &clilen);
        printf("Accepted connection.\n");
        while(1){
            char str[513];
            if(fgets(str, 512, stdin)){
                int a = 0;
                while((str[a++] = getchar()) != '\n' || (str[a++] = getchar()) != '\0'){}
                write(sock, str, a+1);
            }
            else{
                printf("stdin error.\n");
                return 0;
            }
            
        }
        
        
    }
}
