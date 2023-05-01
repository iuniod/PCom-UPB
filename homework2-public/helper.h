#ifndef _HELPER_H
#define _HELPER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <string>
#include <cstring>
#include <sys/socket.h>

#define MAXLINE 1024

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

void send_message(char* message, int socketfd);

struct subscriber {
	char id[10];
	int port;
	char ip[16];
	int socketfd;
	bool online;

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

	void send_register(int socketfd) {
		// send the struct to the server
		send(socketfd, this, sizeof(subscriber), 0);
	}
};

// hash function for unordered_map
struct hash {
	size_t operator()(const subscriber& sub) const {
		return std::hash<std::string>()(sub.id);
	}
};

// equals function for unordered_map
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
		send(socketfd, this, sizeof(topic), 0);
	}
};

#endif  // _HELPER_H
