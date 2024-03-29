#include "helper.h"

void send_message(int socketfd, char *message, int size) {
  int bytes_send = 0;

  /* Send the size of the message */
  while (bytes_send < sizeof(int)) {
    int bytes = send(socketfd, &size + bytes_send, sizeof(int) - bytes_send, 0);
    DIE(bytes < 0, "send");
    bytes_send += bytes;
  }

  /* Send the message */
  bytes_send = 0;
  while (bytes_send < size) {
    int bytes = send(socketfd, message + bytes_send, size - bytes_send, 0);
    DIE(bytes < 0, "send");
    bytes_send += bytes;
  }
}

int receive_message(int socketfd, char *message) {
  int bytes_received = 0;
  int size;

  /* Receive the size of the message */
  while (bytes_received < sizeof(int)) {
    int bytes =
        recv(socketfd, &size + bytes_received, sizeof(int) - bytes_received, 0);
    if (!bytes) {
      return 0;
    }
    bytes_received += bytes;
  }

  /* Receive the message */
  bytes_received = 0;
  while (bytes_received < size) {
    int bytes =
        recv(socketfd, message + bytes_received, size - bytes_received, 0);
    if (!bytes) {
      return 0;
    }
    DIE(bytes < 0, "recv");
    bytes_received += bytes;
  }

  return bytes_received;
}

int receive_udp_message(int socketfd, char *message,
                        struct sockaddr_in &addr_udp, socklen_t addr_udp_len) {
  int bytes = recvfrom(socketfd, message, addr_udp_len, 0,
                       (struct sockaddr *)&addr_udp, &addr_udp_len);
  if (!bytes) {
    return 0;
  }
  DIE(bytes < 0, "recv");

  return bytes;
}

void add_connection_to_epoll(int fd, int epollfd) {
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = fd;
  DIE(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1,
      "epoll_ctl: listen_sock failed");
}
