//
//  hw2_server.c
//  
//
//  Created by borute on 10/15/19.
//

//#include "hw2_server.h"
#include "unp.h"
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
        //secret[i] = tolower(secret[i]);
        used[i] = 0;
    }
    if (g_length != s_length)
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
        if (tolower(secret[i]) == tolower(guess[i])) {ans.place_correct++;}
    }
    for (int i = 0; i < g_length; i++)
    {
        for (int j = 0; j < s_length; j++)
        {
            if (tolower(guess[i]) == tolower(secret[j]) && used[j] == 0)
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
    const char ** a1 = (const char**) a;
    const char ** b1 = (const char**) b;
    return strcmp(*a1, *b1);
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
    //qsort(*dic, count, sizeof(char*), cmpfunc);
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
    printf("%lu\n", strlen(secret));
    return secret;
}

int main(int argc, char* argv[]){
    setvbuf( stdout, NULL, _IONBF, 0 );
    srand(atoi(argv[1]));
    char** dic;
    int dic_size = dic_init(argv[3], &dic);
    char* secret = get_secret(dic, dic_size);
    
    printf("The secret word is: %s\n", secret);
    
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
    listen(listenfd, 10);
    //FD_SET(listenfd, &fds);
    
    
    while(1){
        //printf("Listening for new connections.\n");
        //FD_ZERO(&fds);
        FD_SET(listenfd, &fds);
        int max_playerfd = -1;
        for(int a = 0; a < 5; a++){
                if(player_fds[a] > -1){
                    FD_SET(player_fds[a], &fds);
                    if(player_fds[a] > max_playerfd){
                        max_playerfd = player_fds[a];
                    }
                }
        }
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 5;
        int maxfd = max(max_playerfd, listenfd) + 1;
        select(maxfd, &fds, NULL, NULL, NULL);
        //printf("???\n");
        if(FD_ISSET(listenfd, &fds)){
            printf("New client detected.\n");
            
            socklen_t len = sizeof(cliaddr);
            int connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
            //FD_CLR(listenfd, &fds);
            //FD_SET(connfd, &fds);
            if(player_count >= 5){
                //char* reject_message = "Too many players atm.";
                //read(connfd, buffer, sizeof(buffer));
                //write(connfd, reject_message, strlen(reject_message));
                close(connfd);
                continue;
            }
            player_count++;
            FD_SET(connfd, &fds);
            char welcome_message[1024];
            for(int a = 0; a < 1024; a++){
                welcome_message[a] = '\0';
            }
            sprintf(welcome_message, "Welcome to Guess the Word, please enter your username.");
            //welcome_message[strlen(welcome_message)] = '\0';
            printf("Welcome message sent.\n");
            send(connfd, welcome_message, strlen(welcome_message), 0);
            for(int a = 0; a < 1025; a++){
                buffer[a] = '\0';
            }
            //read username
            read(connfd, buffer, sizeof(buffer));
            FD_CLR(connfd, &fds);
            buffer[strlen(buffer) - 1] = '\0';
            printf("strlen of username is: %lu\n", strlen(buffer));
            bool name_used = false;
            //check if username is used
            for(int a = 0; a < 5; a++){
                if(!slot_available[a]){
                    if(strcmp(buffer, usernames[a]) == 0){
                        name_used = true;
                        char message[1025];
                        for(int c = 0; c < 1025; c++){
                            message[c] = '\0';
                        }
                        sprintf(message, "Username %s is already taken, please enter a different username.", usernames[a]);
                        //message[strlen(message)] = '\0';
                        send(connfd, message, strlen(message), 0);
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
                        //printf("strlen is: %lu\n", strlen(buffer));
                        usernames[a] = calloc(strlen(buffer) + 1, sizeof(char));
                        sprintf(usernames[a], "%s", buffer);
                        usernames[a][strlen(buffer)] = '\0';
                        has_username[a] = true;
                        char message[1025];
                        for(int c = 0; c < 1025; c++){
                            message[c] = '\0';
                        }
                        sprintf(message, "Let's start playing, %s", usernames[a]);
                        //printf("\n?\n");
                        /*
                        printf("?\n");
                        int fuckyou = 0;
                        while(message[fuckyou] != '\0'){
                            printf("%d\n", message[fuckyou] - '\0');
                            fuckyou++;
                        }
                        printf("strlen: %lu\n", strlen(message));
                        */
                        //message[strlen(message)] = '\0';
                        send(connfd, message, strlen(message), 0);
                        usleep(10);
                        char message2[1025];
                        for(int c = 0; c < 1025; c++){
                            message2[c] = '\0';
                        }
                        sprintf(message2, "There are %d player(s) playing. The secret word is %lu letter(s).", player_count, strlen(secret));
                        //printf("\n?\n");
                        //message2[strlen(message2)] = '\0';
                        send(connfd, message2, strlen(message2), 0);
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
                    for(int c = 0; c < 1025; c++){
                        buffer[c] = '\0';
                    }
                    int bytes_read = read(player_fds[a], buffer, sizeof(buffer));
                    FD_CLR(player_fds[a], &fds);
                    buffer[strlen(buffer) - 1] = '\0';
                    printf("%d\n", bytes_read);
                    printf("length of guess word: %lu\n", strlen(buffer));
                    if (bytes_read <= 0){
                        printf("Player %s disconnected.\n", usernames[a]);
                        close(player_fds[a]);
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
                        
                        
                        bool game_over = false;
                        printf("User %s (%d) has guessed: %s\n", usernames[a], a, buffer);
                        
                        struct match_number result = word_match(secret, buffer);
                        
                        
                        char message[2048];
                        for(int c = 0 ; c < 2048; c++){
                            message[c] = '\0';
                        }
                        if(result.correct == -1){
                            sprintf(message, "Invalid guess length. The secret word is %lu letter(s).", strlen(secret));
                            //message[strlen(message)] = '\0';
                            send(player_fds[a], message, strlen(message), 0);
                            continue;
                        }
                        else if(result.place_correct == strlen(secret)){
                            sprintf(message, "%s has correctly guessed the word %s", usernames[a], secret);
                            free(secret);
                            secret = get_secret(dic, dic_size);
                            printf("Game over, new word (%s) has been selected.\n", secret);
                            game_over = true;
                        }
                        else{
                            sprintf(message, "%s guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.", usernames[a], buffer, result.correct, result.place_correct);
                            //message[strlen(message)] = '\0';
                        }
                        for(int b = 0; b < 5; b++){
                            if(has_username[b]){
                                //message[strlen(message)] = '\0';
                                send(player_fds[b], message, strlen(message), 0);
                                if(game_over){
                                    close(player_fds[b]);
                                    player_fds[b] = -1;
                                    slot_available[b] = true;
                                    has_username[b] = false;
                                    player_count--;
                                }
                                //FD_CLR(player_fds[b], &fds);
                            }
                        }
                    }
                    else{
                        //read(player_fds[a], buffer, sizeof(buffer));
                        //buffer[strlen(buffer)] = '\0';
                        bool name_used = false;
                        for(int b = 0; b < 5; a++){
                            if(!slot_available[b]){
                                if(strcmp(buffer, usernames[b]) == 0){
                                    name_used = true;
                                    char message[1025];
                                    for(int c = 0; c < 1025; c++){
                                        message[c] = '\0';
                                    }
                                    strcpy(message, "Username ");
                                    strcat(message, buffer);
                                    strcat(message, " is already taken, please enter a different username.");
                                    //message[strlen(message)] = '\0';
                                    send(player_fds[a], message, strlen(message), 0);
                                    printf("Client requested a used username: %s\n", buffer);
                                    break;
                                }
                            }
                        }
                        if(!name_used){
                            printf("New user (%s) has joined.\n", buffer);
                            free(usernames[a]);
                            usernames[a] = calloc(strlen(buffer) + 1, sizeof(char));
                            sprintf(usernames[a], "%s", buffer);
                            usernames[a][strlen(buffer)] = '\0';
                            has_username[a] = true;
                            char message[1025];
                            for(int c = 0; c < 1025; c++){
                                message[c] = '\0';
                            }
                            sprintf(message, "Let's start playing, %s", usernames[a]);
                            /*
                            int fuckyou = 0;
                            printf("114514");
                            while(message[fuckyou] != '\0'){
                                printf("%d\n", message[fuckyou] - '\0');
                                fuckyou++;
                            }
                             */
                            //message[strlen(message)] = '\0';
                            send(player_fds[a], message, strlen(message), 0);
                            char message2[1025];
                            for(int c = 0; c < 1025; c++){
                                message2[c] = '\0';
                            }
                            sprintf(message2, "There are %d player(s) playing. The secret word is %lu letter(s).", player_count, strlen(secret));
                            //message2[strlen(message2)] = '\0';
                            send(player_fds[a], message2, strlen(message2), 0);
                        }
                    }
                }
            }
        }
    }
}
