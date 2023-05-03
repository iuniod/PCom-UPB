#ifndef _HELPER_H
#define _HELPER_H 1

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
#include <iostream>
#include <memory>


#define MAXLINE 1024
#define INT 0
#define SHORT_REAL 1
#define FLOAT 2
#define STRING 3

/*
 * Macro de verificare a erorilor
 * Exemplu:
 * 		int fd = open (file_name , O_RDONLY);
 * 		DIE( fd == -1, "open failed");
 */

#define DIE(assertion, call_description)                                       \
	do {                                                                         \
		if (assertion) {                                                           \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
			perror(call_description);                                                \
			exit(EXIT_FAILURE);                                                      \
		}                                                                          \
	} while (0)

/**
 * @brief Send a message to a socket
 * 
 * @param message the message to be sent
 * @param socketfd the socket to send the message to
 * @param size the size of the message
 */
void send_message(char* message, int socketfd, int size);

/**
 * @brief Receive a message from a socket
 * 
 * @param message the message to be received
 * @param socketfd the socket to receive the message from
 * @return int the size of the message
 */
int receive_message(char* message, int socketfd);

int receive_udp_message(char* message, struct sockaddr_in &addr_udp, socklen_t addr_udp_len, int socketfd);

struct subscriber {
	char id[10];
	int port;
	char ip[16];
	int socketfd;

	subscriber(char* id, int port, char* ip, int socketfd) {
		strcpy(this->id, id);
		this->port = port;
		strcpy(this->ip, ip);
		this->socketfd = socketfd;
	}

	subscriber() {
		strcpy(this->id, "");
		this->port = 0;
		strcpy(this->ip, "");
		this->socketfd = -1;
	}

	bool operator==(const subscriber& other) const {
		return (strcmp(this->id, other.id) == 0);
	}

	bool operator<(const subscriber& other) const {
		return (this->id < other.id);
	}

	bool operator>(const subscriber& other) const {
		return (this->id > other.id);
	}

	subscriber& operator=(const subscriber& other) {
		strcpy(this->id, other.id);
		this->port = other.port;
		strcpy(this->ip, other.ip);
		this->socketfd = other.socketfd;
		return *this;
	}

	/* Send the subscriber id to the server */
	void send_register(int socketfd) {
		send_message(this->id, socketfd, sizeof(this->id));
	}

	/* Subscribe to a topic */
	void subscribe(std::string topic, int sf, int socketfd) {
		char message[100];
		strcpy(message, "subscribe ");
		strcat(message, this->id);
		strcat(message, " ");
		strcat(message, std::to_string(sf).c_str());
		strcat(message, " ");
		strcat(message, topic.c_str());
		
		send_message(message, socketfd, sizeof(message));
		std::cout << "Subscribed to topic." << std::endl;
	}

	/* Unsubscribe from a topic */
	void unsubscribe(std::string topic, int socketfd) {
		char message[100];
		strcpy(message, "unsubscribe ");
		strcat(message, this->id);
		strcat(message, " ");
		strcat(message, topic.c_str());
		send_message(message, socketfd, sizeof(message));
		std::cout << "Unsubscribed from topic." << std::endl;
	}
};

/* Hash function for unordered_map */
struct hash {
	size_t operator()(const subscriber& sub) const {
		return std::hash<std::string>()(sub.id);
	}
};

/* Equal function for unordered_map */
struct equal_to {
	bool operator()(const subscriber& sub1, const subscriber& sub2) const {
		return (sub1.id == sub2.id);
	}
};

struct topic {
	char name[50];
	int sf;

	topic(char* name, int sf) {
		strcpy(this->name, name);
		this->sf = sf;
	}

	topic() {
		strcpy(this->name, "");
		this->sf = 0;
	}

	bool operator==(const topic& other) const {
		return (strcmp(this->name, other.name) == 0);
	}

	topic& operator=(const topic& other) {
		strcpy(this->name, other.name);
		this->sf = other.sf;
		return *this;
	}

	void send_register(int socketfd) {
		// send the struct to the server
		send_message((char*)this, socketfd, sizeof(topic));
	}
};

struct notification {
	char topic[50];
	uint8_t type;
	char content[1500];
};

struct client_subscription {
	bool is_connected;
	std::vector<topic> topics;
	std::queue<notification> notifications;
};

#endif  // _HELPER_H
