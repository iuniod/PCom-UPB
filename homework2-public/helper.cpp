#include "helper.h"

void send_message(char* message, int socketfd) {
	// send the message using a while loop and send()
	int bytes_send = 0;
	int bytes_total = strlen(message);

	while (bytes_send < bytes_total) {
		int bytes = send(socketfd, message + bytes_send, bytes_total - bytes_send, 0);
		DIE(bytes < 0, "send");
		bytes_send += bytes;
	}

}