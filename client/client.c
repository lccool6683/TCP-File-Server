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
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define PORT 20000
#define LENGTH 512 


void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	/* Variable Definition */
	int sockfd; 
	char revbuf[LENGTH]; 
	char choice[10];
	char fileName[1024];
	struct sockaddr_in remote_addr;

	/* Get the Socket file descriptor */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
		exit(1);
	}

	/* Fill the socket address struct */
	remote_addr.sin_family = AF_INET; 
	remote_addr.sin_port = htons(PORT); 
	inet_pton(AF_INET, "127.0.0.1", &remote_addr.sin_addr); 
	bzero(&(remote_addr.sin_zero), 8);
	
	
	printf("SEND or GET?\n");
	fgets(choice, 10, stdin);

	while(1){
		if(strcmp(choice, "SEND\n") == 0){
			if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1){
				fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
				exit(1);
			}else 
				printf("[Client] Connected to server at port %d...ok!\n", PORT);
			
			/*sent option to server*/
			char *fn = "SEND";
			strcpy(revbuf, fn);
			send(sockfd, revbuf, strlen(revbuf), 0);
			
			/*get file name to send*/
			printf("Enter the file name to send.\n");
			fgets(fileName, 1024, stdin);
			strtok(fileName, "\n");
			//strcpy(revbuf, fileName);
				  
			
			char* fs_name = "./";
			char * str3 = (char *) malloc(1 + strlen(fs_name)+ strlen(fileName) );
			strcpy(str3, fs_name);
			strcat(str3, fileName);
			//char sdbuf[LENGTH]; 
			
			/*send file to server*/
			printf("[Client] Sending %s to the Server... ", str3);
			FILE *fs = fopen(str3, "r");
			if(fs == NULL)
			{
				printf("ERROR: File %s not found.\n", str3);
				exit(1);
			}
			
			send(sockfd, fileName, strlen(fileName), 0);

			bzero(revbuf, LENGTH); 
			int fs_block_sz; 
			while((fs_block_sz = fread(revbuf, sizeof(char), LENGTH, fs)) > 0)
			{
				if(send(sockfd, revbuf, fs_block_sz, 0) < 0)
				{
					fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", str3, errno);
					break;
				}
				bzero(revbuf, LENGTH);
			}
				printf("Ok File %s from Client was Sent!\n", str3);
			
			close (sockfd);
			printf("[Client] Connection lost.\n");
			return (0);
		}
		
		else if(strcmp(choice, "GET\n") == 0){
			if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
			{
				fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
				exit(1);
			}
			else 
				printf("[Client] Connected to server at port %d...ok!\n", PORT);
			
			/*sent option to server*/
			char *fn = "GET";
			strcpy(revbuf, fn);
			send(sockfd, revbuf, strlen(revbuf), 0);
			
			printf("Enter the file name to send.\n");
				  fgets(fileName, 1024, stdin);
				  strtok(fileName, "\n");

				  
					strcpy(revbuf, fileName);
					send(sockfd, revbuf, strlen(revbuf), 0);
			
			/* Receive File from Server */
			
			char* fr_name = "./";
			char * str3 = (char *) malloc(1 + strlen(fr_name)+ strlen(fileName) );
			strcpy(str3, fr_name);
			strcat(str3, fileName);
			
			FILE *fr = fopen(fileName, "w");
			if(fr == NULL)
				printf("File %s Cannot be opened.\n", str3);
			else
			{
				bzero(revbuf, LENGTH); 
				int fr_block_sz = 0;
				while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0)
				{
					int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
						if(write_sz < fr_block_sz){
							error("File write failed.\n");
						}
					bzero(revbuf, LENGTH);
					if (fr_block_sz == 0 || fr_block_sz != 512) {
						break;
					}
				}
				if(fr_block_sz == 0){
					printf("no file\n");
					
					int status;
					status = remove(fileName);
					if( status == 0 )
						printf("%s file deleted successfully.\n",fileName);
					else
					{
						printf("Unable to delete the file\n");
						perror("Error");
					}
					return 0;
				}
				if(fr_block_sz < 0){
					if (errno == EAGAIN){
						printf("recv() timed out.\n");
					}
				else{
					fprintf(stderr, "recv() failed due to errno = %d\n", errno);
				}
				
				}
			printf("[Client] Receiveing file from Server and saving it as ");
			printf(str3);
			printf("\nOk received from server!\n");
			fclose(fr);
			}	
			close (sockfd);
			printf("[Client] Connection lost.\n");
			return (0);
		}
		else{
			printf("SEND or GET?\n");
			fgets(choice, 10, stdin);
		}
	}
	
	return 0;
}


	