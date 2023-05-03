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
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server->udp_sockfd;
	DIE(epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->udp_sockfd, &ev) < 0, "epoll_ctl");
}

void init_tcp_connection(Server *server) {
	/* Creating socket file descriptor */
	DIE((server->tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0, "socket creation failed");

	/* Make ports reusable, in case we run this really fast two times in a row */
	int enable = 1;
	DIE(setsockopt(server->tcp_sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0, "setsockopt(SO_REUSEPORT) failed");

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
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server->tcp_sockfd;
	DIE(epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->tcp_sockfd, &ev) < 0, "epoll_ctl");
}

void Server::init_server(int port) {
	epollfd = epoll_create1(0);
	DIE(epollfd < 0, "epoll_create failed");

	/* Add STDIN_FILENO to epollfd */
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1, "epoll_ctl: stdin failed");

	init_udp_connection(this);
	init_tcp_connection(this);
}

int Server::handler() {
	for ( ; ; ) {
		int nfds = epoll_wait(epollfd, events, MAX_CONNECTIONS, -1);
		DIE(nfds == -1, "epoll_wait failed");

		for (int i = 0; i < nfds; i++) {
			/* Read from STDIN_FILENO - only exit command allowed */
			if (events[i].data.fd == STDIN_FILENO) {
				std::string command;
				std::cin >> command;

				if (command == EXIT) {
					return -1;
				}

				std::cerr << "Invalid command." << std::endl;
				continue;
			}

			/* Reading new connection from TCP socket */
			if (events[i].data.fd == tcp_sockfd) {
				int len;
				struct sockaddr_in cli;

				/* Accept new connection */
				int connection_socket = accept(tcp_sockfd, (struct sockaddr *) &cli, (socklen_t *) &len);
				
				/* Add new connection to epollfd */
				struct epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = connection_socket;
				DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, connection_socket, &ev) == -1, "epoll_ctl: listen_sock failed");

				/* Receive client struct */
				subscriber client, tmp_client;
				receive_message((char*) &client, connection_socket);
				
				/* Check if client is already connected */
				bool already_connected = false;
				bool status = false;
				for (auto &it : subscribers) {
					if (strcmp(it.first.id, client.id) == 0) {
						status = true;
						if (it.second.get_connection_status() == true) {
							already_connected = true;
							break;
						} else {
							it.second.set_connection_status(true);
							it.second.send_notifications(it.first.socketfd);
						}
					}
				}

				if (already_connected) {
					std::cout << "Client " << client.id << " already connected." << std::endl;
					/* Send exit message to client and close connection */
					send_message(EXIT, connection_socket, 4);
					close(connection_socket);
					continue;
				}

				std::cout << "New client " << client.id << " connected from " << inet_ntoa(cli.sin_addr) << ":" << ntohs(cli.sin_port) << "." << std::endl;
				/* Add new client to subscribers */
				if (status == false) {
					client.socketfd = connection_socket;
					subscribers[client] = client_subscription();
				}
				continue;
			}

			/* Reading new message from UDP socket */
			if (events[i].data.fd == udp_sockfd) {
				/* Receive message */
				notification notif = notification();
				struct sockaddr_in addr_udp;

				int buflen = receive_udp_message((char*) &notif, addr_udp, sizeof(notif), udp_sockfd);

				/* Send message to all subscribers */
				for (auto &it : subscribers) {
					if (it.second.get_connection_status() == true) {
						for (auto &topic : it.second.get_topics()) {
							if (strcmp(topic.name, notif.topic) == 0) {
								send_message(NOTIFICATION, it.first.socketfd, 13);
								send_message((char*) &notif, it.first.socketfd, sizeof(notif));
								break;
							}
						}
					} else if (it.second.get_connection_status() == false) {
						/* Send message to offline clients */
						for (auto &topic : it.second.get_topics()) {
							if (strcmp(topic.name, notif.topic) == 0 && topic.sf == 1) {
								it.second.add_notification(notif);
								break;
							}
						}
					}
				}

				continue;
			}

			/* Reading new message from TCP socket */
			if (events[i].events & EPOLLIN) {
				char buffer[MAXLINE];

				/* Receive message */
				int n = receive_message(buffer, events[i].data.fd);
				if (n == 0) {
					close(events[i].data.fd);
					continue;
				}

				/* Check if message is exit */
				if (strncmp(buffer, EXIT, 4) == 0) {
					char client_id[10];
					strncpy(client_id, buffer + 5, 10);

					/* Set client status to false - disconnected */
					bool found = false;
					for (auto &it : subscribers) {
						if (strcmp(it.first.id, client_id) == 0) {
							it.second.set_connection_status(false);
							found = true;
							break;
						}
					}

					/* Check if client was found */
					if (found == false) {
						std::cerr << "Client " << client_id << " not found." << std::endl;
						continue;
					}
					std::cout << "Client " << client_id << " disconnected." << std::endl;
					continue;
				}

				/* Check if message is subscribe */
				if (strncmp(buffer, SUBSCRIBE, 9) == 0) {
					char client_id[10];
					char topic_name[50];
					char sf[2];

					/* Parse message */
					sscanf(buffer + 10, "%s %s %s", client_id, sf, topic_name);

					/* Add topic to client */
					bool found = false;
					for (auto &it : subscribers) {
						if (strcmp(it.first.id, client_id) == 0) {
							found = true;
							it.second.add_topic(topic(topic_name, atoi(sf)));
							break;
						}
					}

					/* Check if client was found */
					if (found == false) {
						std::cerr << "Client " << client_id << " not found." << std::endl;
						continue;
					}
					continue;
				}

				/* Check if message is unsubscribe */
				if (strncmp(buffer, UNSUBSCRIBE, 11) == 0) {
					char client_id[10];
					char topic_name[50];

					/* Parse message */
					sscanf(buffer + 12, "%s %s", client_id, topic_name);

					/* Remove topic from client */
					bool found = false;
					for (auto &it : subscribers) {
						if (strcmp(it.first.id, client_id) == 0) {
							found = true;
							it.second.remove_topic(topic_name);
							break;
						}
					}

					/* Check if client was found */
					if (found == false) {
						std::cerr << "Client " << client_id << " not found." << std::endl;
						continue;
					}
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
