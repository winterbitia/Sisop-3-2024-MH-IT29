#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <time.h>

/*  
    Soal_1 db.c
    VERSION 1 - basic log writing
    Amoes Noland 5027231028
*/

// Define global variables
#define MAX_BUFFER 1028
char dir_entry[MAX_BUFFER],
     dir_db[MAX_BUFFER];
int keygen;

// Function to get maximum key number from auth
void keygen_get(){
    // Generate new shared memory ID
    key_t key = 1000;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer 
    int *keygen_max;
    keygen_max = shmat(shmid, NULL, 0);
    keygen = *keygen_max;

    // DEBUGGING
    // printf("max: %d\n", keygen);

    // Detach variable
    shmdt(keygen_max);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

// Function to write into log
void csv_log(char* type, char *filename){
    chdir(dir_db);
    FILE *file = fopen("db.log", "a");
    if (!file) return;

    // Set up time variables 
    time_t timevar; struct tm *timeinfo;
    time (&timevar); timeinfo = localtime (&timevar);

    // Output into log
    fprintf(file, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] [%s]\n",
            timeinfo->tm_mday,
            timeinfo->tm_mon,
            timeinfo->tm_year+1900,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec,
            type, filename);
    fclose(file);
}

// Function to move csv and call log
void csv_move(char *filename){
    // Check if file exists
    chdir(dir_entry);
    struct stat buffer;   
    if (stat(filename, &buffer) != 0) return;

    // Prepare file variables
    char source[MAX_BUFFER], dest[MAX_BUFFER];
    strcpy(source, dir_entry);
    strcat(source, filename);
    strcpy(dest, dir_db);
    strcat(dest, filename);

    // Move the file and start log
    rename(source, dest);
    if (strstr(filename, "_trashcan.csv") != NULL)
         csv_log("Trash Can", filename);
    else csv_log("Parking Lot", filename);
}

// Function to grab csv filenames from auth
void csv_get(){
    // DEBUGGING
    // printf("key: %d\n", keygen);

    // Generate new shared memory ID
    key_t key = (key_t)keygen--;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer
    char *shm_buffer = (char*)shmat(shmid, NULL, 0);

    // DEBUGGING
    // printf("buf: %s\n", shm_buffer);

    // Call file move
    csv_move(shm_buffer);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

int main(){
    // Chdir will guarantee working root
    chdir("../new-entry"); chdir("new-entry"); chdir("..");
    getcwd(dir_entry, sizeof(dir_entry));

    // Get entry and database directory
    strcpy(dir_db, dir_entry);
    strcat(dir_entry, "/new-entry/");
    strcat(dir_db, "/microservices/database/");

    // DEBUGGING
    // printf("dir: %s\n", dir_entry);    
    // printf("dir: %s\n", dir_db);    

    // Start main parsing process
    keygen_get();
    while (keygen > 1000) csv_get();

    printf("File(s) may have been moved."
          " Check the db.log for more information.");
    return 0;
}