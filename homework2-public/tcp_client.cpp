#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/epoll.h> 

#include "helper.h"

int main(int argc, char *argv[])
{
	int socketfd;
	struct sockaddr_in server_addr;

	if (argc != 4) {
		printf("Usage: %s <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	/* Clean buffers and structures*/
	memset(&server_addr, 0, sizeof(server_addr));
	
	/* Create socket, we use SOCK_STREAM for TCP */
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("[CLIENT] Could not create socket\n");
		return -1;
	}
	
	/* Set port and IP the same as server-side */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[3]));
	server_addr.sin_addr.s_addr = inet_addr(argv[2]);
	
	/* Send connection request to server */
	if(connect(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		printf("[CLIENT] Unable to connect\n");
		return -1;
	}

	/* Send message to server */
	subscriber client(argv[1], atoi(argv[3]), argv[2], socketfd);
	client.send_register(socketfd);

	int epollfd = epoll_create1(0);
	DIE(epollfd < 0, "epoll_create1");

	// add stdin to epoll
	struct epoll_event event;
	event.data.fd = STDIN_FILENO;
	event.events = EPOLLIN;
	DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0, "epoll_ctl");

	// add socket to epoll
	event.data.fd = socketfd;
	event.events = EPOLLIN;
	DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event) < 0, "epoll_ctl");

	while (true) {
		for ( ; ; ) {
			int n = epoll_wait(epollfd, &event, 1, -1);
			DIE(n < 0, "epoll_wait");

			if (event.data.fd == STDIN_FILENO) {
				char buffer[MAXLINE];
				fgets(buffer, MAXLINE, stdin);
				buffer[strlen(buffer) - 1] = '\0';
				if (strcmp(buffer, "exit") == 0) {
					// send exit message to server
					char buffer[MAXLINE];
					sprintf(buffer, "exit %s", argv[1]);
					send(socketfd, buffer, strlen(buffer), 0);
					close(socketfd);
					return 0;
				}
				// [TODO] subscribe and unsubscribe
			} else if (event.data.fd == socketfd) {
				char buffer[MAXLINE];
				int n = read(socketfd, buffer, MAXLINE);
				if (n == 0) {
					close(socketfd);
					return 0;
				}
				
				if (strcmp(buffer, "exit") == 0) {
					close(socketfd);
					return 0;
				}
			}
		}
	}	
	return 0;
}
