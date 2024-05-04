#include "actions.c"
/* Imported:    gap(float distance_now),
                fuel(float fuel_now),
                tire(int tire_now),
                tire_change(char* type_now) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*  
    Soal_3 paddock.c
    VERSION 1 - no logs 
    Amoes Noland 5027231028
*/

#define PORT       8080
#define MAX_BUFFER 1024

// Daemonize the process
void daemonize(){
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    pid_t sid = setsid();
    if (sid < 0)        exit(EXIT_FAILURE);
    if (chdir("/") < 0) exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

// Main series of functions
int main(){
    // Call daemonize function
    // daemonize();

    // Socket variables
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(server_address);
    char buffer[MAX_BUFFER];

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind socket to server
    if (bind(server_socket, (struct sockaddr *)&server_address,
                             sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connection
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Define work variables
    char command [MAX_BUFFER],
         argument[MAX_BUFFER],
         *response;


    // Main server loop
    while(1){
        printf("waiting...\n");
        
        // Attempt to accept client socket
        if ((client_socket = accept(server_socket,
                                (struct sockaddr *)&server_address,
                                (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Accepted %s:%d\n", inet_ntoa(client_address.sin_addr),
                                   ntohs(client_address.sin_port));

    while(1){
        // Read buffer for command
        printf("Waiting for data\n");
        
        memset(buffer, 0, MAX_BUFFER);
        int bytes_read = read(client_socket, buffer, MAX_BUFFER);
        if (bytes_read < 0) {
            perror("Read failed");
            close(client_socket);
            continue;
        } else if (bytes_read == 0) {
            printf("Client disconnect\n");
            break;
        }


        printf("Recieving data: %s\n", buffer);

        sscanf(buffer, "%s %s", command, argument);
        if (strcmp(command, "Gap") == 0){
            float distance_now = atof(argument);
            response = gap(distance_now);
 } else if (strcmp(command, "Fuel") == 0){
            float fuel_now = atof(argument);
            response = fuel(fuel_now);
 } else if (strcmp(command, "Tire") == 0){
            int tire_now = atoi(argument);
            response = tire(tire_now);
 } else if (strcmp(command, "TireChange") == 0){
            response = tire_change(argument);
        } else {
            response = "Invalid command";
        }

        if(send(client_socket, response, strlen(response), 0) < 0){
            perror("Send failed");
        };
    }
        close(client_socket);
    }
        close(server_socket);
    return 0;
}