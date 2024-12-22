#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

void generate_password(char *buffer, char type, int length) {
    const char *chars;
    int chars_length;

    if (type == 'n') {
        chars = "0123456789";
    } else if (type == 'a') {
        chars = "abcdefghijklmnopqrstuvwxyz";
    } else if (type == 'm') {
        chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    } else if (type == 's') {
        chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    } else {
        chars = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz234679";
    }

    chars_length = strlen(chars);

    for (int i = 0; i < length; i++) {
        buffer[i] = chars[rand() % chars_length];
    }
    buffer[length] = '\0';
}

int main() {
#if defined WIN32
    WSADATA wsa_data;
    typedef int socklen_t;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 0;
    }
#endif

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    msg request, response;

    srand(time(NULL));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        clearwinsock();
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        closesocket(sockfd);
        clearwinsock();
        return 1;
    }

    printf("Server is running on port %d\n", SERVER_PORT);

    while (1) {
        char buffer[sizeof(msg)];
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("Failed to receive");
            continue;
        }

        printf("New request from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Copia i dati ricevuti nella struttura `msg`
        memcpy(&request, buffer, sizeof(msg));

        if (request.length < MIN_PASSWORD_LENGTH || request.length > MAX_PASSWORD_LENGTH) {
            response.type = 'e';
            snprintf(response.password, BUFFER_SIZE, "Invalid length\n");
        } else {
            response.type = 'p';
            generate_password(response.password, request.type, request.length);
        }

        // Copia la struttura `msg` nel buffer per l'invio
        memcpy(buffer, &response, sizeof(msg));
        sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, client_len);
    }

    closesocket(sockfd);
    clearwinsock();
    return 0;
}
