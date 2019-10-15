//
//  hw2_client.c
//  
//
//  Created by borute on 10/15/19.
//

#include "hw2_client.h"
#include "lib/unp.h"
#include <stdio.h>
int main(int argc, char* argv[]){
    int sockfd;
    struct sockaddr_in servaddr;
    //int addrlen;
    int port = atoi(argv[2]);
    
    int username_len = 0;
    char* username;
    username = calloc(1025, sizeof(char));
    username[0] = '\0';
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("socket create failed");
        return 0;
    }
    
    bzero(&servaddr, sizeof(servaddr)); 
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    
    if(connect(sockfd, (struct sockaddr*)& servaddr, sizeof(servaddr)) < 0){
        printf("connect failed");
        return 0;
    }
    
    char* server_response;
    server_response = calloc(1025, sizeof(char));
    
    while(1){
        read(sockfd, server_response, sizeof(server_response));
        printf("%s\n", server_response);
        char* temp;
        temp = calloc(2049, sizeof(char));
        strcpy(temp, "Username ");
        strcat(temp, username);
        strcat(temp, " is already taken, please enter a different username");
        if(strcmp(server_response, "Welcome to Guess the Word, please enter your username.") == 0 || strcmp(server_response, temp) == 0){
            fgets(server_response, sizeof(server_response), stdin);
            int index = 0;
            while(index < strlen(server_response)){
                //parse all uppercase letters
                if(server_response[index] >= 'A' && server_response[index] <= 'Z'){
                    server_response[index] = server_response[index] + ('a' - 'A');
                }
                index++;
            }
            server_response[strlen(server_response)] = '\0';
            username = server_response;
            username_len = strlen(username);
            write(sockfd, server_response, strlen(server_response) + 1);
        }
        else if(strcmp(server_response), "GGWP"){
            free(temp);
            break;
        }
        else{
            fgets(server_response, sizeof(server_response), stdin);
            int index = 0;
            while(index < strlen(server_response)){
                //parse all uppercase letters
                if(server_response[index] >= 'A' && server_response[index] <= 'Z'){
                    server_response[index] = server_response[index] + ('a' - 'A');
                }
                index++;
            }
            server_response[strlen(server_response)] = '\0';
            write(sockfd, server_response, strlen(server_response) + 1);
        }
    }
    free(username);
    close(sockfd);
    return 0;
}
