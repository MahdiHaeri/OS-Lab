#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define SAMPLE_SIZE 500000
#define STEPS 12
#define HIST_SIZE 25
#define NUM_PROCESSES 4

void printHistogram(int* hist) {
    for (int i = 0; i < HIST_SIZE; i++) {
        printf("%2d: ", i - 12);
        for (int j = 0; j < hist[i]; j += 100) {
            printf("*");
        }
        printf("\n");
    }
}

int main() {
    // Start timing
    clock_t start = clock();
    
    int shmid = shmget(IPC_PRIVATE, HIST_SIZE * sizeof(int), IPC_CREAT | 0666);
    int* hist = (int*)shmat(shmid, NULL, 0);

    for (int i = 0; i < HIST_SIZE; i++) hist[i] = 0;

    srand(time(NULL));

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            int local_hist[HIST_SIZE] = {0};
            for (int j = 0; j < SAMPLE_SIZE / NUM_PROCESSES; j++) {
                int count = 0;
                for (int k = 0; k < STEPS; k++) {
                    int num = rand() % 100;
                    if (num >= 49) count++;
                    else count--;
                }
                int index = count + 12;
                if (index < 0) index = 0;
                if (index >= HIST_SIZE) index = HIST_SIZE - 1;
                local_hist[index]++;
            }
            for (int idx = 0; idx < HIST_SIZE; idx++) {
                hist[idx] += local_hist[idx];
            }
            shmdt(hist);
            exit(0);
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }

    printHistogram(hist);

    shmdt(hist);
    shmctl(shmid, IPC_RMID, NULL);
    
    // End timing and calculate duration
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Execution time: %.6f seconds\n", time_spent);
    return 0;
}
