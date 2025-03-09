#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shared memory
#include <unistd.h>     // For sleep

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 1024

int main() {
    // Step 1: Open existing shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Step 2: Map shared memory into process address space
    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Step 3: Read message from shared memory
    printf("Reader: Message received -> %s\n", shm_ptr);

    // Cleanup: Unmap shared memory
    munmap(shm_ptr, SHM_SIZE);

    printf("Reader: Exiting...\n");
    return 0;
}

