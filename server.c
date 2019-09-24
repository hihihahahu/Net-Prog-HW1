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

void read_data(char* buffer, int child_sock, sockaddr_in* child_server)
{
	FILE* fp;
	unsigned short* op_pointer;
	int block = 0;
	int done = 0;
	ssize_t data_size;
	int no_data = 0;
	char r_buffer[517];


	op_pointer = (unsigned short*)buffer;
	char filename[256];
	strcpy(filename, buffer + 2);
	fp = open(filename, 'r');

	if (fp == NULL)
	{
		*op_pointer = htons(5);
		*(op_pointer + 1) = htons(1);
		*(op_pointer + 2) = htons(0);
		sendto(child_sock, buffer, 5, 0, (struct sockaddr*) child_server, sizeof(*sock_info));
		return;
	}

	/**op_pointer = htons(4);
	*(op_pointer + 1) = htons(block);
	block++;
	sendto(child_sock, buffer, 4, 0, (struct sockaddr*) child_server, sizeof(*sock_info));*/

	while (!done)
	{
		data_size = recvfrom(sock, buffer, 517, 0, &client, &addr_len);
		if (data_size < 0)
		{
			no_data++;
			if (no_data == 10)
		}
		*op_pointer = htons(3);
		*(op_pointer + 1) = htons(block);
		block++;
		char* buffer_ptr = buffer + 4;
		int i = 0;
		while (1)
		{
			if (feof(fp)) {
				done = 1;
				break;
			}
			if (i == 512) {break;}
			*buffer_ptr = fgetc(fp);
			buffer_ptr++;
			i++;
		}
		*(buffer_ptr) = '\0';

		sendto(child_sock, buffer, i + 5, 0, (struct sockaddr*) child_server, sizeof(*sock_info));

		no_data = 0;
		while (1)
		{
			data_size = recvfrom(sock, r_buffer, 517, 0, &client, &addr_len);
			if (!data_size < 0) {break;}
			no_data++;
			sendto(child_sock, buffer, i + 5, 0, (struct sockaddr*) child_server, sizeof(*sock_info));
			if (no_data == 10) {
				done = 1;
				break;
			}
		}
	}
}

int main (int argc, char** argv){
    int sock;
    socklen_t addr_len;
    int tid_selection;
    struct sockaddr_in server;
    struct timeval timeout;
    char* buffer = calloc(517, sizeof(char));
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
        read_data(buffer, child_sock, child_server);
        
    }
    if(op_code == 2){
        //WRQ
    }
    

}
