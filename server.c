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
#include <stdbool.h>
 
//#include "unp.h"

void read_data(char* buffer, int child_sock, struct sockaddr_in* child_server)
{
    FILE* fp;
    unsigned short* op_pointer;
    int block = 0;
    int done = 0;
    ssize_t data_size;
    int no_data = 0;
    char r_buffer[517];
    char new_buffer[517];
    socklen_t addr_len = sizeof(*child_server);
    op_pointer = (unsigned short*)r_buffer;
    *op_pointer = htons(4);
    *op_pointer = htons(block);
    block++;
    sendto(child_sock, r_buffer, 4, 0, (struct sockaddr*) child_server, sizeof(*child_server));

    op_pointer = (unsigned short*)buffer;
    char filename[256];
    strcpy(filename, buffer + 2);
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        *op_pointer = htons(5);
        *(op_pointer + 1) = htons(1);
        *(op_pointer + 2) = htons(0);
        sendto(child_sock, buffer, 5, 0, (struct sockaddr*) child_server, sizeof(*child_server));
        return;
    }


    op_pointer = (unsigned short*)new_buffer;
    while (!done)
    {
        *op_pointer = htons(3);
        *(op_pointer + 1) = htons(block);
        int i = 0;
        int num_get = fread(new_buffer + 4, sizeof(char), 512, fp);
        if (num_get < 512) {done = 1;}

        printf("buffer: %d\n", new_buffer[1]);
        printf("result: %ld\n", sendto(child_sock, new_buffer, num_get + 4, 0, (struct sockaddr*) child_server, sizeof(*child_server)));
        block++;

        while (1)
        {
            data_size = recvfrom(child_sock, r_buffer, 517, 0, (struct sockaddr*) child_server, &addr_len);
            if (!(data_size < 0)) {break;}
            no_data++;
            sendto(child_sock, new_buffer, num_get + 4, 0, (struct sockaddr*) child_server, sizeof(*child_server));
            if (no_data == 10) {
                done = 1;
                break;
            }
        }
    }
    fclose(fp);
    close(child_sock);
    exit(0);
}


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
    int block = 0;
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
        //printf("gg");
        data_len = recvfrom(socket, buffer, 517, 0, (struct sockaddr*) sock_info, &addr_len);
        printf("block: %d\n", (unsigned short)*(buffer+2));
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
        printf("%ld\n", data_len - 4);
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
        
        
        //last packet
        if(data_len < 516){
            more_packet = false;
        }
        if(data_len - 4 > 0){
            buffer[data_len] = '\0';
            fprintf(fp, "%s", buffer+4);
        }
        
        
        block++;

        //ack
        *op_pointer = htons(4);
        *(op_pointer + 1) = htons(block);
        
        for(int a = 0; a < 4; a++){
            last_packet[a] = buffer[a];
        }
        last_packet_size = 4;
        
        printf("ack: %ld\n", sendto(socket, buffer, 4, 0, (struct sockaddr*) sock_info, sizeof(*sock_info)));
        
    }
    printf("write finished.\n");
    fclose(fp);
}

int main (int argc, char** argv){
    int sock;
    socklen_t addr_len;
    int tid_selection;
    struct sockaddr_in server;
    struct timeval timeout;
    //char* buffer = calloc(517, sizeof(char));
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
    printf("port range (input): %d to %d.\n", atoi(argv[1]), atoi(argv[2]));
    printf("port range (saved): %d to %d.\n", port, tid_selection);
    addr_len = sizeof(server);
    bzero(&server, sizeof(struct sockaddr_in));
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket (parent).\n");
        return 0;
    }
    if(bind(sock, (struct sockaddr *) &server, addr_len) < 0){
        printf("Could not bind socket to server (parent).\n");
        return 0;
    }
    getsockname(sock, (struct sockaddr *) &server, &addr_len);
    //printf("%d\n", addr_len);
    int count = 0;
    //bool done = false;
    //int pid = getpid();
    unsigned short op_code;
    //printf("%s\n", inet_ntoa(server.sin_addr));
    while(1){
        printf("waiting\n");
        count++;
        //struct sockaddr_in client;
        //unsigned short block_num;
        unsigned short* op_pointer;
        recvfrom(sock, buffer, 517, 0, (struct sockaddr*) &server, &addr_len);
        op_pointer = (unsigned short*)buffer;
        op_code = ntohs(*op_pointer);
        if(fork() == 0){
            printf("child #%d\n", count);
            //child go to handle request
            close(sock);
            break;
        }
        else{
            tid_selection--;
        }
    }

    struct sockaddr_in client;
    //unsigned short block_num;
    unsigned short* op_pointer;
    recvfrom(sock, buffer, 517, 0, (struct sockaddr*) &client, &addr_len);
    op_pointer = (unsigned short*)buffer;
    op_code = ntohs(*op_pointer);

    struct sockaddr_in child_server;
    int tid = tid_selection;
    //tid_selection--;
    int child_sock;
    printf("parent port: %d, tid: %d\n", port, tid);
    printf("op_code: %d\n", op_code);
    addr_len = sizeof(child_server);
    bzero(&child_server, sizeof(struct sockaddr_in));
    //tid_selection--;
    child_server.sin_family = AF_INET;
    child_server.sin_port = htons(tid);
    child_server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((child_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket (child).\n");
        return 0;
    }
    if(bind(child_sock, (struct sockaddr *) &child_server, addr_len) < 0){
        printf("Could not bind socket to server (child).\n");
        return 0;
    }
    //set timeout
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(child_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    if(op_code == 1){
        //RRQ
        printf("RRQ\n");
        read_data(buffer, child_sock, &server);
    }
    if(op_code == 2){
        //WRQ
        printf("WRQ\n");
        write_data(child_sock, &server, buffer);
    }
    close(child_sock);
    exit(0);
    return 0;
}
