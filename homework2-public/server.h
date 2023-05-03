#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "helper.h"

#define MAX_CONNECTIONS 32

// struct with the server information
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
				send_message("exit", sub.first.socketfd, 4);
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