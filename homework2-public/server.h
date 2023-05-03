#ifndef SERVER_H
#define SERVER_H

#include "helper.h"


struct Server {
	struct sockaddr_in udp_socket;
	int udp_sockfd;
	int tcp_sockfd;
	int port;
	int listenfd;
	int epollfd;
	struct epoll_event events[MAX_CONNECTIONS];
	std::unordered_map<subscriber, client_subscription, hash, equal_to> subscribers;

	Server(int port) {
		this->port = port;
		init_server(port);
	}

	~Server() {
		/* Send exit message to all subscribers that are connected */
		for (auto& sub : subscribers) {
			if (sub.second.get_connection_status() == true) {
				send_message(EXIT, sub.first.socketfd, 4);
				close(sub.first.socketfd);
			}
		}

		/* Close all sockets */
		close(udp_sockfd);
		close(tcp_sockfd);
	}

	/* Initialize server */
	void init_server(int port);

	/* Accept new connection */
	int accept_connection();

	/* Handle message from TCP & UDP sockets */
	int handler();
};


#endif  // SERVER_H