#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/sendfile.h>

char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html;charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Network Programming website</title></head>\r\n"
"<body><center><img src = \"image1.jpg\"></center></body></html>\r\n";

void sendFile(int fd_client, char *path, char *attr) {

	int ret, fd_img;
	char tempBuffer[4096];
	sprintf(tempBuffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", attr);

	if((fd_img = open(path, O_RDONLY) == -1)) {
		perror("fail to open file\n");
		exit(1);
	}

	write(fd_client, tempBuffer, strlen(tempBuffer));
	while((ret = read(fd_img, tempBuffer,4096)) > 0) {
		write(fd_client, tempBuffer, ret);
	}
	close(fd_img);
}


int main(int argc, char *argv[]) {

	struct sockaddr_in client_addr, server_addr;
	socklen_t sin_len = sizeof(client_addr);
	
	int fd_client, fd_server;
	int on = 1;
	char buffer[2048] = "\0";

	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_server < 0) {
		perror("socket error\n");
		exit(1);
	}

	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);

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

	while(1) {
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		
		if(fd_client == -1) {
			perror("Client connection failed\n");
			continue;
		}
		printf("Client connection success\n");
		
		if(!fork()) {

			close(fd_server);
			memset(buffer, 0 , 2048);
			read(fd_client, buffer, 2047);

			if(!strncmp(buffer, "GET /image1.jpg", 15)) {
				int fp=open("image1.jpg", O_RDONLY);
				int len;
				char tempBuffer[4096];

				sprintf(tempBuffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", "image/jpeg");
			
			
				write(fd_client, tempBuffer, strlen(tempBuffer));
				while((len = read(fp, tempBuffer, 4096)) > 0) {
					write(fd_client, tempBuffer, len);
				}
				close(fp);
			}
			else {
				write(fd_client, webpage, sizeof(webpage) - 1);
			}
			
			close(fd_client);
			exit(0);
		}
		
		close(fd_client);
	}

	return 0;
}

