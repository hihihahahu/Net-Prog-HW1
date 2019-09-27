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

int count;

void handler(int sig)
{
	count--;
}

void str_client(FILE *fp, int sockfd, int port)
{
	ssize_t		n;
	char		buf[MAXLINE];
	fd_set fdset;
	struct timeval timeout = {0, 3};

	while (1)
	{
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		if (select(sockfd + 1, &fdset, NULL, NULL, &timeout) > 0)
		{
			exit(1);
		}
		if (FD_ISSET(sockfd, &fdset))
		{
			n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, NULL);
			buf[n] = 0;	/* null terminate */
			printf(">%d %s", port, buf);
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

	exit(0);
}