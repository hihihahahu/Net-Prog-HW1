#include "lib/unp.h"
#include <stdio.h>

void dg_server(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int n;
	int len = clilen;
	char		mesg[MAXLINE];

	n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
	while (fgets(mesg, MAXLINE, stdin) != NULL)
	{
		sendto(sockfd, mesg, strlen(mesg), 0, pcliaddr, len);

		n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
	}
	mesg[0] = -1;
	sendto(sockfd, mesg, 1, 0, pcliaddr, len);
}

int main(int argc, char **argv)
{
	int					sockfd;
	int 				port = atoi(argv[1]) + 9877;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	if (bind(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
	{
		fprintf(stderr, ">bind fail\n");
		exit(1);
	}

	printf(">Accepted connection\n");

	dg_server(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	printf(">Shutting down due to EOF\n");
	close(sockfd);
	return 0;
}