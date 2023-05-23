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
			try {
				j[ID] = stoi(id);
			} catch (const invalid_argument& ia) {
				j[ID] = "ERROR";
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
			try {
				j[PAGE_COUNT] = stoi(page_count);
			} catch (const invalid_argument& ia) {
				j[PAGE_COUNT] = "ERROR";
			}
			break;
		}
		case DELETE_BOOK: {
			string id;
			cout << ID EQ;
			getline(cin, id);
			try {
				j[ID] = stoi(id);
			} catch (const invalid_argument& ia) {
				j[ID] = "ERROR";
			}
			break;
		}
		default: {
			break;
		}
	}
	return j;
}

/**
 * @brief Verify if the fields of the json are valid
 * 
 * @param j the json object
 * @return true if the fields are valid, false otherwise
 */
bool verify_json(json j) {
	bool is_valid = true;

	if (j.is_null()) {
		cout << MSG_INVALID_JSON;
		return false;
	}

	if (!j[USERNAME].is_null()) {
		string username = j[USERNAME];
		if (j[USERNAME].is_string() == false ||
			username.length() > 50 ||
			username.length() < 3 ||
			username.find_first_not_of(ALPHA_NUM) != string::npos) {
			cout << MSG_INVALID_USERNAME;
			is_valid = false;
		}
	}

	if (!j[PASSWORD].is_null()) {
		string password = j[PASSWORD];
		if (j[PASSWORD].is_string() == false ||
			password.length() > 50 ||
			password.length() < 3) {
			cout << MSG_INVALID_PASSWORD;
			is_valid = false;
		}
	}

	if (!j[TITLE].is_null()) {
		if (j[TITLE].is_string() == false) {
			cout << MSG_INVALID_TITLE;
			is_valid = false;
		}
	}

	if (!j[AUTHOR].is_null()) {
		if (j[AUTHOR].is_string() == false) {
			cout << MSG_INVALID_AUTHOR;
			is_valid = false;
		}
	}

	if (!j[GENRE].is_null()) {
		if (j[GENRE].is_string() == false) {
			cout << MSG_INVALID_GENRE;
			is_valid = false;
		}
	}

	if (!j[PUBLISHER].is_null()) {
		if (j[PUBLISHER].is_string() == false) {
			cout << MSG_INVALID_PUBLISHER;
			is_valid = false;
		}
	}

	if (!j[PAGE_COUNT].is_null()) {
		if (j[PAGE_COUNT].is_number() == false) {
			cout << MSG_INVALID_PAGE_COUNT;
			is_valid = false;
		}
	}

	if (!j[ID].is_null()) {
		if (j[ID].is_number() == false) {
			cout << MSG_INVALID_ID;
			is_valid = false;
		}
	}

	if (is_valid == false) {
		cout << MSG_TRY_AGAIN;
	}

	return is_valid;
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
	if (verify_json(j) == false) {
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


void parse_response(char* response) {
	string response_str(response);
    size_t pos = response_str.find("\r\n\r\n");
    if (pos == std::string::npos) {
        std::cout << "Invalid HTTP response" << std::endl;
        return;
    }

    /* Extract the JSON payload */
    std::string payload = response_str.substr(pos + 4);

    try {
        json jsonData = json::parse(payload);

        if (jsonData.contains("error") && jsonData["error"].is_string()) {
            std::cout << "Error: " << jsonData["error"].get<std::string>() << std::endl;
        } else if (jsonData.is_array()) {
			cout << '[';
			for (auto& elem : jsonData) {
				std::cout << elem << std::endl;
			}
			cout << ']';
		} else {
			std::cout << payload << std::endl;
		}
    } catch (const json::parse_error& e) {
        cout << MSG_SUCCESS;
    }
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
				parse_response(response);

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
				parse_response(response);

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
				parse_response(response);

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
				parse_response(response);

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
				parse_response(response);

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
				parse_response(response);

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
				parse_response(response);

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
				parse_response(response);

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