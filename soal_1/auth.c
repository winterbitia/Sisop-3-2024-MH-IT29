#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <pthread.h>

/*  
    Soal_1 auth.c
    VERSION 3 - send max key
    Amoes Noland 5027231028
*/

// Define global variables
#define MAX_BUFFER 1028
char dir_entry[MAX_BUFFER];
int keygen = 1001;

// Function to add filename to shared memory
void dir_csv_share(char* filename){
    // DEBUGGING
    printf("key: %d\n", keygen);

    // Generate new shared memory ID
    key_t key = (key_t)keygen++;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer
    char *shm_buffer = (char*)shmat(shmid, NULL, 0);
    strcpy(shm_buffer, filename);

    // DEBUGGING
    printf("buf: %s\n", shm_buffer);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

void keygen_send(){
    // Generate new shared memory ID
    key_t key = 1000;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer 
    int *keygen_max;
    keygen_max = shmat(shmid, NULL, 0);
    *keygen_max = --keygen;

    // DEBUGGING
    printf("max: %d\n", *keygen_max);

    // Detach variable
    shmdt(keygen_max);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

// Function to delete incorrect files
void dir_csv_check(){
    DIR *dir = opendir(dir_entry);
    struct dirent *ep;
    if (!dir) return;

    chdir(dir_entry);
    while(ep = readdir(dir)){
        // Skips the special items listed
        if ((strcmp(ep->d_name, ".") == 0 )||
            (strcmp(ep->d_name, "..") == 0))
            continue;
        
        // Deletes incorrect csv files
        if ((strstr(ep->d_name, "_trashcan.csv") == NULL)&&
            (strstr(ep->d_name, "_parkinglot.csv") == NULL)){
            remove(ep->d_name);
            continue;
        }

        // Add correct csv into shared memory
        dir_csv_share(ep->d_name);
    }
    closedir(dir);
    return;
}

int main(){
    // Get new entry dir
    getcwd(dir_entry, sizeof(dir_entry));
    strcat(dir_entry, "/new-entry/"); 
    
    // DEBUGGING
    printf("dir: %s\n", dir_entry);    

    // Start check
    dir_csv_check();

    // Share csv amount
    keygen_send();

    return 0;
}