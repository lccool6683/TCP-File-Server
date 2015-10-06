#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // for close

#define DEFAULT_PORT 7005 
#define LENGTH 512 

void getFile(int sockfd);
void sendFile(int sockfd);
void error(const char *msg);


int main(int argc, char **argv)
{
	/* Variable Definition */
	int port, sockfd;
	char  *host, *choice;
	struct sockaddr_in remote_addr;
	
	/*store command arguments*/
	switch(argc)
    {
        case 3:
            choice = argv[1];
            host =	argv[2];
            port =	DEFAULT_PORT;
            break;
        case 4:
            choice = argv[1];
            host =	argv[2];
            port =	atoi(argv[3]);	// User specified port
            break;
        default:
			fprintf(stderr, "Usage: %s  [GET/SEND] [host] [port]\n", argv[0]);
            exit(1);
    }

	/* Get the Socket file descriptor */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
		exit(1);
	}

	/* Fill the socket address struct */
	remote_addr.sin_family = AF_INET; 
	remote_addr.sin_port = htons(port); 
	inet_pton(AF_INET, host, &remote_addr.sin_addr); 
	bzero(&(remote_addr.sin_zero), 8);
	
	if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1){
			fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
			exit(1);
		}else 
			printf("[Client] Connected to server at port %d...ok!\n", port);

	/*Receive file from client*/
	if(strcmp(choice, "SEND") == 0){
		sendFile(sockfd);
	}
	/*request file from server*/
	else if(strcmp(choice, "GET") == 0){
		getFile(sockfd);
	}
	return 0;
}

void sendFile(int sockfd){
	char revbuf[LENGTH], fileName[LENGTH];
	
	/*sent option to server*/
	strcpy(revbuf, "SEND");
	send(sockfd, revbuf, strlen(revbuf), 0);
	
	/*get file name to send*/
	printf("[Client] Enter the file name to send.\n");
	fgets(fileName, LENGTH, stdin);
	strtok(fileName, "\n");
	
	/*send file to server*/
	printf("[Client] Sending %s to the Server... \n", fileName);
	
	/*check if the file can be opened*/
	FILE *fp = fopen(fileName, "r");
	if(fp == NULL)
	{
		strcpy(fileName, "no file");
		send(sockfd, fileName, strlen(fileName), 0);
		printf("ERROR: File %s not found.\n", fileName);
		printf("[Client] Connection lost.\n");
	}
	
	/*send file name to server*/
	send(sockfd, fileName, strlen(fileName), 0);

	bzero(revbuf, LENGTH); //clean the buffer
	int fs_block_sz; //bytes to send
	while((fs_block_sz = fread(revbuf, sizeof(char), LENGTH, fp)) > 0)
	{
		if(send(sockfd, revbuf, fs_block_sz, 0) < 0)
		{
			fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fileName, errno);
			break;
		}
		bzero(revbuf, LENGTH);
	}
		printf("[Client] Ok File %s from Client was Sent!\n", fileName);
	close (sockfd);
	printf("[Client] Connection lost.\n");
}

void getFile(int sockfd){
	char revbuf[LENGTH], fileName[LENGTH];
	
	/*sent option to server*/
	strcpy(revbuf, "GET");
	send(sockfd, revbuf, strlen(revbuf), 0);
	
	/*get file name for request*/
	printf("[Client] Enter the file name to send.\n");
	fgets(fileName, LENGTH, stdin);
	strtok(fileName, "\n");
	
	/*send file name to server*/
	send(sockfd, fileName, strlen(fileName), 0);
	
	/* Receive File from Server */
	printf("[Client] Receiveing file from Server and saving it as %s\n", fileName);
	FILE *fp = fopen(fileName, "w");
	if(fp == NULL)
		printf("File %s Cannot be opened.\n", fileName);
	else{
		bzero(revbuf, LENGTH); 
		int fr_block_sz = 0;
		while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0)
		{
			int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fp);
				if(write_sz < fr_block_sz){
					error("[Client] File write failed.\n");
				}
			bzero(revbuf, LENGTH);
			if (fr_block_sz == 0 || fr_block_sz != 512) {
				break;
			}
		}
		if(fr_block_sz == 0){
			printf("[Client] File is not found in server\n");
			printf("[Client] Connection lost.\n");
			remove(fileName);
			exit(1);
		}
		if(fr_block_sz < 0){
			if (errno == EAGAIN)
				printf("recv() timed out.\n");
			else
				fprintf(stderr, "recv() failed due to errno = %d\n", errno);
		}
		printf("[Client] Ok received from server!\n");
		fclose(fp);
	}		
	close (sockfd);
	printf("[Client] Connection lost.\n");
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

