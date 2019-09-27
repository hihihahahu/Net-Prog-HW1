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
    struct sockaddr_in server, client;
    if(sock = socket(AF_INET, SOCK_STREAM, 0) < 0){
        printf("Socket creation failed.\n");
        return 0;
    }
    bzero(&server, sizeof(server));
    
    printf("Enter IP address to connect to: ");
    char ip_addr[64];
    fgets(ip_addr, 64, stdin);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = 
    
}
