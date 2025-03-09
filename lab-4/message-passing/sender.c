#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define FIFO_NAME "/tmp/my_fifo"

int main() {
    int fd;
    char message[256];

    // Create FIFO if it does not exist
    mkfifo(FIFO_NAME, 0666);

    printf("Sender: Enter a message: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';  // Remove newline

    // Open FIFO for writing
    fd = open(FIFO_NAME, O_WRONLY);
    write(fd, message, strlen(message) + 1);
    close(fd);

    printf("Sender: Message sent!\n");
    return 0;
}

