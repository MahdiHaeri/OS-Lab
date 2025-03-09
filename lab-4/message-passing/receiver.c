#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/my_fifo"

int main() {
    int fd;
    char message[256];

    // Create FIFO if it does not exist
    mkfifo(FIFO_NAME, 0666);

    // Open FIFO for reading
    fd = open(FIFO_NAME, O_RDONLY);
    read(fd, message, sizeof(message));
    close(fd);

    printf("Receiver: Received message -> %s\n", message);

    // Remove FIFO after reading
    unlink(FIFO_NAME);

    return 0;
}

