#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char command[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        if (strlen(command) <= 1) {
            printf("Please enter a valid command\n");
            continue;
        }

        // Remove trailing newline character
        if (command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }

        send(sock, command, strlen(command), 0);

        char response[8192] = {0};
        if (recv(sock, response, sizeof(response), 0) <= 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("Server response:\n%s\n", response);
    }

    close(sock);
    return 0;
}

