#include "helper.h"
#include <math.h>

void parse_notification(notification notif) {
	int type = (int) notif.type;
	long long sign, value, power;
	uint8_t exponent;

	switch (type) {
		case INT:
			std::cout << notif.topic << " - INT - ";
			// first bit is sign
			sign = (*(char*)notif.content) ? -1 : 1;
			value = ntohl(*(uint32_t*) (notif.content + 1));
			std::cout << (sign == -1 ? "-" : "") << value << std::endl;
			break;
		case SHORT_REAL:
			std::cout << notif.topic << " - SHORT_REAL - " << ntohs(*(uint16_t*) notif.content) /100 << ".";
			value = ntohs(*(uint16_t*) notif.content) % 100;
			if (value < 10) {
				std::cout << "0";
			}
			std::cout << value << std::endl;
			break;
		case FLOAT:
			std::cout << notif.topic << " - FLOAT - ";
			// first bit is sign
			sign = (*(char*)notif.content) ? -1 : 1;
			value = ntohl(*(uint32_t*) (notif.content + 1));
			exponent = *(uint8_t*) (notif.content + 5);
			power = pow(10, exponent);
			std::cout << (sign == -1 ? "-" : "") << value / power << ".";
			value = value % power;
			while (value < power / 10) {
				std::cout << "0";
				power /= 10;
			}
			std::cout << value << std::endl;
			break;
		case STRING:
			std::cout << notif.topic << " - STRING - " << notif.content << std::endl;
			break;
		default:
			break;
	}
}

int main(int argc, char *argv[])
{
	int socketfd;
	struct sockaddr_in server_addr;

	if (argc != 4) {
		printf("Usage: %s <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	
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

	/* Add STDIN to epoll */
	struct epoll_event event;
	event.data.fd = STDIN_FILENO;
	event.events = EPOLLIN;
	DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &event) < 0, "epoll_ctl");

	/* Add socket to epoll */
	event.data.fd = socketfd;
	event.events = EPOLLIN;
	DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event) < 0, "epoll_ctl");

	while (true) {
		for ( ; ; ) {
			int n = epoll_wait(epollfd, &event, 1, -1);
			DIE(n < 0, "epoll_wait");

			/* If STDIN is ready to read */
			if (event.data.fd == STDIN_FILENO) {
				std::string command;
				std::cin >> command;

				if (command == "exit") {
					/* Send exit message to server */
					char buffer[MAXLINE];
					sprintf(buffer, "exit %s", argv[1]);
					send_message(buffer, socketfd, strlen(buffer) + 1);
					close(socketfd);
					return 0;
				}
				// [TODO] subscribe and unsubscribe
				if (command == "subscribe") {
					std::string topic;
					int sf;
					std::cin >> topic >> sf;
					client.subscribe(topic, sf, socketfd);
					continue;
				}
				if (command == "unsubscribe") {
					std::string topic;
					std::cin >> topic;
					client.unsubscribe(topic, socketfd);
					continue;
				}
				printf("[TCP CLIENT] Invalid command\n");
				continue;
			}			
			/* If socket is ready to read */
			if (event.data.fd == socketfd) {
				/* Receive message from server */
				char buffer[MAXLINE];
				int n = receive_message(buffer, socketfd);
				if (n == 0) {
					close(socketfd);
					return 0;
				}
				
				/* If the server is exiting */
				if (strcmp(buffer, "exit") == 0) {
					close(socketfd);
					return 0;
				}

				/* If the server is sending a notification */
				if (strcmp(buffer, "notification") == 0) {
					notification notif;

					/* Receive the notification */
					receive_message((char*) &notif, socketfd);
					/* Print the notification */
					parse_notification(notif);
					continue;
				}
			}
		}
	}	
	return 0;
}
