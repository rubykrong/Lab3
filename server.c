#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>

#ifndef BUF_SIZE
#define BUF_SIZE 256
#endif

void* dostuff(void * socketfd);	
void error(const char *msg);

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
			&clilen);
		if (newsockfd < 0)
			error("ERROR on accept1");
#ifndef THREAD 
		pid = fork();
		if (pid < 0)
			error("ERROR on accept2");
		if (pid == 0)
		{
			close(sockfd);
			dostuff(&newsockfd);
			exit(0);
		}
		else
			close(newsockfd);
#else
		{
			pthread_t tid;
			pthread_create(&tid, NULL, &dostuff, &newsockfd);
		}
#endif
	}
	close(sockfd);
	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void* dostuff(void *voidsockfd)
{
	int sockfd = *((int *)(voidsockfd));
	int n, i, datafilefd, filesize, msgnumber, bytesread;
	char filename[BUF_SIZE], buffer[BUF_SIZE];
	struct stat st;

	bzero(filename, BUF_SIZE);
	n = read(sockfd, filename, BUF_SIZE - 1);
	void* dostuff(void*);
	if (n < 0) 
		error("ERROR reading from socket");
	filename[strlen(filename) - 1] = '\0';
	datafilefd = open(filename, O_RDONLY);
	if (datafilefd == -1)
	{
		n = write(sockfd, &datafilefd, sizeof(int));
		if (n < 0)
			error("ERROR writing to socket");
		return NULL;
	}
	fstat(datafilefd, &st);
	filesize = st.st_size;
	msgnumber = filesize / (BUF_SIZE - 1)
		 + (filesize % (BUF_SIZE - 1) > 0 ? 1 : 0);
	n = write(sockfd, &msgnumber, sizeof(int));
	if (n < 0)
		error("ERROR writing to socket");
	for (i = 0; i < msgnumber; i++)
	{
		bytesread = read(datafilefd, buffer, BUF_SIZE - 1);
		n = write(sockfd, buffer, bytesread);
		if (n < 0)
			error("ERROR writing to socket");
	}
	close(datafilefd);
	return NULL;
}
