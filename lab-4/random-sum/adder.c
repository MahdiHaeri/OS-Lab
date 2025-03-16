#include <stdio.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main() {

    // open shared memory
    int shm_fd = shm_open("shared_memory", O_RDWR, 0666);

    if (shm_fd == -1) {
        printf("Shared memory failed\n");
        return -1;
    }

    // map the shared memory in the address space of the process.
    int *ptr = (int *)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        return -1;
    }

    // add the two numbers stored in the shared memory.
    int a = *ptr;
    int b = *(ptr + 1);

    printf("Sum of the a: %d and b: %d is %d\n", a, b, a + b);

    // remove the shared memory.
    shm_unlink("shared_memory");
    return 0;
}