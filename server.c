//
//  server.c
//  
//
//  Created by borute on 9/24/19.
//

#include <stdio.h>
#include <unpv13e/lib/unp.h>


int main (int argc, char** argv){
    int socket;
    socklen_t addr_len;
    int tid_selection;
    struct sockaddr_in server;
    //default is 8080, but will be replaced with user input
    int port = 8080;
    if(argc == 3){
        port = atoi(argv[1]);
        tid_selection = atoi(argv[2]);
    }
    else{
        printf("Number of arguments is not 3.\n");
        return 0;
    }
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket.\n");
        return 0;
    }
    bzero(&server, sizeof(server));
    server.sin_family = PF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    bool done = false;
    int pid = getpid();
    while(!done){
        struct sockaddr_in client;
        unsigned short op_cpde;
        unsigned short block_num;
        unsigned short* op_pointer;
        char* buffer;
        buffer = calloc(517, sizeof(char));
        int data_size = recvfrom(sock, buffer, 1024, 0, &client, &addr_len);
        op_pointer = (unsigned short*)buffer;
        op_code = ntohs(*op_pointer);
        else{
            if(fork() == 0){
                //child go to handle request
                close(sock);
                break;
            }
        }
    }
    struct sockaddr_in child_sock;
    int tid = tid_selection;
    tid_selection--;
    child_sock.sin_family = PF_INET;
    child_sock.sin_port = htons(tid);
    child_sock.sin_addr.s_addr = htonl(INADDR_ANY);
}
