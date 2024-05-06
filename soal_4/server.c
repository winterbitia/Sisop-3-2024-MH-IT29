#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 8080
#define CMD_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 8192
#define CSV_PATH "/home/malvin/sisopmodul3/soal_4/myanimelist.csv"
#define LOG_PATH "/home/malvin/sisopmodul3/soal_4/change.log"

void write_to_log(const char *type, const char *message) {
    FILE *log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, 20, "%d/%m/%y - %I:%M %p", tm_info);

    fprintf(log_file, "[%s] [%s] %s\n", time_str, type, message);
    fclose(log_file);
}

void handle_command(int client_socket, const char *command) {
    char response[RESPONSE_BUFFER_SIZE] = {0};

    if (strcmp(command, "SHOW") == 0) {
        FILE *csv_file = fopen(CSV_PATH, "r");
        if (csv_file == NULL) {
            sprintf(response, "Error opening file: No such file or directory");
        } else {
            char line[1024];
            while (fgets(line, sizeof(line), csv_file)) {
                strcat(response, line);
            }
            fclose(csv_file);
        }
    } else if (strncmp(command, "GENRE", 5) == 0) {
        char genre[256] = {0};
        sscanf(command, "%*s %s", genre);

        char cmd[CMD_BUFFER_SIZE];
        snprintf(cmd, CMD_BUFFER_SIZE, "grep %s %s", genre, CSV_PATH);
        
        FILE *grep_pipe = popen(cmd, "r");
        if (grep_pipe == NULL) {
            perror("Error executing grep command");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), grep_pipe)) {
            strcat(response, line);
        }

        pclose(grep_pipe);
    } else if (strncmp(command, "DAY", 3) == 0) {
        char day[256] = {0};
        sscanf(command, "%*s %s", day);

        char cmd[CMD_BUFFER_SIZE];
        snprintf(cmd, CMD_BUFFER_SIZE, "grep %s %s", day, CSV_PATH);

        FILE *grep_pipe = popen(cmd, "r");
        if (grep_pipe == NULL) {
            perror("Error executing grep command");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), grep_pipe)) {
            strcat(response, line);
        }

        pclose(grep_pipe);
    } else if (strncmp(command, "STATUS", 6) == 0) {
        char status[256] = {0};
        sscanf(command, "%*s %s", status);

        char cmd[CMD_BUFFER_SIZE];
        snprintf(cmd, CMD_BUFFER_SIZE, "grep %s %s", status, CSV_PATH);

        FILE *grep_pipe = popen(cmd, "r");
        if (grep_pipe == NULL) {
            perror("Error executing grep command");
            exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), grep_pipe)) {
            strcat(response, line);
        }

        pclose(grep_pipe);
    } else if (strncmp(command, "ADD", 3) == 0 || strncmp(command, "EDIT", 4) == 0 || strncmp(command, "DEL", 3) == 0) {
        // Log the command
        write_to_log("ADD/EDIT/DEL", command);

        // Handle the command (add/edit/del) here
    
        // For now, just set the response to a placeholder message
        sprintf(response, "Command \"%s\" executed successfully", command);
    } else {
        // Invalid command
        sprintf(response, "Invalid Command");
    }

    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (1) {
        char command[1024] = {0};
        if (recv(new_socket, command, sizeof(command), 0) <= 0) {
            break;
        }

        handle_command(new_socket, command);
    }

    return 0;
}

