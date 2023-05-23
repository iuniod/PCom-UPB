#include "requests.h"

char *compute_get_request(const char *host, const char *url, char *query_params,
                            char **cookies, int cookies_count, const char *auth_jwt)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth_jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_jwt);
        compute_message(message, line);
    }

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }
    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(const char *host, const char *url, const char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, const char *auth_jwt)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char*) calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth_jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_jwt);
        compute_message(message, line);
    }
    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    for (int i = 0; body_data != NULL && i < body_data_fields_count; i++) {
        // if (i == 0) {
        //     strcat(body_data_buffer, "&");
        // }
        strcat(body_data_buffer, body_data[i]);
    }

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %lu", strlen(body_data_buffer));
    compute_message(message, line);
    // Step 4 (optional): add cookies
    if (cookies != NULL) {
       
    }
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}

char* compute_delete_request(const char *host, const char *url, char *query_params,
                            char **cookies, int cookies_count, const char *auth_jwt)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (auth_jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_jwt);
        compute_message(message, line);
    }

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }
    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

std::string extract_cookie(const char* response) {
    const char* cookie = strstr(response, "connect.sid");
    if (cookie == nullptr) {
        return "";
    }
    cookie += strlen("connect.sid") + 2;

    const char* end = strstr(cookie, ";");
    if (end == nullptr) {
        return "";
    }

    std::string auth_cookie(cookie, end - cookie);

    return auth_cookie;
}

std::string extract_library_token(const char* response) {
    const char* token = strstr(response, "token");
    if (token == nullptr) {
        return "";
    }
    token += strlen("token") + 3;

    const char* end = strstr(token, "\"");
    if (end == nullptr) {
        return "";
    }

    std::string auth_token(token, end - token);

    return auth_token;
}