#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#define BACKUP_FILE "current_number.txt"

volatile sig_atomic_t crashed = 0;

void saveState(int num) {
    FILE *backup = fopen(BACKUP_FILE, "w");
    if (backup != NULL) {
        fprintf(backup, "%d", num);
        fclose(backup);
    }
}

int getState(void) {
    FILE *backup = fopen(BACKUP_FILE, "r");
    int num = 0;
    if (backup != NULL) {
        fscanf(backup, "%d", &num);
        fclose(backup);
    }
    return num;
}

void crashHandler(int signum) {
    crashed = 1;
}


int main (void) {

    int num, prev = 0;
    while(1) {
        
        num = getState();
        if(prev == num) {
   	        break;  
        } 
        prev = num;
        sleep(1);
    }
   
   
   system("gnome-terminal -- ./main");
    for(int i = num+1; i < num+6; i++) {
        printf("num: %d\n", i);
        saveState(i);
        usleep(900*1000);
    }

    return 0;
}
