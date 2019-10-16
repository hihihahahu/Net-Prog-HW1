#include "lib/unp.h"
#include <stdio.h>

int count;

void handler(int sig)
{
	count--;
}

void str_client(FILE *fp, int sockfd, int port)
{
	ssize_t		n;
	ssize_t 	a;
	char		buf[MAXLINE];
	fd_set fdset;
	struct timeval timeout = {0, 3};

	while (1)
	{
        char temp[2];
        temp[0] = 'a';
        temp[1] = '\0';
        write(sockfd, temp, 2);
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 3;
		if ((a = (select(sockfd + 1, &fdset, NULL, NULL, &timeout))) < 0)
		{
			exit(1);
		}
		if (FD_ISSET(sockfd, &fdset))
		{

			n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, NULL);
			//if(n > 0){
				if (n > 1)
				{
					if (buf[0] == '\0')
					{
                        if(buf[1] == '\0'){
                            printf("n = %ld\n", n);
                            printf("server %d closed\n", port);
                            return;
                        }
                        else{
                            continue;
                        }
						
					}
				}
                buf[n] = 0;    /* null terminate */
                if(n > 1 && buf[0] == '\n'){
                    printf(">%d %s", port, buf+1);

                }
                else{
                    printf(">%d %s", port, buf);

                }
                //fflush(stdout);
			//}
		}
	}
}

int main(int argc, char **argv)
{
	signal(SIGCHLD, handler);
	int port;

	count = 0;

	while (1)
	{
		fscanf(stdin, "%d", &port);
		if (count >= 5) {continue;}
		count++;
		if (fork() == 0) {break;}
	}

	int					sockfd;
	struct sockaddr_in	servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	str_client(stdin, sockfd, port);		/* do it all */

	close(sockfd);
	exit(0);
}
