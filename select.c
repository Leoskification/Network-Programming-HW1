#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/select.h>

char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html;charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>"
"<html><head><title>Network Programming website</title></head>"
"<body><center><img src =\"image2.jpg\"></center></body></html>\r\n";

int main() {
	
	struct sockaddr_in client_addr, server_addr;
	socklen_t client_len;
	
	int fd_client, fd_server;
	int i, flag = 1, max, maxFd, socketFd;
	int availConnect, numClient[FD_SETSIZE];
	char buffer[2048] = "\0";

	ssize_t temp;
	fd_set currentSet, allSet;
	
	fd_server = socket(AF_INET, SOCK_STREAM, 0);

	if(fd_server < 0) {
		perror("socket error\n");
		exit(1);
	}

	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8081);

	if(bind(fd_server, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("bind error\n");
		close(fd_server);
		exit(1);
	}

	if(listen(fd_server, 10) == -1) {
		perror("listen error\n");
		close(fd_server);
		exit(1);
	}

	maxFd = fd_server;
	max = -1;
	FD_ZERO(&allSet);
	FD_SET(fd_server, &allSet);

	for(i = 0; i < FD_SETSIZE; i++) {
		numClient[i] = -1;
	}

	while(1) {
		currentSet = allSet;
		availConnect = select(maxFd + 1, &currentSet, NULL, NULL, NULL);
	
		if(FD_ISSET(fd_server, &currentSet) != 0) {
			client_len = sizeof(client_addr);
			fd_client = accept(fd_server, (struct sockaddr*)&client_addr, &client_len);
			
			for(i = 0; i < FD_SETSIZE; i++) {
				if(numClient[i] == -1) {
					numClient[i] = fd_client;
					break;
				}
			}
			
			if(i == FD_SETSIZE) {
				perror("server is full\n");
			}

			FD_SET(fd_client, &allSet);
			if(fd_client > maxFd) {
				maxFd = fd_client;
			}
			
			if(i > max) {
				max = i;
			}
			
			availConnect--;
			if(availConnect <= 0) {
				continue;
			}
		}

		for(i = 0; i <= max; i++) {
			if((socketFd = numClient[i]) < 0) {
				continue;
			}
			
			if(FD_ISSET(socketFd, &currentSet) != 0) {
				if((temp = read(socketFd, buffer, 2047)) == 0) {
					close(socketFd);
					FD_CLR(socketFd, &allSet);
					numClient[i] = -1;
				}
				if(!strncmp(buffer, "GET /image2.jpg", 15)) {
					int fp = open("image2.jpg", O_RDONLY);	
					int len;
					char tempBuffer[4096];
					sprintf(tempBuffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", "image/jpeg");
					write(socketFd, tempBuffer, strlen(tempBuffer));
					while((len = read(fp, tempBuffer, 8096)) > 0) {
						write(socketFd, tempBuffer, len);
					}
				}
				else {
					write(socketFd, webpage, sizeof(webpage) - 1);
				}
				close(socketFd);
				FD_CLR(socketFd, &allSet);
				numClient[i] = -1;
			}

			availConnect--;
			if(availConnect <= 0) {
				break;
			}
		}
	}
}
