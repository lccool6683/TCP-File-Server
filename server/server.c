#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h> 
#include <unistd.h> // for close

#define PORT 20000 
#define BACKLOG 5
#define LENGTH 512 

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main ()
{
	/* Defining Variables */
	int sockfd; 
	int nsockfd; 
	int sin_size; 
	struct sockaddr_in addr_local; /* client addr */
	struct sockaddr_in addr_remote; /* server addr */
	
	

	/* Get the Socket file descriptor */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
		exit(1);
	}
	else 
		printf("[Server] Obtaining socket descriptor successfully.\n");

	/* Fill the client socket address struct */
	addr_local.sin_family = AF_INET; // Protocol Family
	addr_local.sin_port = htons(PORT); // Port number
	addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
	bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

	/* Bind a special Port */
	
	if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
		fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
		exit(1);
	}else 
		printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);

	/* Listen remote connect/calling */
	if(listen(sockfd,BACKLOG) == -1){
		fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
		exit(1);
	}else
		printf ("[Server] Listening the port %d successfully.\n", PORT);

	while(1)
	{
		char dataBuf[LENGTH] = ""; // Receiver buffer
		sin_size = sizeof(struct sockaddr_in);

		/* Wait a connection, and obtain a new socket file despriptor for single connection */
		if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1) 
		{
		    fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
			exit(1);
		}
		else 
			printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));
		
		recv(nsockfd, dataBuf, LENGTH, 0);
		if(strcmp(dataBuf, "SEND") == 0){
			printf("going to send\n");
			recv(nsockfd, dataBuf, LENGTH, 0);
			strtok(dataBuf, "\n");
			char* fs_name = "./";
			char * str3 = (char *) malloc(1 + strlen(fs_name)+ strlen(dataBuf) );
			strcpy(str3, fs_name);
			strcat(str3, dataBuf);
			
			/*Receive File from Client */
			FILE *fr = fopen(str3, "w");
			if(fr == NULL)
				printf("File %s Cannot be opened file on server.\n", str3);
			else{
				bzero(dataBuf, LENGTH); 
				int fr_block_sz = 0;
				while((fr_block_sz = recv(nsockfd, dataBuf, LENGTH, 0)) > 0) {
					int write_sz = fwrite(dataBuf, sizeof(char), fr_block_sz, fr);
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
					} else{
						fprintf(stderr, "recv() failed due to errno = %d\n", errno);
						exit(1);
					}
				}
				printf("Ok received from client!\n");
				fclose(fr); 
			}
		}else if(strcmp(dataBuf, "GET") == 0){
			printf("going to GET\n");
			recv(nsockfd, dataBuf, LENGTH, 0);
			strtok(dataBuf, "\n");
			
			/* Send File to Client */
		    char* fs_name = "./";
			char * str3 = (char *) malloc(1 + strlen(fs_name)+ strlen(dataBuf) );
			strcpy(str3, fs_name);
			strcat(str3, dataBuf);
		    char sdbuf[LENGTH]; // Send buffer
		    
		    FILE *fs = fopen(dataBuf, "r");
		    if(fs != NULL){
				printf("[Server] Sending %s to the Client...", dataBuf);
				bzero(sdbuf, LENGTH); 
				int fs_block_sz; 
				while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0){
					if(send(nsockfd, sdbuf, fs_block_sz, 0) < 0){
						fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", dataBuf, errno);
						exit(1);
					}
					bzero(sdbuf, LENGTH);
				}
				printf("Ok sent to client!\n");
				fclose(fs); 
				printf("[Server] Connection with Client closed. Server will wait now...\n");
		    }else{
				fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", dataBuf, errno);
				close (nsockfd);
			}

		}else{
			printf("wrong input\n");
			break;
		}
	}
	
	return 0;
}