#ifndef CLIENT_H
#define CLIENT_H



#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include <sys/socket.h>
#include "json.hpp"
#include "helpers.h"
#include "requests.h"

#define PORT 8080
#define IP_ADDRESS "34.254.242.81"
#define REGISTER_URL "/api/v1/tema/auth/register"
#define LOGIN_URL "/api/v1/tema/auth/login"
#define LOGOUT_URL "/api/v1/tema/auth/logout"
#define ENTER_LIBRARY_URL "/api/v1/tema/library/access"
#define BOOKS_URL "/api/v1/tema/library/books"
#define BOOK_URL "/api/v1/tema/library/books/"

#define RESPONSE_OK "HTTP/1.1 200 OK"
#define RESPONSE_ERROR "HTTP/1.1 400 Bad Request"

using namespace std;
using json = nlohmann::json;

enum string_code {
	REGISTER,
	LOGIN,
	ENTER_LIBRARY,
	GET_BOOKS,
	GET_BOOK,
	ADD_BOOK,
	DELETE_BOOK,
	LOGOUT,
	EXIT,
	UNDEFINED
};

const unordered_map <string, string_code> codes = {
	{"register", REGISTER},
	{"login", LOGIN},
	{"enter_library", ENTER_LIBRARY},
	{"get_books", GET_BOOKS},
	{"get_book", GET_BOOK},
	{"add_book", ADD_BOOK},
	{"delete_book", DELETE_BOOK},
	{"logout", LOGOUT},
	{"exit", EXIT}
};

string_code hashit (std::string const& command) {
	return codes.find(command) != codes.end() ? codes.at(command) : UNDEFINED;
}

#endif // CLIENT_H