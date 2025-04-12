#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SAMPLE_SIZE 500000
#define STEPS 12
#define HIST_SIZE 25

void printHistogram(int* hist) {
    for (int i = 0; i < HIST_SIZE; i++) {
        printf("%2d: ", i - 12);
        int stars = hist[i] / 100;
        for (int j = 0; j < stars; j++) {
            printf("*");
        }
        printf("\n");
    }
}

int main() {
    int hist[HIST_SIZE] = {0};
    srand(time(NULL));

    // Start timer
    clock_t start = clock();

    for (int i = 0; i < SAMPLE_SIZE; i++) {
        int count = 0;
        for (int j = 0; j < STEPS; j++) {
            int num = rand() % 100;
            if (num >= 49) count++;
            else count--;
        }
        int index = count + 12;
        if (index < 0) index = 0;
        if (index >= HIST_SIZE) index = HIST_SIZE - 1;
        hist[index]++;
    }

    // End timer and calculate duration
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printHistogram(hist);
    printf("Execution time: %.6f seconds\n", time_spent);

    return 0;
}
