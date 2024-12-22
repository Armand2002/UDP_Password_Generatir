#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SERVER_PORT 12345
#define BUFFER_SIZE 64
#define MAX_PASSWORD_LENGTH 32
#define MIN_PASSWORD_LENGTH 6

// Structure for request and response messages
typedef struct {
    char type;             // Command type ('n', 'a', 'm', 's', 'u')
    int length;            // Password length
    char password[BUFFER_SIZE]; // Password or error message
} msg;

#endif // PROTOCOL_H
