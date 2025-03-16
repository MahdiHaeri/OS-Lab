#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main() {
    srand(time(NULL));

    // open shared memory 
    int shm_fd = shm_open("shared_memory", O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1) {
        printf("Shared memory failed\n");
        exit(-1);
    }

    // set the size of the shared memory.
    ftruncate(shm_fd, sizeof(int));

    // map the shared memory in the address space of the process.
    int *ptr = (int *)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        return -1;
    }

    // generate a random number and store it in the shared memory.
    int a = rand() % 100;
    int b = rand() % 100;

    *ptr = a;
    *(ptr + 1) = b;

    printf("Random numbers generated: %d, %d\n", a, b);

    // close the shared memory.
    close(shm_fd);
    return 0;
}