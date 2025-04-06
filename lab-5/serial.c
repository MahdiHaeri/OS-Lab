#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL)); // Seed the random number generator
    int sampleCount = 5000; // Number of samples to generate
    int hist[25] = {0}; // Histogram array

    clock_t start = clock(); // Start timing

    for (int i = 0; i < sampleCount; i++) {
        int count = 0;
        for (int j = 0; j < 12; j++) {
            int randomNumber = rand() % 100; // Generate a random number between 0 and 99
            if (randomNumber < 50) {
                count++;
            } else {
                count--;
            }
        }

        int index = count + 12; // Adjust index to be between 0 and 24
        if (index >= 0 && index < 25) {
            hist[index]++; // Increment the histogram count
        }
    }

    clock_t end = clock(); // End timing
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC; // Calculate elapsed time

    // printf("Histogram:\n");
    // for (int i = 0; i < 25; i++) {
    //     printf("%2d: ", i);
    //     for (int j = 0; j < hist[i]; j++) {
    //         printf("*");
    //     }
    //     printf("\n");
    // }

    printf("Time taken: %f seconds\n", time_spent); // Print elapsed time
    return 0;
}
