#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define NUM_PROCESSES 4 // Number of child processes
#define SAMPLE_COUNT 500
#define HISTOGRAM_SIZE 25

// Structure to hold the shared histogram
typedef struct {
    int hist[HISTOGRAM_SIZE];
} SharedHistogram;

// Function to perform the simulation for a portion of the samples
void simulate(int start_index, int end_index, SharedHistogram *shared_hist) {
    srand(time(NULL) ^ getpid()); // Seed with different values for each process

    for (int i = start_index; i < end_index; i++) {
        int count = 0;
        for (int j = 0; j < 12; j++) {
            int randomNumber = rand() % 100;
            if (randomNumber < 50) {
                count++;
            } else {
                count--;
            }
        }

        int index = count + 12;
        if (index >= 0 && index < HISTOGRAM_SIZE) {
            // Atomically increment the histogram count
            __sync_fetch_and_add(&shared_hist->hist[index], 1);
        }
    }
}

int main() {
    // Allocate shared memory for the histogram
    SharedHistogram *shared_hist = mmap(NULL, sizeof(SharedHistogram),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_hist == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the shared histogram to zero
    for (int i = 0; i < HISTOGRAM_SIZE; i++) {
        shared_hist->hist[i] = 0;
    }

    clock_t start = clock();

    // Calculate the number of samples per process
    int samples_per_process = SAMPLE_COUNT / NUM_PROCESSES;
    int remainder = SAMPLE_COUNT % NUM_PROCESSES;

    // Create child processes
    pid_t pids[NUM_PROCESSES];
    int current_start = 0;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int current_end = current_start + samples_per_process + (i < remainder ? 1 : 0);
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Child process
            simulate(current_start, current_end, shared_hist);
            exit(EXIT_SUCCESS);
        }
        current_start = current_end;
    }

    // Wait for all child processes to complete
    for (int i = 0; i < NUM_PROCESSES; i++) {
        waitpid(pids[i], NULL, 0);
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Histogram (Multiprocessing):\n");
    for (int i = 0; i < HISTOGRAM_SIZE; i++) {
        printf("%2d: ", i);
        for (int j = 0; j < shared_hist->hist[i]; j++) {
            printf("*");
        }
        printf("\n");
    }

    printf("Time taken (Multiprocessing): %f seconds\n", time_spent);

    // Unmap the shared memory
    if (munmap(shared_hist, sizeof(SharedHistogram)) == -1) {
        perror("munmap failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}