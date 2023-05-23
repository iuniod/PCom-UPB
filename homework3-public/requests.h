#ifndef _REQUESTS_
#define _REQUESTS_

#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <string>

#define EMPTY ""

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(const char *host, const char *url, char *query_params,
							char **cookies, int cookies_count, const char *auth_jwt);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(const char *host, const char *url, const char* content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count, const char *auth_jwt);

// computes and returns a DELETE request string (cookies can be NULL if not needed)
char *compute_delete_request(const char *host, const char *url, char *query_params,
							char **cookies, int cookies_count, const char *auth_jwt);

/**
 * @brief Extracts the cookie or the jwt from the response
 * 
 * @param response the response from the server
 * @param type 0 for cookie, 1 for jwt
 * @return std::string the cookie or the jwt
 */
std::string extract(char* response, int type);

#endif
