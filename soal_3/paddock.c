#include "actions.c"
/* Imported: gap(float distance_now),
             fuel(float fuel_now),
             tire(int tire_now),
             tire_change(char* type_now) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

/*  
    Soal_3 paddock.c
    VERSION 4 - log writing
    Amoes Noland 5027231028
*/

// Global definitions
#define PORT       8080
#define MAX_BUFFER 1024
char dir_log[MAX_BUFFER];

// Touch fork function
void touch(char *item){
    pid_t pid = fork();
    if (pid < 0) {
        printf("Error: Fork failed\n");
        exit(1);
    }
    if (0 == pid){
        char *cmd = "/usr/bin/touch";
        char *arg[] = {"touch", item, NULL};
        execvp(cmd,arg);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
    }
}

// Log function
void log_write(char* source, char* command, char* argument){
    FILE *file = fopen(dir_log, "a");
    if (!file) return; 

    // Set up time variables 
    time_t timevar; struct tm *timeinfo;
    time (&timevar); timeinfo = localtime (&timevar);

    // Output into log
    fprintf(file, "[%s [%02d/%02d/%04d %02d:%02d:%02d]: [%s] [%s]\n",
            source,
            timeinfo->tm_mday,
            timeinfo->tm_mon,
            timeinfo->tm_year+1900,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec,
            command, argument);
    fclose(file);
}


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

    // DEBUGGING
    // printf("Server listening on port %d\n", PORT);

    // Define work variables
    int ohio_sigma_skibidi_rizz = 1;
    char command [MAX_BUFFER],
         argument[MAX_BUFFER],
         *response;
    getcwd(dir_log, sizeof(dir_log));
    strcat(dir_log, "/server/race.log");
    touch(dir_log);

    // Call daemonize function
    daemonize();

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

        // DEBUGGING
        printf("Accepted %s:%d\n", inet_ntoa(client_address.sin_addr),
                                   ntohs(client_address.sin_port));

    while(1){
        // DEBUGGING
        printf("Waiting for data\n");
        
        // Clear buffer and read from client
        memset(buffer, 0, MAX_BUFFER);
        int bytes_read = read(client_socket, buffer, MAX_BUFFER);
        if (bytes_read < 0) {
            perror("Read failed");
            break;
        } else if (bytes_read == 0) {
            printf("Client disconnect\n");
            break;
        }

        // DEBUGGING
        printf("Recieving data: %s\n", buffer);

        // Processing the buffer and creating a response
        sscanf(buffer, "%s %s", command, argument);
        if (strcmp(command, "Gap") == 0){
            response = gap(atof(argument));
 } else if (strcmp(command, "Fuel") == 0){
            response = fuel(atof(argument));
            if (!strchr(argument, '%'))
            strcat(argument, "%");
 } else if (strcmp(command, "Tire") == 0){
            response = tire(atoi(argument));
 } else if (strcmp(command, "TireChange") == 0){
            response = tire_change(argument);
        } else {
            strcpy(argument, "INVALID COMMAND");
            response = "Invalid command";
            ohio_sigma_skibidi_rizz = 0;
        }

        // Write results into the log
        log_write("Driver] ", command, argument);
        if (ohio_sigma_skibidi_rizz)
        log_write("Paddock]", command, response);
        ohio_sigma_skibidi_rizz = 1;

        // Send the output to the client
        if(send(client_socket, response, strlen(response), 0) < 0){
            perror("Send failed");
        };
    }
        // Close client if a break happens at input process
        close(client_socket);
        sleep(5);
    }
    // Close server if a break happens somehow
    close(server_socket);
    return 0;
}