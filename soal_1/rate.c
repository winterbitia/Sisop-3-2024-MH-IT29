#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>

/*  
    Soal_1 rate.c
    VERSION 3 - allow read from db
    Amoes Noland 5027231028
*/

// Define global variables
#define MAX_BUFFER 1028
char dir_entry[MAX_BUFFER];
int keygen;

// Define comparison variables
char max_t_name[MAX_BUFFER],
     max_p_name[MAX_BUFFER],
     max_t_fname[MAX_BUFFER],
     max_p_fname[MAX_BUFFER];
float max_t = 0, max_p = 0;

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

void csv_parse(char* filename){
    // Open the selected file
    chdir(dir_entry);
    FILE *file = fopen(filename, "r");

    // Checks database if not in entry
    if (!file) {
        chdir("../microservices/database");
        file = fopen(filename, "r");
        if (!file) return;
    }

    // Ignore csv header
    fscanf(file, "%*s");
    // Start parsing data
    // Ignore trailing newline and parse commas
    char name[MAX_BUFFER]; float rating;
    while (fscanf(file, "%*c%[^,],%f", name, &rating) == 2){
        // Already authenticated, which means
        // if not trashcan, guaranteed parking lot
        if (strstr(filename, "_trashcan.csv") != NULL){
            if (rating < max_t) continue;
            max_t = rating;
            strcpy(max_t_name, name);
            strcpy(max_t_fname, filename);
        } else {
            if (rating < max_p) continue;
            max_p = rating;
            strcpy(max_p_name, name);
            strcpy(max_p_fname, filename);
        }
    }
    fclose(file);
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

    // Send buffer to parse
    csv_parse(shm_buffer);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

void rating(char* type, char* fname, char* name, float rating){
    printf("\nType: %s\n"
           "Filename: %s\n"
           "--------------------\n"
           "Name: %s\n"
           "Rating: %.1f\n",
           type, fname, name, rating);
}

int main(){
    // Chdir will guarantee new-entry
    chdir("../new-entry"); chdir("new-entry");
    getcwd(dir_entry, sizeof(dir_entry));

    // DEBUGGING
    // printf("dir: %s\n", dir_entry);    

    // Start main parsing process
    keygen_get();
    while (keygen > 1000) csv_get();

    // Get final output
    rating("Trash Can"  ,max_t_fname,max_t_name,max_t);
    rating("Parking Lot",max_p_fname,max_p_name,max_p);

    return 0;
}