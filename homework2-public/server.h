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
	std::unordered_map<subscriber, std::pair<bool, std::vector<topic>>, hash, equal_to> subscribers;

	// constructor
	Server(int port) {
		this->port = port;
		init_server(port);
	}

	// destructor
	~Server() {
		// send exit message to all clients
		for (auto& sub : subscribers) {
			send(sub.first.socketfd, "exit", 4, 0);
		}
		close(udp_sockfd);
		close(tcp_sockfd);
	}

	// initialize the server
	void init_server(int port);

	// accept a connection
	int accept_connection();

	int handler();
};


#endif  // SERVER_H