#include "client.h"

/**
 * @brief Get the json object from the user input according to the command
 * 		  type
 * 
 * @param command type of command
 * @return json the json object
 */
json get_json(string_code command) {
	json j;

	switch (command) {
		case REGISTER: {
			string username, password;
			cout << USERNAME EQ;
			getline(cin, username);
			cout << PASSWORD EQ;
			getline(cin, password);
			j[USERNAME] = username;
			j[PASSWORD] = password;

			break;
		}
		case LOGIN: {
			string username, password;
			cout << USERNAME EQ;
			getline(cin, username);
			cout << PASSWORD EQ;
			getline(cin, password);
			j[USERNAME] = username;
			j[PASSWORD] = password;

			break;
		}
		case GET_BOOK: {
			string id;
			cout << ID EQ;
			getline(cin, id);
			if (isdigit(id.c_str()) == false) {
				j[ID] = "ERROR";
			} else {
				j[ID] = stoi(id);
			}
			break;
		}
		case ADD_BOOK: {
			string title, author, genre, publisher, page_count;
			cout << TITLE EQ;
			getline(cin, title);
			cout << AUTHOR EQ;
			getline(cin, author);
			cout << GENRE EQ;
			getline(cin, genre);
			cout << PUBLISHER EQ;
			getline(cin, publisher);
			cout << PAGE_COUNT EQ;
			getline(cin, page_count);
			j[TITLE] = title;
			j[AUTHOR] = author;
			j[GENRE] = genre;
			j[PUBLISHER] = publisher;
			if (isdigit(atoi(page_count.c_str())) == false) {
				j[PAGE_COUNT] = "ERROR";
			} else {
				j[PAGE_COUNT] = stoi(page_count);
			}
			break;
		}
		case DELETE_BOOK: {
			string id;
			cout << ID EQ;
			getline(cin, id);
			if (isdigit(atoi(id.c_str())) == false) {
				j[ID] = "ERROR";
			} else {
				j[ID] = stoi(id);
			}
			break;
		}
		default: {
			break;
		}
	}
	return j;
}

bool verify_json(json j, string_code command) {
	switch (command) {
		case REGISTER: {
			/* Verify if the json object contains in each field a string */
			if (j[USERNAME].is_string() == false || j[PASSWORD].is_string() == false) {
				cout << MSG_INVALID_COMMAND;
				return false;
			}
			break;
		}
		case LOGIN: {
			/* Verify if the json object contains in each field a string */
			if (j[USERNAME].is_string() == false || j[PASSWORD].is_string() == false) {
				cout << MSG_INVALID_COMMAND;
				return false;
			}
			break;
		}
		case GET_BOOK: {
			/* Verify if the json object contains in each field an integer */
			if (j[ID].is_number_integer() == false) {
				cout << MSG_INVALID_COMMAND;
				return false;
			}
			break;
		}
		case ADD_BOOK: {
			/* Verify if the json object contains in each field a string or an integer */
			if (j[TITLE].is_string() == false || j[AUTHOR].is_string() == false ||
				j[GENRE].is_string() == false || j[PUBLISHER].is_string() == false ||
				j[PAGE_COUNT].is_number_integer() == false) {
				cout << MSG_INVALID_COMMAND;
				return false;
			}
			break;
		}
		case DELETE_BOOK: {
			/* Verify if the json object contains in each field an integer */
			if (j[ID].is_number_integer() == false) {
				cout << MSG_INVALID_COMMAND;
				return false;
			}
			break;
		}
		default: {
			break;
		}
	}
	return true;
}

/**
 * @brief Get the body data object from the user input according to the
 * 		  command type - it can be the whole json or just the id
 * 
 * @param command type of command
 * @param is_valid true if the fields are valid, false otherwise
 * @return char* the body data
 */
char* get_body_data(string_code command) {
	json j = get_json(command);
	if (verify_json(j, command) == false) {
		return NULL;
	}

	/* Return the whole json if the command is login, register or add_book */
	if (command == LOGIN || command == REGISTER || command == ADD_BOOK) {
		char *payload =  (char*) calloc(j.dump().length() + 1, sizeof(char));
		strcpy(payload, j.dump().c_str());
		
		char *body = (char*) calloc(strlen(payload) + 1, sizeof(char));
		strcpy(body, payload);
		
		return body;
	}

	/* Otherwise, return only the id */
	char *payload = (char*) calloc(j[ID].dump().length() + 1, sizeof(char));
	strcpy(payload, j[ID].dump().c_str());

	return payload;
}

/**
 * @brief Extract the cookie from the response
 * 
 * @param response the response from the server
 * @return string the cookie
 */
string extract_cookie(char *response) {
	char *cookie = strstr(response, "Set-Cookie: ");
	if (cookie == NULL) {
		return EMPTY;
	}
	cookie += strlen("Set-Cookie: ");
	
	char *end = strstr(cookie, ";");
	if (end == NULL) {
		return EMPTY;
	}

	char *auth_cookie = (char*) calloc(end - cookie + 1, sizeof(char));
	strncpy(auth_cookie, cookie, end - cookie);

	return string(auth_cookie);
}

/**
 * @brief Extract the library token from the response
 * 
 * @param response the response from the server
 * @return string the library token
 */
string extract_library_token(char *response) {
	char *token = strstr(response, "token");
	if (token == NULL) {
		return EMPTY;
	}
	token += strlen("token") + 3;

	char *end = strstr(token, "\"");
	if (end == NULL) {
		return EMPTY;
	}

	char *auth_token = (char*) calloc(end - token + 1, sizeof(char));
	strncpy(auth_token, token, end - token);

	return string(auth_token);
}

int main() {
	int socket_fd;
	string auth_cookie = EMPTY;
	string auth_library_token = EMPTY;
	
	while (true) {
		/* Read the command */
		string command;
		getline(cin, command);

		/* Handle the command */
		string_code code = hashit(command);
		switch (code) {
			case LOGIN: {
				/* Check if the user is already logged in */
				if (auth_cookie != EMPTY) {
					cout << MSG_ALREADY_LOGGED_IN;
					break;
				}

				/* Get the body data */
				char *payload = get_body_data(code);
				if (payload == NULL) {
					break;
				}

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
				char *message = compute_post_request(IP_ADDRESS, LOGIN_URL, APP_JSON, &payload, 1, NULL, 0, NULL);
				send_to_server(socket_fd, message);

				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Extract the cookie from the response */
				if (strstr(response, RESPONSE_OK) != NULL) {
					auth_cookie = extract_cookie(response);
				}
				
				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case REGISTER: {
				/* Get the body data */
				char *payload = get_body_data(code);
				if (payload == NULL) {
					break;
				}
				
				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
 				char *message = compute_post_request(IP_ADDRESS, REGISTER_URL, APP_JSON, &payload, 1, NULL, 0, NULL);
				send_to_server(socket_fd, message);

				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case ENTER_LIBRARY: {
				/* Get the authentification cookie */
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
				char *message = compute_get_request(IP_ADDRESS, ENTER_LIBRARY_URL, NULL, &auth_cookie_str, 1, NULL);
				send_to_server(socket_fd, message);

				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Extract the library token from the response */
				if (strstr(response, RESPONSE_OK) != NULL) {
					auth_library_token = extract_library_token(response);
				}

				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case GET_BOOKS: {
				/* Get the authentification cookie and library token */
				char *auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
				char *message = compute_get_request(IP_ADDRESS, BOOKS_URL, NULL, &auth_cookie_str, 1, auth_library_token_str);
				send_to_server(socket_fd, message);
				
				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case GET_BOOK: {
				/* Get the book id and book url */
				char* trash = get_body_data(code);
				// if (trash == NULL) {
				// 	break;
				// }
				int id = atoi(trash);
				string book_url = BOOK_URL + to_string(id);

				/* Get the authentification cookie and library token */
				char* auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());
				

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
				char *message = compute_get_request(IP_ADDRESS, book_url.c_str(), NULL, &auth_cookie_str, 1, auth_library_token_str);
				send_to_server(socket_fd, message);
				
				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case ADD_BOOK: {
				/* Get the body data */
				char *payload = get_body_data(code);
				if (payload == NULL) {
					break;
				}

				/* Get the authentification cookie and library token */
				char *auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				char *message = compute_post_request(IP_ADDRESS, BOOKS_URL, APP_JSON, &payload, 1, &auth_cookie_str, 1, auth_library_token_str);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case DELETE_BOOK: {
				/* Get the book id and book url */
				char* trash = get_body_data(code);
				if (trash == NULL) {
					break;
				}
				int id = atoi(trash);
				string book_url = BOOK_URL + to_string(id);

				/* Get the authentification cookie and library token */
				char *auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
				char *message = compute_delete_request(IP_ADDRESS, book_url.c_str(), NULL, &auth_cookie_str, 1, auth_library_token_str);
				send_to_server(socket_fd, message);

				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Close connection to server */
				close_connection(socket_fd);
				break;
			}
			case LOGOUT: {
				/* Get the authentification cookie */
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());

				/* Open connection to server */
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				/* Send the request */
				char *message = compute_get_request(IP_ADDRESS, LOGOUT_URL, NULL, &auth_cookie_str, 1, NULL);
				send_to_server(socket_fd, message);

				/* Receive the response */
				char *response = receive_from_server(socket_fd);
				puts(response);

				/* Check if the response is ok and clear the authentification cookie and library token */
				if (strstr(response, RESPONSE_OK) != NULL) {
					auth_cookie = EMPTY;
					auth_library_token = EMPTY;
				}

				/* Close connection to server */
				close_connection(socket_fd);

				break;
			}
			case EXIT: {
				/*Exit the program*/
				return 0;
				break;
			}

			default: {
				/* Invalid command */
				cout << MSG_INVALID_COMMAND;
				break;
			}
		}
	}
}