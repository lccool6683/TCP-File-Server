#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // for close

#define DEFAULT_PORT 7005 
#define BACKLOG 5
#define LENGTH 512 

void sendFile(int nsockfd);
void getFile(int nsockfd);
void error(const char *msg);

int main (int argc, char **argv)
{
	/* Defining Variables */
	int port, sockfd, nsockfd, sin_size;
	struct sockaddr_in addr_local; /* client addr */
	struct sockaddr_in addr_remote; /* server addr */
	
	/*store command arguments*/
	switch(argc)
    {
        case 1:
            port = DEFAULT_PORT;
            break;
        case 2:
            port = atoi(argv[1]);	// Get user specified port
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            exit(1);
    }
	
	/* Get the Socket file descriptor */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
		exit(1);
	}else 
		printf("[Server] Obtaining socket descriptor successfully.\n");

	/* Fill the client socket address struct */
	addr_local.sin_family = AF_INET; // Protocol Family
	addr_local.sin_port = htons(port); // Port number
	addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
	bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

	
	/* Bind a special Port */
	if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
		fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
		exit(1);
	}else 
		printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",port);

	/* Listen remote connect calling */
	if(listen(sockfd,BACKLOG) == -1){
		fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
		exit(1);
	}else
		printf ("[Server] Listening the port %d successfully.\n", port);


	while(1)
	{
		char dataBuf[LENGTH] = ""; // Receiver buffer
		sin_size = sizeof(struct sockaddr_in);

		/* Wait a connection, and obtain a new socket file despriptor for single connection */
		if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1){
		    fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
			exit(1);
		}else 
			printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));
		
		/*recive client option*/
		recv(nsockfd, dataBuf, LENGTH, 0);
		
		/*recive file from client*/
		if(strcmp(dataBuf, "SEND") == 0){
			getFile(nsockfd);
		/*send file to client*/
		}else if(strcmp(dataBuf, "GET") == 0){
			sendFile(nsockfd);
		}else{
			printf("[Server] Wrong input command\n");
			break;
		}
	}
	
	return 0;
}

void getFile(int nsockfd){
	char dataBuf[LENGTH]; // Receiver buffer
	
	printf("[Server] Client is sending file....\n");	
	/*get file name from client to store*/
	recv(nsockfd, dataBuf, LENGTH, 0);
	strtok(dataBuf, "\n");
	
	/*error handling for file is not exist in client side*/
	if(strcmp(dataBuf, "no file") == 0){
		printf("[Server] File is not found in client\n");
		printf("[Server] Connection with Client closed. Server will wait now...\n");
		close (nsockfd);
	}else{
		/*Receive File from Client */
		printf("[Server] Receiveing file from client and saving it as %s\n", dataBuf);
		FILE *fp = fopen(dataBuf, "w");
		if(fp == NULL)
			printf("File %s Cannot be opened file on server.\n", dataBuf);
		else{
			bzero(dataBuf, LENGTH); 
			int fr_block_sz = 0;
			while((fr_block_sz = recv(nsockfd, dataBuf, LENGTH, 0)) > 0) {
				int write_sz = fwrite(dataBuf, sizeof(char), fr_block_sz, fp);
				if(write_sz < fr_block_sz){
					error("File write failed on server.\n");
				}
				bzero(dataBuf, LENGTH);
				if (fr_block_sz == 0 || fr_block_sz != 512) {
					break;
				}
			}
			if(fr_block_sz < 0){
				if (errno == EAGAIN){
					printf("recv() timed out.\n");
				}else{
					fprintf(stderr, "recv() failed due to errno = %d\n", errno);
					exit(1);
				}
			}
			printf("[Server] Ok received from client!\n");
			fclose(fp); 
			printf("[Server] Connection with Client closed. Server will wait now...\n");
		}
		close (nsockfd);
	}
}

void sendFile(int nsockfd){
	char dataBuf[LENGTH];
	
	printf("[Server] Client is requesting file....\n");
	/*get file name from client to send*/
	recv(nsockfd, dataBuf, LENGTH, 0);
	strtok(dataBuf, "\n");
	
	/* Send File to Client */
	FILE *fp = fopen(dataBuf, "r");
	if(fp != NULL){
		printf("[Server] Sending %s to the Client...\n", dataBuf);
		bzero(dataBuf, LENGTH); 
		int fs_block_sz; 
		while((fs_block_sz = fread(dataBuf, sizeof(char), LENGTH, fp))>0){
			if(send(nsockfd, dataBuf, fs_block_sz, 0) < 0){
				fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", dataBuf, errno);
				exit(1);
			}
			bzero(dataBuf, LENGTH);
		}
		printf("[Server] Ok sent to client!\n");
		fclose(fp); 
		printf("[Server] Connection with Client closed. Server will wait now...\n");
	}else{
		fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", dataBuf, errno);
		printf("[Server] Connection with Client closed. Server will wait now...\n");
		close (nsockfd);
	}
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}