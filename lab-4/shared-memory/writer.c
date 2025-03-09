#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shared memory
#include <unistd.h>     // For ftruncate

#define SHM_NAME "/my_shared_memory" // Shared memory name
#define SHM_SIZE 1024                // Shared memory size in bytes

int main() {
    // Step 1: Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Step 2: Set shared memory size
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Step 3: Map shared memory to process address space
    char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Step 4: Write message to shared memory
    printf("Writer: Enter a message to send: ");
    fgets(shm_ptr, SHM_SIZE, stdin);

    printf("Writer: Message written to shared memory.\n");
    
    // Step 5: Keep the process running for the reader to read
    printf("Writer: Press Enter to exit and remove shared memory...\n");
    getchar();

    // Cleanup: Unmap and remove shared memory
    munmap(shm_ptr, SHM_SIZE);
    shm_unlink(SHM_NAME); // Remove shared memory

    printf("Writer: Shared memory removed. Exiting...\n");
    return 0;
}

