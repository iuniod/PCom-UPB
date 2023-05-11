#include "helper.h"

void print_INT(char *topic, char *content) {
  long long sign = (*(char *)content) ? -1 : 1;
  long long value = ntohl(*(uint32_t *)(content + 1));

  std::cout << " - INT - " << (sign == -1 ? "-" : EMPTY_STRING) << value
            << std::endl;
}

void print_SHORT_REAL(char *topic, char *content) {
  std::cout << " - SHORT_REAL - " << ntohs(*(uint16_t *)content) / 100 << ".";

  long long value = ntohs(*(uint16_t *)content) % 100;

  if (value < 10) {
    std::cout << "0";
  }
  std::cout << value << std::endl;
}

void print_FLOAT(char *topic, char *content) {
  long long sign = (*(char *)content) ? -1 : 1;
  long long value = ntohl(*(uint32_t *)(content + 1));
  uint8_t exponent = *(uint8_t *)(content + 5);
  long long power = pow(10, exponent);

  std::cout << " - FLOAT - " << (sign == -1 ? "-" : EMPTY_STRING)
            << value / power << ".";

  value = value % power;
  while (value < power / 10) {
    std::cout << "0";
    power /= 10;
  }

  std::cout << value << std::endl;
}

void print_STRING(char *topic, char *content) {
  std::cout << " - STRING - " << content << std::endl;
}

void parse_notification(notification notif) {
  long long sign, value, power;
  uint8_t exponent;

  std::cout << notif.ip << ":" << notif.port << " - " << notif.topic;

  switch ((int)notif.type) {
  case INT:
    print_INT(notif.topic, notif.content);
    break;
  case SHORT_REAL:
    print_SHORT_REAL(notif.topic, notif.content);
    break;
  case FLOAT:
    print_FLOAT(notif.topic, notif.content);
    break;
  case STRING:
    print_STRING(notif.topic, notif.content);
    break;
  default:
    break;
  }
}

void handle_stdin_message(subscriber &client, int socketfd) {
  std::string command;
  std::cin >> command;

  if (command == EXIT) {
    /* Send exit message to server */
    char buffer[MAXLINE];
    sprintf(buffer, "exit %s", client.id);
    send_message(socketfd, buffer, strlen(buffer) + 1);
    close(socketfd);
    exit(EXIT_SUCCESS);
  }

  if (command == SUBSCRIBE) {
    std::string topic;
    int sf;
    std::cin >> topic >> sf;
    client.subscribe(topic, sf, socketfd);
    return;
  }

  if (command == UNSUBSCRIBE) {
    std::string topic;
    std::cin >> topic;
    client.unsubscribe(topic, socketfd);
    return;
  }

  std::cerr << INVALID_COMMAND;
}

void handle_server_message(int socketfd) {
  /* Receive message from server */
  notification notif;
  int n = receive_message(socketfd, (char *)&notif);

  /* If the server has closed the connection */
  if (n == 0) {
    close(socketfd);
    exit(EXIT_SUCCESS);
  }

  /* If the server has sent a notification */
  parse_notification(notif);
}

int main(int argc, char *argv[]) {
  int socketfd;
  struct sockaddr_in server_addr;

  if (argc != 4) {
    printf("Usage: %s <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  /* Clean buffers and structures*/
  memset(&server_addr, 0, sizeof(server_addr));

  /* Create socket, we use SOCK_STREAM for TCP */
  DIE((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0, "socket");

  /* Make ports reusable, in case we run this really fast two times in a row */
  int enable = 1;
  DIE(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0,
      "setsockopt(SO_REUSEADDR) failed");

  /* Disable Nagle's algorithm */
  int flag = 1;
  DIE(setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0,
      "setsockopt(TCP_NODELAY) failed");

  /* Set port and IP the same as server-side */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[3]));
  server_addr.sin_addr.s_addr = inet_addr(argv[2]);

  /* Send connection request to server */
  DIE(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
          0,
      "connect");

  /* Send message to server */
  subscriber client(argv[1], atoi(argv[3]), argv[2], socketfd);
  client.send_register(socketfd);

  int epollfd = epoll_create1(0);
  DIE(epollfd < 0, "epoll_create1");

  /* Add STDIN to epoll */
  add_connection_to_epoll(STDIN_FILENO, epollfd);

  /* Add socket to epoll */
  add_connection_to_epoll(socketfd, epollfd);

  while (true) {
    while (true) {
      struct epoll_event event;
      int n = epoll_wait(epollfd, &event, 1, -1);
      DIE(n < 0, "epoll_wait");

      /* If STDIN is ready to read */
      if (event.data.fd == STDIN_FILENO) {
        handle_stdin_message(client, socketfd);
        continue;
      }

      /* If socket is ready to read */
      if (event.data.fd == socketfd) {
        handle_server_message(socketfd);
        continue;
      }
    }
  }
  return 0;
}
