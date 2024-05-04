#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/*  
    Soal_3 driver.c
    VERSION 1 - barely exits
    Amoes Noland 5027231028
*/

#define IP         "127.0.0.1"
#define PORT       8080
#define MAX_BUFFER 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_address,
                                sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to address %s:%d\n", inet_ntoa(server_address.sin_addr),
                                           ntohs(server_address.sin_port));

    char buffer[MAX_BUFFER],
         response[MAX_BUFFER];

    while(1){
        printf("\n[DRIVER ASSISTANT]\n(COMMANDS: Gap, Fuel, Tire, TireChange, Exit)\n\nDriver  : ");
        memset(buffer, 0, MAX_BUFFER);
        fgets(buffer, MAX_BUFFER, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "Exit") == 0){
            printf("Client exited");
            return 0;
        }

        send(client_socket, buffer, strlen(buffer), 0);

        memset(response, 0, MAX_BUFFER);
        
        int bytes_read = read(client_socket, response, MAX_BUFFER);
        if (bytes_read < 0) {
            perror("Read failed");
            close(client_socket);
            continue;
        } else if (bytes_read == 0) {
            printf("Server disconnect\n");
            break;
        }

        printf("Paddock : %s\n", response);
    }

    close(client_socket);
    return 0;
}