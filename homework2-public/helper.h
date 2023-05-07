#ifndef _HELPER_H
#define _HELPER_H

#include "defines.h"

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
void send_message(int socketfd, char* message, int size);

/**
 * @brief Receive a message from a socket
 * 
 * @param message the message to be received
 * @param socketfd the socket to receive the message from
 * @return int the size of the message
 */
int receive_message(int socketfd, char* message);

/**
 * @brief Receive a message from a UDP socket
 * 
 * @param message the message to be received
 * @param addr_udp the address of the UDP sender
 * @param addr_udp_len the length of the UDP sender address
 * @param socketfd the socket to receive the message from
 * @return int the size of the message
 */
int receive_udp_message(int socketfd, char* message, struct sockaddr_in &addr_udp, socklen_t addr_udp_len);

/**
 * @brief Add a connection to the epoll
 * 
 * @param fd the file descriptor of the connection
 * @param epollfd the epoll file descriptor
 */
void add_connection_to_epoll(int fd, int epollfd);

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
		strcpy(this->id, EMPTY_STRING);
		this->port = 0;
		strcpy(this->ip, EMPTY_STRING);
		this->socketfd = -1;
	}

	/* Send the subscriber id to the server */
	void send_register(int socketfd) {
		send_message(socketfd, this->id, sizeof(this->id));
	}

	/* Subscribe to a topic */
	void subscribe(std::string topic, int sf, int socketfd) {
		char message[100];
		strcpy(message, SUBSCRIBE);
		strcat(message, SPACE_STRING);
		strcat(message, this->id);
		strcat(message, SPACE_STRING);
		strcat(message, std::to_string(sf).c_str());
		strcat(message, SPACE_STRING);
		strcat(message, topic.c_str());
		
		send_message(socketfd, message, sizeof(message));
		std::cout << "Subscribed to topic." << std::endl;
	}

	/* Unsubscribe from a topic */
	void unsubscribe(std::string topic, int socketfd) {
		char message[100];
		strcpy(message, UNSUBSCRIBE);
		strcat(message, SPACE_STRING);
		strcat(message, this->id);
		strcat(message, SPACE_STRING);
		strcat(message, topic.c_str());
		send_message(socketfd, message, sizeof(message));
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

	client_subscription() {
		this->is_connected = true;
		notifications = std::queue<notification>();
		topics = std::vector<topic>();
	}

	bool get_connection_status() {
		return this->is_connected;
	}

	void set_connection_status(bool status) {
		this->is_connected = status;
	}

	std::vector<topic> get_topics() {
		return this->topics;
	}

	void add_notification(notification notif) {
		this->notifications.push(notif);
	}

	void add_topic(topic topic) {
		this->topics.push_back(topic);
	}

	void remove_topic(char *topic_name) {
		int n = this->topics.size();
		for (auto it = this->topics.begin(); it != this->topics.end(); it++) {
			if (strcmp(it->name, topic_name) == 0) {
				this->topics.erase(it);
				break;
			}
		}
	}

	/* Send all unread notifications to the client */
	void send_notifications(int socketfd) {
		while (!this->notifications.empty()) {
			notification notif = this->notifications.front();
			send_message(socketfd, (char*)&notif, sizeof(notification));
			this->notifications.pop();
		}
	}

	std::queue<notification> get_notifications() {
		return this->notifications;
	}
};

#endif  // _HELPER_H
