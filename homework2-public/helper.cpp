#include "helper.h"
#include <iostream>

void send_message(char* message, int socketfd, int size) {
	int bytes_send = 0;

	while (bytes_send < sizeof(int)) {
		int bytes = send(socketfd, &size + bytes_send, sizeof(int) - bytes_send, 0);
		DIE(bytes < 0, "send");
		bytes_send += bytes;
	}

	bytes_send = 0;
	while (bytes_send < size) {
		int bytes = send(socketfd, message + bytes_send, size - bytes_send, 0);
		DIE(bytes < 0, "send");
		bytes_send += bytes;
	}
}

int receive_message(char* message, int socketfd) {
	int bytes_received = 0;
	int size;

	while (bytes_received < sizeof(int)) {
		int bytes = recv(socketfd, &size + bytes_received, sizeof(int) - bytes_received, 0);
		if (!bytes) {
			return 0;
		}
		bytes_received += bytes;
	}

	bytes_received = 0;
	while (bytes_received < size) {
		int bytes = recv(socketfd, message + bytes_received, size - bytes_received, 0);
		if (!bytes) {
			return 0;
		}
		DIE(bytes < 0, "recv");
		bytes_received += bytes;
	}

	return bytes_received;
}