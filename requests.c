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

char *compute_get_request(char *host, char *url, char *query_params, char* token_jwt,
                            char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN); // resetez line buffer
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    // Daca am token_jwt, adaug header-ul corespunzator:
    if (token_jwt != NULL) {
        memset(line, 0, LINELEN); // resetez line buffer
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    if (cookies != NULL) {
        memset(line, 0, LINELEN); // resetez line buffer
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i != cookies_count - 1) {
                strcat(line, "; "); // separ cookie-urile prin ;
            }
        }
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type,
                            char **body_data, int body_data_fields_count,
                            char* token_jwt, char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    memset(line, 0, LINELEN); // resetez line buffer
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    memset(line, 0, LINELEN); // resetez line buffer
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // Construiesc body_data_buffer pentru a obtine size-ul corect:
    // functie creata in cadrul laboratorului, la indrumarea titularului
    strcpy(body_data_buffer, body_data[0]);

    for (int i = 1; i < body_data_fields_count; i++)
    {
        strcat(body_data_buffer, "&");
        strcat(body_data_buffer, body_data[i]);
    }
    
    int content_length = strlen(body_data_buffer);

    memset(line, 0, LINELEN); // resetez line buffer
    sprintf(line, "Content-Length: %d", content_length);
    compute_message(message, line);

    // Daca am token_jwt, adaug header-ul corespunzator:
    if (token_jwt != NULL) {
        memset(line, 0, LINELEN); // resetez line buffer
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    // Step 4 (optional): add cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN); // resetez line buffer
        strcat(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i != cookies_count - 1) {
                strcat(line, "; "); // separ cookie-urile prin ;
            }
        }
        compute_message(message, line);
    }
    
    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(body_data_buffer, 0, LINELEN); // resetez bufferul
    strcat(body_data_buffer, *body_data);
    compute_message(message, body_data_buffer);

    free(line);
    free(body_data_buffer);

    return message;
}

char *compute_delete_request(char *host, char *url, char* token_jwt) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2: add the host
    memset(line, 0, LINELEN); // resetez line buffer
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    // Daca am token_jwt, adaug header-ul corespunzator:
    if (token_jwt != NULL) {
        memset(line, 0, LINELEN); // resetez line buffer
        sprintf(line, "Authorization: Bearer %s", token_jwt);
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");

    free(line);
    return message;
}