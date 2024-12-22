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
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main() {
#if defined WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 0;
    }
#endif

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    msg request, response;
    char buffer[sizeof(msg)];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        clearwinsock();
        return 1;
    }

    server = gethostbyname("passwdgen.uniba.it");
    if (server == NULL) {
        fprintf(stderr, "Error resolving server address\n");
        clearwinsock();
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_addr.sin_port = htons(SERVER_PORT);

    while (1) {
        printf("Enter command: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "q") == 0) {
            break;
        }

        if (strcmp(buffer, "h") == 0) {
            printf("Password Generator Help Menu\n");
            printf("Commands:\n");
            printf("n LENGTH : numeric password\n");
            printf("a LENGTH : alphabetic password\n");
            printf("m LENGTH : mixed password\n");
            printf("s LENGTH : secure password\n");
            printf("u LENGTH : unambiguous secure password\n");
            printf("q : quit\n");
            continue;
        }

        if (sscanf(buffer, "%c %d", &request.type, &request.length) != 2) {
            printf("Invalid input. Type 'h' for help.\n");
            continue;
        }

        if (request.length < MIN_PASSWORD_LENGTH || request.length > MAX_PASSWORD_LENGTH) {
            printf("Invalid length. Must be between 6 and 32.\n");
            continue;
        }

        // Copy request to buffer and send
        memcpy(buffer, &request, sizeof(msg));
        sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Receive response into buffer and copy to response structure
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (n < 0) {
            perror("Failed to receive");
            continue;
        }
        memcpy(&response, buffer, sizeof(msg));

        if (response.type == 'e') {
            printf("Error: %s\n", response.password);
        } else {
            printf("Generated password: %s\n", response.password);
        }
    }

    closesocket(sockfd);
    clearwinsock();
    return 0;
}
