#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pthread.h>

/*  
    Soal_1 auth.c
    VERSION 1 - deletion only
    Amoes Noland 5027231028
*/

// Define global variables
#define MAX_BUFFER 512
char dir_main[MAX_BUFFER]; 
char dir_entry[MAX_BUFFER];

// Function to delete incorrect files
void dir_csv_check(){
    DIR *dir = opendir(dir_entry);
    struct dirent *ep;
    if (!dir) return;

    chdir(dir_entry);
    while(ep = readdir(dir)){
        if ((strcmp(ep->d_name, ".") == 0 )||
            (strcmp(ep->d_name, "..") == 0))
            continue;
        
        if ((strstr(ep->d_name, "trashcan.csv") == NULL)&&
            (strstr(ep->d_name, "parkinglot.csv") == NULL))
            remove(ep->d_name);
    }
    closedir(dir);
    return;
}

int main(){
    // Get main dir
    getcwd(dir_main, sizeof(dir_main));
    // Get new entry dir
    strcpy(dir_entry, dir_main);
    strcat(dir_entry, "/new-entry/"); 
    printf("%s\n", dir_entry);    

    // Start deletion
    dir_csv_check();

    return 0;
}