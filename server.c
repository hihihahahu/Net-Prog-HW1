//
//  server.c
//  
//
//  Created by borute on 9/24/19.
//

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include "unpv13e/lib/unp.h"


void write_data(int socket, struct sockaddr_in* sock_info, char* buffer){
    int tid = ntohs(sock_info -> sin_port);
    unsigned short* op_pointer = (unsigned short*)buffer;
    char file[256];
    FILE* fp;
    socklen_t addr_len;
    char last_packet[517];
    int last_packet_size = 0;
    ssize_t data_len;
    int timeout = 0;
    
    strcpy(file, buffer+2);
    fp = fopen(file, "w");
    //ack packet
    *op_pointer = htons(4);
    *(op_pointer + 1) = htons(0);
    
    for(int a = 0; a < 4; a++){
        last_packet[a] = buffer[a];
    }
    last_packet_size = 4;
    
    sendto(socket, buffer, 4, 0, (struct sockaddr*) sock_info, sizeof(*sock_info));
    
    bool more_packet = true;
    
    while(more_packet){
        data_len = recvfrom(socket, buffer, 517, 0, (struct sockaddr*) sock_info, &addr_len);
        if(data_len < 0 && errno == EWOULDBLOCK){
            timeout++;
            if(timeout >= 10){
                printf("Connection timed out.\n");
                break;
            }
            //try resending the last packet
            for(int a = 0; a < 517; a++){
                buffer[a] = last_packet[a];
            }
            sendto(socket, buffer, last_packet_size, 0, (struct sockaddr*) sock_info, sizeof(*sock_info));
            continue;
        }
        //check tid
        if(ntohs(sock_info -> sin_port) != tid){
            *op_pointer = htons(5);
            *(op_pointer + 1) = htons(5);
            *(buffer + 4) = 0;
            sendto(socket, buffer, 5, 0, (struct sockaddr*) sock_info, sizeof(*sock_info));
            continue;
        }
        timeout = 0;
        
        if(ntohs(*op_pointer) != 3){
            if(ntohs(*op_pointer) == 2){
                //try resending the last packet
                for(int a = 0; a < 517; a++){
                    buffer[a] = last_packet[a];
                }
                sendto(socket, buffer, last_packet_size, 0, (struct sockaddr*) sock_info, sizeof(*sock_info));
            }
            continue;
        }
        
        buffer[data_len] = '\0';
        //last packet
        if(data_len < 516){
            more_packet = false;
        }
        //ack
        *op_pointer = htons(4);
        *(op_pointer + 1) = htons(0);
        
        for(int a = 0; a < 4; a++){
            last_packet[a] = buffer[a];
        }
        last_packet_size = 4;
        
        sendto(socket, buffer, 4, 0, (struct sockaddr*) sock_info, sizeof(*sock_info));
        
    }
    fclose(fp);
}

int main (int argc, char** argv){
    int sock;
    socklen_t addr_len;
    int tid_selection;
    struct sockaddr_in server;
    struct timeval timeout;
    //default is 8080, but will be replaced with user input
    int port = 8080;
    char* buffer;
    buffer = calloc(517, sizeof(char));
    if(argc == 3){
        port = atoi(argv[1]);
        tid_selection = atoi(argv[2]);
    }
    else{
        printf("Number of arguments is not 3.\n");
        return 0;
    }
    if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket.\n");
        return 0;
    }
    if((bind(sock, (struct sockaddr *) &server, sizeof(server))) < 0){
        printf("Could not bind socket to server.\n");
        return 0;
    }
    bzero(&server, sizeof(server));
    server.sin_family = PF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    bool done = false;
    //int pid = getpid();
    unsigned short op_code;
    while(!done){
        struct sockaddr_in client;
        //unsigned short block_num;
        unsigned short* op_pointer;
        recvfrom(sock, buffer, 517, 0, (struct sockaddr*) &client, &addr_len);
        op_pointer = (unsigned short*)buffer;
        op_code = ntohs(*op_pointer);
        
            if(fork() == 0){
                //child go to handle request
                close(sock);
                break;
            }
            else{
                tid_selection--;
            }
        
    }
    struct sockaddr_in child_server;
    int tid = tid_selection;
    int child_sock;
    //tid_selection--;
    child_server.sin_family = PF_INET;
    child_server.sin_port = htons(tid);
    child_server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((child_sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket.\n");
        return 0;
    }
    if((bind(child_sock, (struct sockaddr *) &child_sock, sizeof(child_sock))) < 0){
        printf("Could not bind socket to server.\n");
        return 0;
    }
    //set timeout
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(child_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    if(op_code == 1){
        //RRQ
        
    }
    if(op_code == 2){
        //WRQ
    }
    

}
