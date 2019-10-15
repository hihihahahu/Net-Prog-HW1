//
//  hw2_server.c
//  
//
//  Created by borute on 10/15/19.
//

#include "hw2_server.h"
#include "lib/unp.h"
#include <stdio.h>

int main(int argc, char* argv[]){
    fd_set fds;
    FD_ZERO(&fds);
    
    int listenfd;
    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    int player_count = 0;
    
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("socket create failed");
        return 0;
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[2]));
    
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 50);
    
    while(1){
        
    }
}
