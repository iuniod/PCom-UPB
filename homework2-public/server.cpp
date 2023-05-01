#include "server.h"
#include "helper.h"

void init_udp_connection(Server *server) {
	char buffer[MAXLINE];
	char *hello = "Hello from server";

	/* Creating socket file descriptor */
	if ( (server->udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&server->udp_socket, 0, sizeof(server->udp_socket));

	/* Make ports reusable, in case we run this really fast two times in a row */
	int enable = 1;
	if (setsockopt(server->udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	/* Filling server information */
	server->udp_socket.sin_family = AF_INET; // IPv4
	server->udp_socket.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 
	server->udp_socket.sin_port = htons(server->port);

	/* Bind the socket with the server address */
	if (bind(server->udp_sockfd, (const struct sockaddr *)&server->udp_socket,
			sizeof(server->udp_socket)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
}

void init_tcp_connection(Server *server) {
	/* Creating socket file descriptor */
	if ( (server->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	/* Make ports reusable, in case we run this really fast two times in a row */
	int enable = 1;
	if (setsockopt(server->tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}


	/* Filling server information */
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(server->port);

	/* Bind the socket with the server address */
	if (bind(server->tcp_sockfd, (const struct sockaddr *)&server_addr,
			sizeof(server_addr)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	/* Listen for connections */
	if (listen(server->tcp_sockfd, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server->tcp_sockfd;
	if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->tcp_sockfd, &ev) == -1) {
		std::cout << server->epollfd << " " << server->tcp_sockfd << std::endl;
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}
}

void Server::init_server(int port) {
	epollfd = epoll_create1(0);
	if (epollfd < 0) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	// add stdin to epoll
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
		perror("epoll_ctl: stdin");
		exit(EXIT_FAILURE);
	}

	init_udp_connection(this);
	init_tcp_connection(this);
}

int Server::handler() {
	for ( ; ; ) {
		int nfds = epoll_wait(epollfd, events, MAX_CONNECTIONS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nfds; i++) {
			// read from stdin
			if (events[i].data.fd == STDIN_FILENO) {
				char buffer[MAXLINE];
				fgets(buffer, MAXLINE, stdin);
				if (strncmp(buffer, "exit", 4) == 0) {
					return -1;
				}

				continue;
			}
			if (events[i].data.fd == tcp_sockfd) {
				int len;
				struct sockaddr_in cli;
				int connection_socket = accept(tcp_sockfd, (struct sockaddr *) &cli, (socklen_t *) &len);
				
				struct epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = connection_socket;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connection_socket, &ev) == -1) {
					perror("epoll_ctl: listen_sock");
					exit(EXIT_FAILURE);
				}

				// receive clients struct
				subscriber client;
				recv(connection_socket, &client, sizeof(client), 0);
				
				// check if client is already connected
				bool already_connected = false;
				bool status = false;
				for (auto it : subscribers) {
					if (strcmp(it.first.id, client.id) == 0 && it.second.first == true) {
						status = it.second.first;
						already_connected = true;
						break;
					}
				}

				if (already_connected) {
					std::cout << "Client " << client.id << " already connected." << status << std::endl;
					// send exit message
					send_message("exit", connection_socket);
					close(connection_socket);
					continue;
				}

				std::cout << "New client " << client.id << " connected from" << inet_ntoa(cli.sin_addr) << ":" << ntohs(cli.sin_port) << "." << std::endl;
				// add client to subscribers
				subscribers[client] = std::make_pair(true, std::vector<topic>());

				continue;
			}

			if (events[i].events & EPOLLIN) {
				char buffer[MAXLINE];
				int n = recv(events[i].data.fd, buffer, MAXLINE, 0);
				if (n == 0) {
					close(events[i].data.fd);
					continue;
				}

				// check if client is exiting
				if (strncmp(buffer, "exit", 4) == 0) {
					char client_id[10];
					strncpy(client_id, buffer + 5, 10);

					// set client as disconnected
					bool found = false;
					for (auto &it : subscribers) {
						if (strcmp(it.first.id, client_id) == 0) {
							it.second.first = false;
							found = true;
							break;
						}
					}
					if (found == false) {
						std::cout << "Client " << client_id << " not found." << std::endl;
						continue;
					}
					std::cout << "Client " << client_id << " disconnected." << std::endl;
					continue;
				}
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

	Server server(atoi(argv[1]));

	std::vector<struct pollfd> poll_fds;
	int num_clients = 1;
	int rc;

	while (1) {
		int rc = server.handler();
		if (rc == -1) {
			break;
		}
	}

	server.~Server();
	
	return 0; 
} 
