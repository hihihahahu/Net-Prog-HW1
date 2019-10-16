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
#include <ctype.h>

struct match_number
{ 
   int correct;
   int place_correct;
};

struct match_number word_match(char* secret, char* guess)
{
    struct match_number ans;
    char a;
    int g_length = strlen(guess);
    int s_length = strlen(secret);
    int used[s_length];
    for (int i = 0; i < s_length; i++)
    {
        secret[i] = tolower(secret[i]);
        used[i] = 0;
    }
    if (g_length != s_length + 1)
    {
        ans.correct = -1;
        ans.place_correct = -1;
        return ans;
    }
    ans.correct = 0;
    ans.place_correct = 0;
    if (guess[g_length - 1] == '\n')
    {
        guess[g_length - 1] = '\0';
        g_length--;
    }
    for (int i = 0; i < s_length; i++)
    {
        if (secret[i] == guess[i]) {ans.place_correct++;}
    }
    for (int i = 0; i < g_length; i++)
    {
        for (int j = 0; j < s_length; j++)
        {
            if (guess[i] == secret[j] && used[j] == 0)
            {
                used[j] = 1;
                ans.correct++;
                break;
            }
        }
    }
    return ans;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(*(char**)a) - *(*(char**)b));
}

int dic_init(char* file_name, char***dic)
{
    int count = 0;
    FILE* fp = fopen(file_name, "r");
    char str[1025];
    if (fp == NULL)
    {
        fprintf(stderr, "could not open file\n");
        exit(1);
    }
    while (fgets(str, 1025, fp) != NULL)
    {
        count++;
    }
    rewind(fp);
    *dic = (char**)calloc(count, sizeof(char*));
    for (int i = 0; i < count; i++)
    {
        (*dic)[i] = (char*)calloc(1025, sizeof(char));
        fgets((*dic)[i], 1025, fp);
    }
    fclose(fp);
    qsort(*dic, count, sizeof(char*), cmpfunc);
    return count;
}

char* get_secret(char** dic, int dic_size)
{
    char* secret = (char*)calloc(1025, sizeof(char));
    char* tmp = dic[rand() % dic_size];
    int l = strlen(secret);
    int pos = 0;
    while (1)
    {
        if (tmp[pos] == '\0' || tmp[pos] == '\n' || tmp[pos] - '0' < 0)
        {
            secret[pos] = '\0';
            break;
        }
        secret[pos] = tmp[pos];
        pos++;
    }
    printf("%d\n", strlen(secret));
    return secret;
}

int main(int argc, char* argv[]){
    fd_set fds;
    
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
    //FD_SET(listenfd, &fds);
    
    
    while(1){
        //printf("Listening for new connections.\n");
        FD_SET(listenfd, &fds);
        int max_playerfd = -1;
        for(int a = 0; a < 5; a++){
            if(!slot_available[a]){
                if(player_fds[a] > max_playerfd){
                    max_playerfd = player_fds[a];
                }
                if(player_fds[a] > -1){
                    FD_SET(player_fds[a], &fds);
                }
            }
        }
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 5;
        int maxfd = max(max_playerfd, listenfd) + 1;
        select(maxfd, &fds, NULL, NULL, &tv);
        //printf("???\n");
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
            buffer[strlen(buffer) - 1] = '\0';
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
                        char str[3];
                        sprintf(str, "%d", player_count);
                        str[strlen(str)] = '\0';
                        strcpy(message, "Let's start playing, ");
                        strcat(message, usernames[a]);
                        strcat(message, "\nThere are ");
                        strcat(message, str);
                        strcat(message, " player(s) playing.\n");
                        message[strlen(message)] = '\0';
                        write(connfd, message, strlen(message) + 1);
                        //strcpy(prompt, "There are ");
                        
                        //write(player_fds[a], prompt, strlen(prompt) + 1);
                        break;
                    }
                }
            }
            else if(name_used){
                for(int a = 0; a < 5; a++){
                    if(slot_available[a]){
                        slot_available[a] = false;
                        player_fds[a] = connfd;
                        break;
                    }
                }
            }
        }
        //printf("checking player connection\n");
        for(int a = 0; a < 5; a++){
            //printf("checking player %d\n", a);
            if(!slot_available[a]){
                if(FD_ISSET(player_fds[a], &fds)){
                    //char recvline[MAXLINE];
                    int bytes_read = read(player_fds[a], buffer, sizeof(buffer));
                    printf("%d\n", bytes_read);
                    if (bytes_read == 0){
                        printf("Player %s disconnected.\n", usernames[a]);
                        FD_CLR(player_fds[a], &fds);
                        player_fds[a] = -1;
                        slot_available[a] = true;
                        has_username[a] = false;
                        player_count--;
                        continue;
                    }
                    
                    if(has_username[a] && bytes_read > 0){
                        
                        /*
                         
                         handle guesses here
                         
                         */
                        
                        
                        
                        printf("User %s (%d) has guessed: %s\n", usernames[a], a, buffer);
                        char message[1024];
                        strcpy(message, "this is some message\n");
                        //message[strlen(message)] = '\0';
                        write(player_fds[a], message, strlen(message) + 1);
                    }
                    else{
                        read(player_fds[a], buffer, sizeof(buffer));
                        buffer[strlen(buffer)] = '\0';
                        bool name_used = false;
                        for(int b = 0; b < 5; a++){
                            if(!slot_available[b]){
                                if(strcmp(buffer, usernames[b]) == 0){
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
                            char message[1025];
                            char str[3];
                            sprintf(str, "%d", player_count);
                            str[strlen(str)] = '\0';
                            strcpy(message, "Let's start playing, ");
                            strcat(message, usernames[a]);
                            strcat(message, "\nThere are ");
                            strcat(message, str);
                            strcat(message, " player(s) playing.\n");
                            message[strlen(message)] = '\0';
                            write(player_fds[a], message, strlen(message) + 1);
                        }
                    }
                }
            }
        }
    }
}
