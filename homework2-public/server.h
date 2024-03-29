#ifndef SERVER_H
#define SERVER_H

#include "helper.h"
#include <cstdint>

/* Structure for the server */
struct Server {
  /* Internal variables */
  struct sockaddr_in udp_socket;
  int udp_sockfd;
  int tcp_sockfd;
  int port;
  int listenfd;
  int epollfd;
  struct epoll_event events[MAX_CONNECTIONS];
  std::unordered_map<subscriber, client_subscription, hash, equal_to>
      subscribers;

  /* Constructor */
  Server(int port) {
    this->port = port;
    init_server(port);
  }

  /* Destructor */
  ~Server() {
    /* Send exit message to all subscribers that are connected */
    for (auto &sub : subscribers) {
      if (sub.second.get_connection_status() == true) {
        close(sub.first.socketfd);
      }
    }

    /* Close all sockets */
    close(udp_sockfd);
    close(tcp_sockfd);
  }

  /* Initialize server */
  void init_server(int port);

  /* Handle message from TCP & UDP sockets */
  int handler();

  /* Handle message from STDIN_FILENO */
  int stdin_handler() {
    std::string command;
    std::cin >> command;

    if (command == EXIT) {
      return -1;
    }

    std::cerr << INVALID_COMMAND;
    return 0;
  }

  /* Handle new connection from TCP socket */
  void tcp_handler() {
    int len;
    struct sockaddr_in cli;

    /* Accept new connection */
    int connection_socket =
        accept(tcp_sockfd, (struct sockaddr *)&cli, (socklen_t *)&len);

    /* Add new connection to epollfd */
    add_connection_to_epoll(connection_socket, epollfd);

    /* Receive client struct */
    subscriber client, tmp_client;
    receive_message(connection_socket, (char *)&client);

    /* Check if client is already connected */
    bool already_connected = false;
    bool status = false;
    auto it = get_client(client.id);
    if (it != subscribers.end()) {
      status = true;
      if (it->second.get_connection_status() == true) {
        already_connected = true;
      } else {
        /* Client is already subscribed, but not connected - wake him up */
        it->second.set_connection_status(true);
        it->second.send_notifications(it->first.socketfd);
      }
    }

    if (already_connected) {
      std::cout << "Client " << client.id << " already connected." << std::endl;
      /* Send exit message to client and close connection */
      close(connection_socket);
      return;
    }

    std::cout << "New client " << client.id << " connected from "
              << inet_ntoa(cli.sin_addr) << ":" << ntohs(cli.sin_port) << "."
              << std::endl;
    /* Add new client to subscribers */
    if (status == false) {
      client.socketfd = connection_socket;
      subscribers[client] = client_subscription();
    }
  }

  /* Handle message from UDP socket */
  void udp_handler() {
    /* Receive message */
    notification notif = notification();
    struct sockaddr_in addr_udp;

    int buflen = receive_udp_message(udp_sockfd, (char *)&notif, addr_udp,
                                     sizeof(notif) - sizeof(uint32_t) -
                                         sizeof(uint16_t));

    notif.ip = addr_udp.sin_addr.s_addr;
    notif.port = addr_udp.sin_port;
    /* Send message to all subscribers */
    for (auto &it : subscribers) {
      if (it.second.get_connection_status() == true) {
        for (auto &topic : it.second.get_topics()) {
          if (strcmp(topic.name, notif.topic) == 0) {
            send_message(it.first.socketfd, (char *)&notif, sizeof(notif));
            break;
          }
        }
      } else {
        /* Send message to offline clients */
        for (auto &topic : it.second.get_topics()) {
          if (strcmp(topic.name, notif.topic) == 0 && topic.sf == 1) {
            it.second.add_notification(notif);
            break;
          }
        }
      }
    }
  }

  /* Handle message from TCP socket */
  void tcp_message_handler(epoll_event &event) {
    char buffer[MAXLINE];

    /* Receive message */
    int n = receive_message(event.data.fd, buffer);
    if (n == 0) {
      close(event.data.fd);
      return;
    }

    /* Check if message is exit */
    if (strncmp(buffer, EXIT, 4) == 0) {
      char client_id[10];
      strncpy(client_id, buffer + 5, 10);

      /* Set client status to false - disconnected */
      auto it = get_client(client_id);
      if (it == subscribers.end()) {
        std::cerr << "Client " << client_id << " not found." << std::endl;
        return;
      }

      it->second.set_connection_status(false);

      std::cout << "Client " << client_id << " disconnected." << std::endl;
      return;
    }

    /* Check if message is subscribe */
    if (strncmp(buffer, SUBSCRIBE, 9) == 0) {
      char client_id[10];
      char topic_name[50];
      char sf[2];

      /* Parse message */
      sscanf(buffer + 10, "%s %s %s", client_id, sf, topic_name);

      /* Add topic to client */
      auto it = get_client(client_id);
      if (it == subscribers.end()) {
        std::cerr << "Client " << client_id << " not found." << std::endl;
        return;
      }

      it->second.add_topic(topic(topic_name, atoi(sf)));

      return;
    }

    /* Check if message is unsubscribe */
    if (strncmp(buffer, UNSUBSCRIBE, 11) == 0) {
      char client_id[10];
      char topic_name[50];

      /* Parse message */
      sscanf(buffer + 12, "%s %s", client_id, topic_name);

      /* Remove topic from client */
      auto it = get_client(client_id);
      if (it == subscribers.end()) {
        std::cerr << "Client " << client_id << " not found." << std::endl;
        return;
      }

      it->second.remove_topic(topic_name);
      return;
    }
  }

  /* Get a specific client */
  std::unordered_map<subscriber, client_subscription, hash, equal_to>::iterator
  get_client(char *client_id) {
    for (auto it = subscribers.begin(); it != subscribers.end(); ++it) {
      if (strcmp(it->first.id, client_id) == 0) {
        return it;
      }
    }
    return subscribers.end();
  }
};

#endif // SERVER_H