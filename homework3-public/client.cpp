#include "client.h"

json get_json(string_code command) {
	json j;
	switch (command) {
		case REGISTER: {
			string username, password;
			cout << "username=";
			getline(cin, username);
			cout << "password=";
			getline(cin, password);
			j["username"] = username;
			j["password"] = password;
			break;
		}
		case LOGIN: {
			string username, password;
			cout << "username=";
			getline(cin, username);
			cout << "password=";
			getline(cin, password);
			j["username"] = username;
			j["password"] = password;
			break;
		}
		case GET_BOOK: {
			string id;
			cout << "id=";
			getline(cin, id);
			j["id"] = stoi(id);
			break;
		}
		case ADD_BOOK: {
			string title, author, genre, publisher, page_count;
			cout << "title=";
			getline(cin, title);
			cout << "author=";
			getline(cin, author);
			cout << "genre=";
			getline(cin, genre);
			cout << "publisher=";
			getline(cin, publisher);
			cout << "page_count=";
			getline(cin, page_count);
			j["title"] = title;
			j["author"] = author;
			j["genre"] = genre;
			j["publisher"] = publisher;
			j["page_count"] = stoi(page_count);
			break;
		}
		default: {
			break;
		}
	}
	return j;
}

char* get_body_data(string_code command, int &id) {
	json j = get_json(command);
	char *payload =  (char*) calloc(j.dump().length() + 1, sizeof(char));
	strcpy(payload, j.dump().c_str());
	if (command == GET_BOOK) {
		id = j["id"];
	}
	return payload;
}

string extract_cookie(char *response) {
	char *cookie = strstr(response, "Set-Cookie: ");
	if (cookie == NULL) {
		return "";
	}
	cookie += strlen("Set-Cookie: ");
	char *end = strstr(cookie, ";");
	if (end == NULL) {
		return "";
	}
	char *auth_cookie = (char*) calloc(end - cookie + 1, sizeof(char));
	strncpy(auth_cookie, cookie, end - cookie);
	return string(auth_cookie);
}

string extract_library_token(char *response) {
	char *token = strstr(response, "token");
	if (token == NULL) {
		return "";
	}
	token += strlen("token") + 3;
	char *end = strstr(token, "\"");
	if (end == NULL) {
		return "";
	}
	char *auth_token = (char*) calloc(end - token + 1, sizeof(char));
	strncpy(auth_token, token, end - token);
	return string(auth_token);
}

int main() {
	int socket_fd;
	string auth_cookie, auth_library_token;
	
	while (true) {
		// [TODO] Get command from user
		string command;
		getline(cin, command);

		// [TODO] Handle command
		string_code code = hashit(command);
		switch (code) {
			case LOGIN: {
				int trash = 0;
				char *payload = get_body_data(code, trash);

				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				char *message = compute_post_request(IP_ADDRESS, LOGIN_URL, "application/json", &payload, 1, NULL, 0, NULL);
				puts(message);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);

				if (strstr(response, RESPONSE_OK) != NULL) {
					auth_cookie = extract_cookie(response);
				}
				// [TODO] Close connection to server
				close_connection(socket_fd);

				break;
			}
			case REGISTER: {
				int trash = 0;
				char *payload = get_body_data(code, trash);
				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

 				char *message = compute_post_request(IP_ADDRESS, REGISTER_URL, "application/json", &payload, 1, NULL, 0, NULL);
				puts(message);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);
				// [TODO] Close connection to server
				close_connection(socket_fd);
				break;
			}
			case ENTER_LIBRARY: {
				if (auth_cookie == "") {
					cout << "You must be logged in to enter the library" << endl;
					break;
				}
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());

				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				char *message = compute_get_request(IP_ADDRESS, ENTER_LIBRARY_URL, NULL, &auth_cookie_str, 1, NULL);
				puts(message);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);

				if (strstr(response, RESPONSE_OK) != NULL) {
					auth_library_token = extract_library_token(response);
				}

				// [TODO] Close connection to server
				close_connection(socket_fd);
				break;
			}
			case GET_BOOKS: {
				if (auth_library_token == "") {
					cout << "You must be logged in to enter the library" << endl;
					break;
				}

				char *auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());

				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				char *message = compute_get_request(IP_ADDRESS, BOOKS_URL, NULL, NULL, 0, auth_library_token_str);
				puts(message);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);

				// [TODO] Close connection to server
				close_connection(socket_fd);


				break;
			}
			case GET_BOOK: {
				int id = -1;
				char *payload = get_body_data(code, id);

				string book_url = BOOK_URL + to_string(id);

				char* auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());
				

				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);

				char *message = compute_get_request(IP_ADDRESS, book_url.c_str(), NULL, &payload, 1, auth_library_token_str);
				puts(message);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);

				// [TODO] Close connection to server
				close_connection(socket_fd);
				break;
			}
			case ADD_BOOK: {
				int trash = 0;
				char *payload = get_body_data(code, trash);

				char *auth_library_token_str = (char*) calloc(auth_library_token.length() + 1, sizeof(char));
				strcpy(auth_library_token_str, auth_library_token.c_str());

				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
				break;
			}
			case DELETE_BOOK: {
				break;
			}
			case LOGOUT: {
				char *auth_cookie_str = (char*) calloc(auth_cookie.length() + 1, sizeof(char));
				strcpy(auth_cookie_str, auth_cookie.c_str());
				// [TODO] Open connection to server
				socket_fd = open_connection(IP_ADDRESS, PORT, AF_INET, SOCK_STREAM, 0);
				
				char *message = compute_get_request(IP_ADDRESS, LOGOUT_URL, NULL, &auth_cookie_str, 1, NULL);
				puts(message);
				send_to_server(socket_fd, message);
				char *response = receive_from_server(socket_fd);
				puts(response);

				if (strstr(response, RESPONSE_OK) != NULL) {
					auth_cookie = "";
				}
				// [TODO] Close connection to server
				close_connection(socket_fd);

				break;
			}
			case EXIT: {
				close_connection(socket_fd);
				return 0;
				break;
			}

			default: {
				cout << "Invalid command" << endl;
				break;
			}
		}
	}
}