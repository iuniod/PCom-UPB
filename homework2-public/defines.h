#ifndef DEFINES_H_
#define DEFINES_H_

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <errno.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


#define MAX_CONNECTIONS 1024
#define MAXLINE 1024

#define INT 0
#define SHORT_REAL 1
#define FLOAT 2
#define STRING 3

#define EMPTY_STRING ""
#define SPACE_STRING " "
#define SUBSCRIBE "subscribe"
#define UNSUBSCRIBE "unsubscribe"
#define EXIT "exit"

#define INVALID_COMMAND "Invalid command.\n"


#endif  // DEFINES_H_