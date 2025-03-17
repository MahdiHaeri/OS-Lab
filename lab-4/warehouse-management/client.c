#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_host_name> <server_port_number> <client_name>\n", argv[0]);
        return 1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("connect");
        close(client_socket);
        return 1;
    }

    printf("Connected to server\n");

    // Send client name to server
    send(client_socket, argv[3], strlen(argv[3]), 0);

    // Check if server might reject the connection
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    // Set socket to non-blocking temporarily to check for immediate error response
    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    
    // Brief delay to allow server to respond if there's an error
    usleep(100000);  // 100ms
    
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    // Set socket back to blocking mode
    fcntl(client_socket, F_SETFL, flags);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        // If the message contains error about name or server full
        if (strstr(buffer, "Client name already in use") || strstr(buffer, "server full")) {
            printf("%s", buffer);
            close(client_socket);
            return 1;
        }
        
        // If we received some other message, print it
        printf("%s", buffer);
    }

    // Display welcome message regardless of server response
    printf("\nLogged in as: %s\n", argv[3]);
    printf("Available commands:\n");
    printf("  create <product_name> [quantity]  - Create a new product\n");
    printf("  add <product_name> <quantity>     - Add quantity to product\n");
    printf("  reduce <product_name> <quantity>  - Reduce quantity from product\n");
    printf("  remove <product_name>             - Remove a product (quantity must be 0)\n");
    printf("  send <client_name> <product_name> <quantity> - Send product to another client\n");
    printf("  list                              - List all products\n");
    printf("  quit                              - Disconnect from server\n");

    while (1) {
        printf("\n> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strlen(buffer) == 0) {
            continue;  // Skip empty commands
        }

        send(client_socket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            printf("Server disconnected\n");
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    close(client_socket);
    printf("Disconnected from server\n");
    return 0;
}