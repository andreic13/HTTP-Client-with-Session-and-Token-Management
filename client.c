#include <stdio.h>     /* printf, fgets, stdin, stdout */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

// am preluat din laboratorul 9 fisierele .c si .h: requests, helpers, buffer.
// si am inclus fisierele .c si .h :parson
// pentru a lucra cu formatul json

#define SERVER_ADDR "34.246.184.49"
#define SERVER_PORT 8080
#define SERVER_PATH_REGISTER "/api/v1/tema/auth/register"
#define SERVER_PATH_LOGIN "/api/v1/tema/auth/login"
#define SERVER_PATH_ACCES_LIB "/api/v1/tema/library/access"
#define SERVER_PATH_GET_BOOKS "/api/v1/tema/library/books"
#define SERVER_PATH_ADD_BOOK "/api/v1/tema/library/books"
#define SERVER_PATH_LOGOUT "/api/v1/tema/auth/logout"

char *cookie_for_connection = NULL; // variabila globala pentru cookie-ul de conexiune
char *token_jwt = NULL; // variabila globala pentru token-ul jwt pentru biblioteca

void case_register(int sockfd)
{
    char username[LINELEN], password[LINELEN];
    char *message, *response;

    // preiau username:
    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0'; // elimin \n de la final

    // preiau password:
    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0'; // elimin \n de la final

    // verifica daca username e gol:
    if (strlen(username) == 0) {
        printf("Eroare: Username-ul nu poate fi gol\n");
        return;
    }

    // verific daca apare spatiu in username:
    for (int i = 0; i < strlen(username); i++) {
        if (username[i] == ' ') {
            printf("Eroare: Username-ul nu poate contine spatii\n");
            return;
        }
    }

    // verifica daca password e gol:
    if (strlen(password) == 0) {
        printf("Eroare: Parola nu poate fi goala\n");
        return;
    }

    // verific daca apare spatiu in password:
    for (int i = 0; i < strlen(password); i++) {
        if (password[i] == ' ') {
            printf("Eroare: Parola nu poate contine spatii\n");
            return;
        }
    }

    // construiesc payload-ul pentru un singur obiect json:
    JSON_Value *init_value = json_value_init_object();
    JSON_Object *init_object = json_value_get_object(init_value);

    json_object_set_string(init_object, "username", username);
    json_object_set_string(init_object, "password", password);

    char* string_value = json_serialize_to_string(init_value);

    // trimit datele catre server
    message = compute_post_request(SERVER_ADDR, SERVER_PATH_REGISTER, 
                "application/json", &string_value, 1, NULL, NULL, 0);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Utilizatorul există deja!\n");
    } else {
        // altfel, afisez mesajul de succes:
        printf("Utilizator înregistrat cu succes!\n");
    }

    // eliberez memoria:
    json_value_free(init_value);
    json_free_serialized_string(string_value);
    free(message);
    free(response);
}

void case_login(int sockfd) {
    char username[LINELEN], password[LINELEN];
    char *message, *response;

    // preiau username:
    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0'; // elimin \n de la final

    // preiau password:
    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0'; // elimin \n de la final

    // verifica daca username e gol:
    if (strlen(username) == 0) {
        printf("Eroare: Username-ul nu poate fi gol\n");
        return;
    }

    // verific daca apare spatiu in username:
    for (int i = 0; i < strlen(username); i++) {
        if (username[i] == ' ') {
            printf("Eroare: Username-ul nu poate contine spatii\n");
            return;
        }
    }

    // verifica daca password e gol:
    if (strlen(password) == 0) {
        printf("Eroare: Parola nu poate fi goala\n");
        return;
    }

    // verific daca apare spatiu in password:
    for (int i = 0; i < strlen(password); i++) {
        if (password[i] == ' ') {
            printf("Eroare: Parola nu poate contine spatii\n");
            return;
        }
    }

    // construiesc payload-ul pentru un singur obiect json:
    // adaug username si password in body_data
    JSON_Value *init_value = json_value_init_object();
    JSON_Object *init_object = json_value_get_object(init_value);

    json_object_set_string(init_object, "username", username);
    json_object_set_string(init_object, "password", password);

    char* string_value = json_serialize_to_string(init_value);

    // trimit datele catre server
    message = compute_post_request(SERVER_ADDR, SERVER_PATH_LOGIN, 
                "application/json", &string_value, 1, NULL, NULL, 0);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Username sau parola gresite\n");
    } else {
        // altfel, afisez mesajul de succes:
        printf("Utilizatorul a fost logat cu succes\n");

        // extrag cookie-ul din json:
        char *pointer_to_cookie = strstr(response, "Cookie: ");
        pointer_to_cookie += strlen("Cookie: "); // sar peste "Cookie: "

        // elimin informatiile suplimentare (de la date) :
        char *pointer_to_cookie_bad = strstr(pointer_to_cookie, "Date:");
        
        // realoc fostul cookie de conexiune, daca exista:
        if (cookie_for_connection != NULL) {
            cookie_for_connection = realloc(cookie_for_connection, 
                                    strlen(pointer_to_cookie) - strlen(pointer_to_cookie_bad) + 1);
        } else {
            // aloc memorie pentru acesta:
            cookie_for_connection = calloc(strlen(pointer_to_cookie) - strlen(pointer_to_cookie_bad) + 1,
                                            sizeof(char));
        }
        // copiez cookie-ul in variabila globala:
        strncpy(cookie_for_connection, pointer_to_cookie, 
                strlen(pointer_to_cookie) - strlen(pointer_to_cookie_bad));

        // eliberez fostul token jwt, daca exista:
        if (token_jwt != NULL) {
            token_jwt = NULL;
            free(token_jwt);
        }
    }

    // eliberez memoria:
    json_value_free(init_value);
    json_free_serialized_string(string_value);
    free(message);
    free(response);
}

void enter_library(int sockfd) {
    char *message, *response;

    // verific daca am cookie-ul de conexiune:
    if (cookie_for_connection == NULL) {
        printf("Eroare: Nu sunteti logat!\n");
        return;
    }

    // trimit cerere pentru a prelua datele de la server:
    message = compute_get_request(SERVER_ADDR, SERVER_PATH_ACCES_LIB, NULL, NULL, &cookie_for_connection, 1);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server:
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Datele nu pot fi accesate\n");
    } else {
        // altfel, afisez mesajul de succes:
        printf("Utilizatorul a primit acces la biblioteca cu succes\n");

        // obtin token-ul jwt de acces:
        char *pointer_to_token = strstr(response, "token");
        pointer_to_token += strlen("token") + 3; // sar peste "token": "
        // tai finalul token-ului: " "} "
        pointer_to_token[strlen(pointer_to_token) - 2] = '\0';

        // realoc fostul token jwt, daca exista:
        if (token_jwt != NULL) {
            token_jwt = realloc(token_jwt, strlen(pointer_to_token) + 1);
        } else {
            // aloc memorie pentru acesta:
            token_jwt = calloc(strlen(pointer_to_token) + 1, sizeof(char));
        }

        // copiez token-ul in variabila globala:
        strcpy(token_jwt, pointer_to_token);
    }

    // eliberez memoria:
    free(message);
    free(response);
}

void get_books(int sockfd) {
    char *message, *response;

    // verific daca am token-ul jwt pentru a accesa biblioteca:
    if (token_jwt == NULL) {
        printf("Eroare: Nu aveti acces la biblioteca\n");
        return;
    }

    // trimit cerere pentru a prelua datele de la server:
    message = compute_get_request(SERVER_ADDR, SERVER_PATH_GET_BOOKS, NULL, token_jwt, NULL, 0);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server:
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Nu ati putut accesa biblioteca\n");
    } else {
        // altfel, afisez cartile disponibile:
        // iau raspunsul JSON din response:
        char *pointer_to_books = basic_extract_json_response_list(response);
        
        // afisez cartile disponibile:
        JSON_Value *init_value = json_parse_string(pointer_to_books);
        JSON_Array *books = json_value_get_array(init_value); // lista de obiecte json (carti)
        int books_size = json_array_get_count(books); // numarul de carti

        printf("[\n");
        for (int i = 0; i < books_size; i++) {
            JSON_Object *book = json_array_get_object(books, i);
            int current_id = json_object_get_number(book, "id"); // id-ul cartii
            const char *current_title = json_object_get_string(book, "title"); // titlul cartii
            
            // formatul json:
            printf(" {\n  \"id\": %d,\n  \"title\": \"%s\"\n }\n", current_id, current_title);
        }
        printf("]\n");
    }

    // eliberez memoria:
    free(message);
    free(response);
}

void add_book(int sockfd) {
    char title[LINELEN], author[LINELEN], genre[LINELEN], 
        publisher[LINELEN], page_count[LINELEN];
    
    char *message, *response;

    // preiau datele de la tastatura:
    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strlen(title) - 1] = '\0'; // elimin \n de la final
    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strlen(author) - 1] = '\0'; // elimin \n de la final
    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strlen(genre) - 1] = '\0'; // elimin \n de la final
    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strlen(publisher) - 1] = '\0'; // elimin \n de la final
    printf("page_count=");
    fgets(page_count, sizeof(page_count), stdin);
    page_count[strlen(page_count) - 1] = '\0'; // elimin \n de la final

    // testez ca si campurile sa nu fie goale:
    if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || 
        strlen(publisher) == 0 || strlen(page_count) == 0) {
        printf("Eroare: Nu puteti lasa campuri necompletate\n");
        return;
    }

    // testez ca page_count sa fie un numar:
    for (int i = 0; i < strlen(page_count) - 1; i++) {
        if (page_count[i] < '0' || page_count[i] > '9') {
            printf("Eroare: Tip de date incorect pentru numarul de pagini\n");
            return;
        }
    }

    // convertesc page_count in numar:
    int page_count_int = atoi(page_count);

    // verific daca am token-ul jwt pentru a accesa biblioteca:
    if (token_jwt == NULL) {
        printf("Eroare: Nu aveti acces la biblioteca\n");
        return;
    }


    // construiesc payload-ul pentru un singur obiect json:
    JSON_Value *init_value = json_value_init_object();
    JSON_Object *init_object = json_value_get_object(init_value);

    json_object_set_string(init_object, "title", title);
    json_object_set_string(init_object, "author", author);
    json_object_set_string(init_object, "genre", genre);
    json_object_set_string(init_object, "publisher", publisher);
    json_object_set_number(init_object, "page_count", page_count_int);

    char* string_value = json_serialize_to_string(init_value);

    // trimit datele catre server
    message = compute_post_request(SERVER_ADDR, SERVER_PATH_GET_BOOKS, 
                "application/json", &string_value, 1, token_jwt, NULL, 0);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Cartea nu a putut fi adaugata\n");
    } else {
        // altfel, afisez mesajul de succes:
        printf("Cartea a fost adaugata cu succes\n");
    }

    // eliberez memoria:
    json_value_free(init_value);
    json_free_serialized_string(string_value);
    free(message);
    free(response);
}

void get_book(int sockfd) {
    char input_id_str[LINELEN];
    char *message, *response;

    // preiau id-ul cartii de la tastatura:
    printf("id=");
    fgets(input_id_str, sizeof(input_id_str), stdin);
    input_id_str[strlen(input_id_str) - 1] = '\0'; // elimin \n de la final

    // testez ca input_id_str sa fie un numar:
    for (int i = 0; i < strlen(input_id_str); i++) {
        if (input_id_str[i] < '0' || input_id_str[i] > '9') {
            printf("Eroare: Tip de date incorect pentru id-ul cartii\n");
            return;
        }
    }

    // convertesc input_id_str in numar:
    int input_id = atoi(input_id_str);

    // verific daca am token-ul jwt pentru a accesa biblioteca:
    if (token_jwt == NULL) {
        printf("Eroare: Nu aveti acces la biblioteca\n");
        return;
    }

    // construiesc url-ul de la care preiau cartea:
    char *url = calloc(LINELEN, sizeof(char));
    sprintf(url, "/api/v1/tema/library/books/%d", input_id);

    // trimit cerere pentru a prelua datele de la server:
    message = compute_get_request(SERVER_ADDR, url, NULL, token_jwt, NULL, 0);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server:
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Id-ul introdus nu este atribuit unei carti!\n");
    } else {
        // altfel, afisez cartea ceruta:
        // iau raspunsul JSON din response:
        char *pointer_to_book = basic_extract_json_response(response);
        
        // afisez cartea ceruta:
        JSON_Value *init_value = json_parse_string(pointer_to_book);
        JSON_Object *book = json_value_get_object(init_value); // obiectul json (cartea)

        int current_id = json_object_get_number(book, "id"); // id-ul cartii
        const char *current_title = json_object_get_string(book, "title"); // titlul cartii
        const char *current_author = json_object_get_string(book, "author"); // autorul cartii
        const char *current_publisher = json_object_get_string(book, "publisher"); // editura cartii
        const char *current_genre = json_object_get_string(book, "genre"); // genul cartii
        int current_page_count = json_object_get_number(book, "page_count"); // numarul de pagini al cartii
            
        // formatul json: (era prea lung asa ca am separat printf in 2 randuri)
        printf("{\n \"id\": %d,\n \"title\": \"%s\",\n \"author\": \"%s\",\n \"publisher\": \"%s\",\n ", 
                current_id, current_title, current_author, current_publisher);
        printf("\"genre\": \"%s\",\n \"page_count\": %d\n}\n", current_genre, current_page_count);
    }

    // eliberez memoria:
    free(url);
    free(message);
    free(response);
}

void delete_book(int sockfd) {
    char input_id_str[LINELEN];
    char *message, *response;

    // preiau id-ul cartii de la tastatura:
    printf("id=");
    fgets(input_id_str, sizeof(input_id_str), stdin);
    input_id_str[strlen(input_id_str) - 1] = '\0'; // elimin \n de la final

    // testez ca input_id_str sa fie un numar:
    for (int i = 0; i < strlen(input_id_str); i++) {
        if (input_id_str[i] < '0' || input_id_str[i] > '9') {
            printf("Eroare: Tip de date incorect pentru id-ul cartii\n");
            return;
        }
    }

    // convertesc input_id_str in numar:
    int input_id = atoi(input_id_str);

    // verific daca am token-ul jwt pentru a accesa biblioteca:
    if (token_jwt == NULL) {
        printf("Eroare: Nu aveti acces la biblioteca\n");
        return;
    }


    // construiesc url-ul de la care vreau sa sterg cartea:
    char *url = calloc(LINELEN, sizeof(char));
    sprintf(url, "/api/v1/tema/library/books/%d", input_id);

    // trimit cererea de stergere catre server:
    message = compute_delete_request(SERVER_ADDR, url, token_jwt);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server:
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Id-ul introdus nu este atribuit unei carti!\n");
    } else {
        // altfel, afisez mesajul de succes:
        printf("Cartea cu id %d a fost stearsa cu succes\n", input_id);
    }

    // eliberez memoria:
    free(url);
    free(message);
    free(response);
}

void case_logout(int sockfd) {
    char *message, *response;

    // verific daca am cookie-ul de conexiune:
    if (cookie_for_connection == NULL) {
        printf("Eroare: Nu sunteti logat!\n");
        return;
    }

    // trimit cererea de logout catre server:
    message = compute_get_request(SERVER_ADDR, SERVER_PATH_LOGOUT, NULL, 
                                    NULL, &cookie_for_connection, 1);
    send_to_server(sockfd, message);

    // primesc raspunsul de la server:
    response = receive_from_server(sockfd);

    // daca am primit eroare afisez mesajul de eroare:
    if (strstr(response, "Bad") != NULL || strstr(response, "error") != NULL) {
        printf("Eroare: Utilizatorul nu a putut fi delogat\n");
    } else {
        // altfel, afisez mesajul de succes:
        printf("Utilizatorul s-a delogat cu succes!\n");

        // resetez cookie-ul de conexiune:
        if (cookie_for_connection != NULL) {
            cookie_for_connection = NULL;
            free(cookie_for_connection);
        }

        // resetez token-ul jwt:
        if (token_jwt != NULL) {
            token_jwt = NULL;
            free(token_jwt);
        }
    }

    // eliberez memoria:
    free(message);
    free(response);
}

void run_server()
{
    char input[LINELEN];

    while (1) {
        // citeste de la tastatura
        memset(input, 0, sizeof(input));
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0'; // elimin \n de la final

        // in functie de comanda, apelez functia corespunzatoare:
        if (strcmp(input, "register") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            case_register(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "login") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            case_login(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "enter_library") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            enter_library(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "get_books") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            get_books(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "get_book") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            get_book(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "add_book") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            add_book(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "delete_book") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            delete_book(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "logout") == 0) {
            int sockfd = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
            case_logout(sockfd);
            close_connection(sockfd);
        } else if (strcmp(input, "exit") == 0) {
            // eliberez cookie-ul de conexiune:
            if (cookie_for_connection != NULL) {
                cookie_for_connection = NULL;
                free(cookie_for_connection);
            }

            // eliberez token-ul jwt:
            if (token_jwt != NULL) {
                token_jwt = NULL;
                free(token_jwt);
            }

            // ies din program:
            break;
        } else {
            printf("Eroare: Input invalid\n");
        }
    }
}

int main(int argc, char *argv[])
{
    run_server();

    return 0;
}
