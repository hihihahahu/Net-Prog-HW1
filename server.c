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

//The function that do RRQ
//buffer stores "01 filename"
//child_sock is the socket using
//child_client is the address of the client
void read_data(char* buffer, int child_sock, struct sockaddr_in* child_client)
{
    FILE* fp;
    unsigned short* op_pointer;
    int block = 0;
    int done = 0;
    ssize_t data_size;
    int no_data = 0;
    char r_buffer[517];
    char new_buffer[517];
    socklen_t addr_len = sizeof(*child_client);
    op_pointer = (unsigned short*)r_buffer;
    *op_pointer = htons(4);
    *op_pointer = htons(block);
    block++;

    //Open the file
    op_pointer = (unsigned short*)buffer;
    char filename[256];
    strcpy(filename, buffer + 2);
    fp = fopen(filename, "r");

    //Error checking: file does not exist
    if (fp == NULL)
    {
        *op_pointer = htons(5);
        *(op_pointer + 1) = htons(1);
        *(op_pointer + 2) = htons(0);

        //Send error message, error code = 1
        sendto(child_sock, buffer, 5, 0, (struct sockaddr*) child_client, sizeof(*child_client));
        return;
    }


    op_pointer = (unsigned short*)new_buffer;

    //While still have package to send
    while (!done)
    {
        //op code = 3
        *op_pointer = htons(3);
        *(op_pointer + 1) = htons(block);
        int i = 0;

        //Read 512 characters from the file
        int num_get = fread(new_buffer + 4, sizeof(char), 512, fp);

        //End of file
        if (num_get < 512) {done = 1;}

        printf("buffer: %d\n", new_buffer[1]);

        //Send the package to the server and increment block
        printf("result: %ld\n", sendto(child_sock, new_buffer, num_get + 4, 0, (struct sockaddr*) child_client, sizeof(*child_client)));
        block++;

        //Deal with timeout
        while (1)
        {
            //Try receiving ack from client, waiting for 1s
            data_size = recvfrom(child_sock, r_buffer, 517, 0, (struct sockaddr*) child_client, &addr_len);
            //If ack received, continue sending
            if (!(data_size < 0)) {break;}
            no_data++;

            //Otherwise, try to resend the last package
            sendto(child_sock, new_buffer, num_get + 4, 0, (struct sockaddr*) child_client, sizeof(*child_client));

            //If 10s timeout, terminate the connection
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
    //send ack packets & read data
    while(more_packet){
        //printf("gg");
        data_len = recvfrom(socket, buffer, 517, 0, (struct sockaddr*) sock_info, &addr_len);
        printf("block: %d\n", (unsigned short)*(buffer+2));
        //timeout detection
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
        timeout = 0;
        printf("%ld\n", data_len - 4);
        
        
        //last packet
        if(data_len < 516){
            more_packet = false;
        }
        //write to file if there is data
        if(data_len - 4 > 0){
            buffer[data_len] = '\0';
            fprintf(fp, "%s", buffer+4);
        }
        
        
        block++;

        //ack
        *op_pointer = htons(4);
        *(op_pointer + 1) = htons(block);
        //send ack pack
        for(int a = 0; a < 4; a++){
            last_packet[a] = buffer[a];
        }
        last_packet_size = 4;
        
        printf("ack: %ld\n", sendto(socket, buffer, 4, 0, (struct sockaddr*) sock_info, sizeof(*sock_info)));
        
    }
    printf("write finished.\n");
    fclose(fp);
}

//main function
//argv indicates range of port
int main (int argc, char** argv){
    int sock;
    socklen_t addr_len;
    int tid_selection;
    struct sockaddr_in server;
    struct sockaddr_in client;

    struct timeval timeout;

    //default is 8080, but will be replaced with user input
    int port = 8080;

    //Create buffer to store package
    char* buffer;
    buffer = calloc(517, sizeof(char));

    //Set the port range
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

    //Set the address of the server
    bzero(&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Set up socket
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket (parent).\n");
        return 0;
    }

    //Bind socket to port
    if(bind(sock, (struct sockaddr *) &server, addr_len) < 0){
        printf("Could not bind socket to server (parent).\n");
        return 0;
    }
    getsockname(sock, (struct sockaddr *) &server, &addr_len);
    int count = 0;
    unsigned short op_code;

    //While loop for server
    while(1){
        printf("waiting\n");
        count++;
        //struct sockaddr_in client;
        //unsigned short block_num;
        unsigned short* op_pointer;

        //Waiting for connection request from client
        recvfrom(sock, buffer, 517, 0, (struct sockaddr*) &client, &addr_len);

        //Get the op code
        op_pointer = (unsigned short*)buffer;
        op_code = ntohs(*op_pointer);

        //fork
        if(fork() == 0){
            printf("child #%d\n", count);
            //child go to handle request
            close(sock);
            break;
        }
        else{
            //parent continue in loop
            //use the next highest port
            tid_selection--;
        }
    }

    struct sockaddr_in child_server;
    int tid = tid_selection;
    int child_sock;
    printf("parent port: %d, tid: %d\n", port, tid);
    printf("op_code: %d\n", op_code);
    addr_len = sizeof(child_server);

    //Set address of the child server
    bzero(&child_server, sizeof(struct sockaddr_in));
    child_server.sin_family = AF_INET;
    child_server.sin_port = htons(tid);
    child_server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Set child socket
    if((child_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Could not create socket (child).\n");
        return 0;
    }

    //Bind child socket to corresponding port
    if(bind(child_sock, (struct sockaddr *) &child_server, addr_len) < 0){
        printf("Could not bind socket to server (child).\n");
        return 0;
    }

    //set timeout = 1s
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(child_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    //RRQ
    if(op_code == 1){
        printf("RRQ\n");
        read_data(buffer, child_sock, &client);
    }

    //WRQ
    if(op_code == 2){
        printf("WRQ\n");
        write_data(child_sock, &client, buffer);
    }

    //Terminate connection
    close(child_sock);
    exit(0);
}
