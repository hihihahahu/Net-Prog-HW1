#include "lib/unp.h"
#include <stdio.h>

int count = 0;

void handler(int sig)
{
	count--;
}

void dg_client(int sockfd, SA *pservaddr, socklen_t servlen, int port)
{
	int				n;
	char			line[MAXLINE + 1] = "114514";

	printf("%ld\n",sendto(sockfd, line, 6, 0, pservaddr, servlen));
	while (1) {
		n = recvfrom(sockfd, line, MAXLINE, 0, NULL, NULL);
		if (line[0] < 0)
		{
			printf(">Server on %d closed\n", port);
			return;
		}

		sendto(sockfd, line, n, 0, pservaddr, servlen);

		line[n] = 0;	/* null terminate */
		printf(">%d %s", port, line);
	}
}


int main(int argc, char **argv)
{
	Signal(SIGCHLD, handler);
	int port;

	int count = 0;

	while (1)
	{
		fscanf(stdin, "%d", &port);
		printf("114514\n");
		if (count >= 5) {continue;}
		count++;
		if (fork() == 0) {break;}
	}

	int					sockfd;
	struct sockaddr_in	servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);


	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	dg_client(sockfd, (SA *) &servaddr, sizeof(servaddr), port);

	exit(0);
}