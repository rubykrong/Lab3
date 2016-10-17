#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

#ifndef BUF_SIZE
#define BUF_SIZE 256
#endif

void error(const char *msg);

int main(int argc, char *argv[])
{
	int sockfd, portno, n, msgnumber, outfd, i;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[BUF_SIZE], filename[256];
	if (argc < 3) 
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	printf("Please enter the name of file you want to download: ");
	bzero(filename, 256);
	fgets(filename, 255, stdin);
	n = write(sockfd, filename, 255);
	if (n < 0) 
		error("ERROR writing to socket");
	bzero(buffer,256);
	n = read(sockfd, &msgnumber, sizeof(int));
	if (n < 0) 
		error("ERROR reading from socket");
	if (msgnumber == -1)
	{
		printf("File you want does not exist\n");
		return 0;
	}
	filename[strlen(filename) - 1] = '1';
	outfd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	printf("Downloading file...\n");
	for (i = 0; i < msgnumber; i++)
	{
		n = read(sockfd, buffer, BUF_SIZE - 1);
		if (n < 0)
			error("ERROR reading from socket");
		write(outfd, buffer, n);
	}
	printf("Downloaded successfully to %s.\n", filename);
	close(outfd);
	close(sockfd);
	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}
