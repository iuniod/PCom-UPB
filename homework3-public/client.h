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
#define APP_JSON "application/json"
#define REGISTER_URL "/api/v1/tema/auth/register"
#define LOGIN_URL "/api/v1/tema/auth/login"
#define LOGOUT_URL "/api/v1/tema/auth/logout"
#define ENTER_LIBRARY_URL "/api/v1/tema/library/access"
#define BOOKS_URL "/api/v1/tema/library/books"
#define BOOK_URL "/api/v1/tema/library/books/"

#define RESPONSE_OK "HTTP/1.1 200 OK"
#define USERNAME "username"
#define PASSWORD "password"
#define TITLE "title"
#define AUTHOR "author"
#define GENRE "genre"
#define PAGE_COUNT "page_count"
#define PUBLISHER "publisher"
#define ID "id"
#define STATUS "status"
#define DATA "data"
#define ERROR "error"
#define EQ "="
#define EMPTY ""

#define MSG_EMPTY "Empty message\n\n"
#define MSG_ALREADY_LOGGED_IN "You are already logged in\nIf you want to login \
with another account, please logout first\n\n"	
#define MSG_INVALID_COMMAND "Invalid command\n\n"
#define MSG_INVALID_USERNAME "Invalid username - username must contain only letters, \
digits and underscores\n"
#define MSG_INVALID_PASSWORD "Invalid password - password must not contain spaces\n"
#define MSG_INVALID_TITLE "Invalid title - title must be a non-empty string\n"
#define MSG_INVALID_AUTHOR "Invalid author - author must be a non-empty string\n"
#define MSG_INVALID_GENRE "Invalid genre - genre must be a non-empty string\n"
#define MSG_INVALID_PAGE_COUNT "Invalid page_count - page_count must be a \
positive integer\n"
#define MSG_INVALID_PUBLISHER "Invalid publisher - publisher must be a \
non-empty string\n"
#define MSG_INVALID_ID "Invalid id - id must be a positive integer\n"
#define MSG_INVALID_JSON "Invalid JSON - please check your input\n\n"
#define MSG_TRY_AGAIN "Please try again\n\n"

#define MSG_SUCCESS "The request was successful\n\n"
#define MSG_REGISTER_SUCCESS "You have successfully registered\n\n"
#define MSG_LOGIN_SUCCESS "You have successfully logged in\n\n"
#define MSG_LOGOUT_SUCCESS "You have successfully logged out\n\n"
#define MSG_ENTER_LIBRARY_SUCCESS "You have successfully entered the library\n\n"
#define MSG_GET_BOOKS_SUCCESS "You have successfully retrieved the books\n\n"
#define MSG_GET_BOOK_SUCCESS "You have successfully retrieved the book\n\n"
#define MSG_ADD_BOOK_SUCCESS "You have successfully added the book\n\n"
#define MSG_DELETE_BOOK_SUCCESS "You have successfully deleted the book\n\n"

#define PRINT_ALL_BOOK "{\nid: %d\ntitle: %s\nauthor: %s\ngenre: %s\npublisher: %s\npage_count: %d\n}\n"
#define PRINT_BOOK "{\ntitle: %s\nid: %d\n}"

#define ALPHA_NUM "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"

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