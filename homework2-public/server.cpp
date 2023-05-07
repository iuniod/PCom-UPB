#include "server.h"

void init_udp_connection(Server *server) {
	/* Creating socket file descriptor */
	DIE((server->udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0, "socket creation failed");

	memset(&server->udp_socket, 0, sizeof(server->udp_socket));

	/* Make ports reusable, in case we run this really fast two times in a row */
	int enable = 1;
	DIE(setsockopt(server->udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0, "setsockopt(SO_REUSEADDR) failed");

	/* Filling server information */
	server->udp_socket.sin_family = AF_INET; // IPv4
	server->udp_socket.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 
	server->udp_socket.sin_port = htons(server->port);

	/* Bind the socket with the server address */
	DIE(bind(server->udp_sockfd, (const struct sockaddr *)&server->udp_socket, sizeof(server->udp_socket)) < 0, "bind failed");

	/* Add socket to epoll */
	add_connection_to_epoll(server->udp_sockfd, server->epollfd);
}

void init_tcp_connection(Server *server) {
	/* Creating socket file descriptor */
	DIE((server->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0, "socket creation failed");

	/* Make ports reusable, in case we run this really fast two times in a row */
	int enable = 1;
	DIE(setsockopt(server->tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0, "setsockopt(SO_REUSEADDR) failed");

	/* Disable Nagle's algorithm */
	int flag = 1;
	DIE(setsockopt(server->tcp_sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0, "setsockopt(TCP_NODELAY) failed");

	/* Filling server information */
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(server->port);

	/* Bind the socket with the server address */
	DIE(bind(server->tcp_sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0, "bind failed");

	/* Listen for connections */
	DIE(listen(server->tcp_sockfd, 5) < 0, "listen failed");

	/* Add socket to epoll */
	add_connection_to_epoll(server->tcp_sockfd, server->epollfd);
}

void Server::init_server(int port) {
	epollfd = epoll_create1(0);
	DIE(epollfd < 0, "epoll_create failed");

	/* Add STDIN_FILENO to epollfd */
	add_connection_to_epoll(STDIN_FILENO, epollfd);

	init_udp_connection(this);
	init_tcp_connection(this);
}

int Server::handler() {
	while (true) {
		int nfds = epoll_wait(epollfd, events, MAX_CONNECTIONS, -1);
		DIE(nfds == -1, "epoll_wait failed");

		for (int i = 0; i < nfds; i++) {
			/* Read from STDIN_FILENO - only exit command allowed */
			if (events[i].data.fd == STDIN_FILENO) {
				int rc = stdin_handler();
				if (rc == -1) {
					return rc;
				}
				continue;
			}

			/* Reading new connection from TCP socket */
			if (events[i].data.fd == tcp_sockfd) {
				tcp_handler();
				continue;
			}

			/* Reading new message from UDP socket */
			if (events[i].data.fd == udp_sockfd) {
				udp_handler();
				continue;
			}

			/* Reading new message from TCP socket */
			if (events[i].events & EPOLLIN) {
				tcp_message_handler(events[i]);
				continue;
			}

		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	Server server(atoi(argv[1]));

	std::vector<struct pollfd> poll_fds;
	int num_clients = 1;
	int rc;

	while (true) {
		int rc = server.handler();
		if (rc == -1) {
			break;
		}
	}

	server.~Server();
	
	return 0; 
} 
