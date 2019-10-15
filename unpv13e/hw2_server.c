//
//  hw2_server.c
//  
//
//  Created by borute on 10/15/19.
//

#include "hw2_server.h"
#include "lib/unp.h"
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]){
    fd_set fds;
    FD_ZERO(&fds);
    
    int listenfd;
    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    int player_count = 0;
    
    
    int player_fds[5];
    bool slot_available[5];
    bool has_username[5];
    char** usernames = calloc(5, sizeof(char*));
    char buffer[1025];
    
    for(int a = 0; a < 5; a++){
        player_fds[a] = -1;
        slot_available[a] = true;
        has_username[a] = false;
    }
    
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("socket create failed");
        return 0;
    }
    
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[2]));
    
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 50);
    FD_SET(listenfd, &fds);
    
    while(1){
        int max_playerfd = -1;
        for(int a = 0; a < 5; a++){
            if(!slot_available[a]){
                if(player_fds[a] > max_playerfd){
                    max_playerfd = player_fds[a];
                }
            }
        }
        int maxfd = max(max_playerfd, listenfd) + 1;
        select(maxfd, &fds, NULL, NULL, NULL);
        if(FD_ISSET(listenfd, &fds)){
            printf("New client detected.\n");
            //reads username
            
            socklen_t len = sizeof(cliaddr);
            int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
            if(player_count >= 5){
                char* reject_message = "Too many players atm.\n";
                read(connfd, buffer, sizeof(buffer));
                write(connfd, reject_message, strlen(reject_message) + 1);
                close(connfd);
                continue;
            }
            player_count++;
            FD_SET(connfd, &fds);
            char* welcome_message = "Welcome to Guess the Word, please enter your username.";
            printf("Welcome message sent.\n");
            write(connfd, welcome_message, strlen(welcome_message) + 1);
            read(connfd, buffer, sizeof(buffer));
            buffer[strlen(buffer)] = '\0';
            //check if username is used
            bool name_used = false;
            for(int a = 0; a < 5; a++){
                if(!slot_available[a]){
                    if(strcmp(buffer, usernames[a]) == 0){
                        name_used = true;
                        char message[1025];
                        strcpy(message, "Username ");
                        strcat(message, buffer);
                        strcat(message, " is already taken, please enter a different username");
                        message[strlen(message)] = '\0';
                        write(connfd, message, strlen(message) + 1);
                        printf("Client requested a used username: %s\n", buffer);
                        break;
                    }
                }
            }
            if(!name_used){
                for(int a = 0; a < 5; a++){
                    if(slot_available[a]){
                        printf("New user (%s) has joined.\n", buffer);
                        slot_available[a] = false;
                        player_fds[a] = connfd;
                        free(usernames[a]);
                        usernames[a] = calloc(strlen(buffer) + 1, sizeof(char));
                        for(int b = 0; b < strlen(buffer); b++){
                            usernames[a][b] = buffer[b];
                        }
                        usernames[a][strlen(buffer)] = '\0';
                        has_username[a] = true;
                        char message[1025];
                        strcpy(message, "Let's start playing, ");
                        strcat(message, usernames[a]);
                        strcat(message, "\n\0");
                        write(connfd, message, strlen(message) + 1);
                    }
                }
            }
            else if(name_used){
                for(int a = 0; a < 5; a++){
                    if(slot_available[a]){
                        slot_available[a] = false;
                        player_fds[a] = connfd;
                    }
                }
            }
        }
        for(int a = 0; a < 5; a++){
            if(!slot_available[a]){
                if(FD_ISSET(player_fds[a], &fds)){
                    char recvline[MAXLINE];
                    if (Readline(player_fds[a], recvline, MAXLINE) == 0){
                        printf("Player %s disconnected.\n", usernames[a]);
                        FD_CLR(player_fds[a], &fds);
                        player_fds[a] = -1;
                        slot_available[a] = true;
                        has_username[a] = false;
                        continue;
                    }
                    if(has_username[a]){
                        
                        /*
                         
                         handle guesses here
                         
                         */
                        
                        
                        
                        read(player_fds[a], buffer, sizeof(buffer));
                        printf("User %s has guessed: %s\n", usernames[a], buffer);
                        char message[1024];
                        strcpy(message, "this is some message\n");
                        message[strlen(message)] = '\0';
                        write(player_fds[a], message, strlen(message) + 1);
                    }
                    else{
                        read(player_fds[a], buffer, sizeof(buffer));
                        buffer[strlen(buffer)] = '\0';
                        bool name_used = false;
                        for(int a = 0; a < 5; a++){
                            if(!slot_available[a]){
                                if(strcmp(buffer, usernames[a]) == 0){
                                    name_used = true;
                                    char message[1025];
                                    strcpy(message, "Username ");
                                    strcat(message, buffer);
                                    strcat(message, " is already taken, please enter a different username");
                                    message[strlen(message)] = '\0';
                                    write(player_fds[a], message, strlen(message) + 1);
                                    printf("Client requested a used username: %s\n", buffer);
                                    break;
                                }
                            }
                        }
                        if(!name_used){
                            printf("New user (%s) has joined.\n", buffer);
                            free(usernames[a]);
                            usernames[a] = calloc(strlen(buffer) + 1, sizeof(char));
                            for(int b = 0; b < strlen(buffer); b++){
                                usernames[a][b] = buffer[b];
                            }
                            usernames[a][strlen(buffer)] = '\0';
                            has_username[a] = true;
                            char message[1024];
                            strcpy(message, "Let's start playing, ");
                            strcat(message, usernames[a]);
                            strcat(message, "\n\0");
                            write(player_fds[a], message, strlen(message) + 1);
                        }
                    }
                }
            }
        }
    }
}
