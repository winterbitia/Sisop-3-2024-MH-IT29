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
    Soal_1 rate.c
    VERSION 1 - infrastructure
    Amoes Noland 5027231028
*/

// Define global variables
#define MAX_BUFFER 1028
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
    printf("max: %d\n", keygen);

    // Detach variable
    shmdt(keygen_max);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

void csv_parse(char* filename){
    return; 
}

// Function to grab csv filenames from auth
void csv_get(){
    // DEBUGGING
    printf("key: %d\n", keygen);

    // Generate new shared memory ID
    key_t key = (key_t)keygen--;
    int shmid = shmget(key, MAX_BUFFER, IPC_CREAT | 0666);

    // Write into buffer
    char *shm_buffer = (char*)shmat(shmid, NULL, 0);

    // DEBUGGING
    printf("buf: %s\n", shm_buffer);

    // Detach buffer to prepare next ID
    shmdt(shm_buffer);

    // DESTROY Shared memory (for testing)
    // shmctl(shmid, IPC_RMID, NULL);
}

int main(){
    keygen_get();
    while (keygen > 1000) csv_get();

    return 0;
}