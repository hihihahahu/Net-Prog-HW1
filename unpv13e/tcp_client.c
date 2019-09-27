//
//  tcp_client.c
//  
//
//  Created by borute on 9/26/19.
//

#include <stdio.h>
#include "lib/unp.h"

int main(){
    int sock;
    int connection_count = 0;
    struct sockaddr_in server, client;
    if(sock = socket(AF_INET, SOCK_STREAM, 0) < 0){
        printf("Socket creation failed.\n");
        return 0;
    }
    bzero(&server, sizeof(server));
    
    printf("Enter IP address to connect to: ");
    char ip_addr[64];
    fgets(ip_addr, 64, stdin);
    
    while(1){
        int port = -1;
        scanf("%d", &port);
        if(connection_count < 5){
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = inet_addr(ip_addr);
            server.sin_port = htons(port);
            
        }
        
    }
    
    
}
