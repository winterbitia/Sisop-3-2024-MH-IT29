#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/*  
    Soal_3 driver.c
    VERSION 3 - print optimization
    Amoes Noland 5027231028
*/

// Global definitions
#define IP         "127.0.0.1"
#define PORT       8080
#define MAX_BUFFER 1024

// I'm addicted to formatting
void line(){
    printf("-------------------------------------\n");
}

// Main series of functions
int main() {
    // Socket variables
    int client_socket;
    struct sockaddr_in server_address;

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Address conversion for communication
    if (inet_pton(AF_INET, IP, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server from client
    if (connect(client_socket, (struct sockaddr *)&server_address,
                                sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // DEBUGGING
    // printf("Connected to address %s:%d\n", inet_ntoa(server_address.sin_addr),
    //                                        ntohs(server_address.sin_port));

    // Define work variables
    char buffer[MAX_BUFFER],
         response[MAX_BUFFER];

    // Initial message
    printf("[CONNECTED TO DRIVER ASSISTANT]\n"); line();
    printf("Here are the available commands:\n"
           "  Gap        (float)  units\n"
           "  Fuel       (float)  percentage\n"
           "  Tire       (int)    usage\n"
           "  TireChange (string) Soft/Medium\n"
           "  Exit\n"); line();
    printf("~ GOOD LUCK ON THE RACE! ~\n");

    while(1){
        printf("\nInput Driver   : ");
        memset(buffer, 0, MAX_BUFFER);
        fgets(buffer, MAX_BUFFER, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "Exit") == 0){
            printf("Output Driver  : Client exited");
			close(client_socket);
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

        printf("Output Paddock : %s\n", response);
    }

    // Close client socket on disconnect
    close(client_socket);
    return 0;
}
