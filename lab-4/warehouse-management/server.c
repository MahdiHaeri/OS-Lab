#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define MAX_PRODUCTS 100
#define MAX_NAME_LENGTH 50
#define BUFFER_SIZE 1024

// Structure to store product information
typedef struct {
    char name[MAX_NAME_LENGTH];
    int quantity;
} Product;

// Structure to store client information
typedef struct {
    int socket;
    char name[MAX_NAME_LENGTH];
    Product inventory[MAX_PRODUCTS];
    int product_count;
} Client;

// Global array to store clients
Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to find a client by name
int find_client(const char *name) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Function to find a product in a client's inventory
int find_product(Client *client, const char *name) {
    for (int i = 0; i < client->product_count; i++) {
        if (strcmp(client->inventory[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Function to add a product to a client's inventory
int add_product(Client *client, const char *name, int quantity) {
    if (client->product_count >= MAX_PRODUCTS) {
        return -1; // Inventory full
    }
    int product_index = find_product(client, name);
    if (product_index != -1) {
        return -2; // Product already exists
    }
    strcpy(client->inventory[client->product_count].name, name);
    client->inventory[client->product_count].quantity = quantity;
    client->product_count++;
    return 0;
}

// Function to remove a product from a client's inventory
int remove_product(Client *client, const char *name) {
    int index = find_product(client, name);
    if (index == -1) {
        return -1; // Product not found
    }
    if (client->inventory[index].quantity != 0) {
        return -2; // Quantity not zero
    }
    for (int i = index; i < client->product_count - 1; i++) {
        client->inventory[i] = client->inventory[i + 1];
    }
    client->product_count--;
    return 0;
}

// Function to handle client requests
void *handle_client(void *arg) {
    Client *client = (Client *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            break; // Client disconnected or error
        }

        buffer[bytes_received] = '\0';
        char command[20], name[MAX_NAME_LENGTH], target_client_name[MAX_NAME_LENGTH];
        int amount;

        if (sscanf(buffer, "%s %s %s %d", command, target_client_name, name, &amount) == 4 && strcmp(command, "send") == 0) {
            // Send command
            pthread_mutex_lock(&clients_mutex);
            int target_client_index = find_client(target_client_name);
            if (target_client_index == -1) {
                send(client->socket, "Target client not found\n", strlen("Target client not found\n"), 0);
            } else {
                int product_index = find_product(client, name);
                if (product_index == -1 || client->inventory[product_index].quantity < amount) {
                    send(client->socket, "Insufficient quantity\n", strlen("Insufficient quantity\n"), 0);
                } else {
                    client->inventory[product_index].quantity -= amount;
                    int target_product_index = find_product(&clients[target_client_index], name);
                    if (target_product_index == -1) {
                        add_product(&clients[target_client_index], name, amount);
                    } else {
                        clients[target_client_index].inventory[target_product_index].quantity += amount;
                    }
                    send(client->socket, "OK\n", strlen("OK\n"), 0);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
        } else if (sscanf(buffer, "%s %s %d", command, name, &amount) == 3) {
            if (strcmp(command, "add") == 0) {
                int index = find_product(client, name);
                if (index == -1) {
                    send(client->socket, "Product not found\n", strlen("Product not found\n"), 0);
                } else {
                    client->inventory[index].quantity += amount;
                    send(client->socket, "OK\n", strlen("OK\n"), 0);
                }
            } else if (strcmp(command, "reduce") == 0) {
                int index = find_product(client, name);
                if (index == -1) {
                    send(client->socket, "Product not found\n", strlen("Product not found\n"), 0);
                } else {
                    client->inventory[index].quantity -= amount;
                    if (client->inventory[index].quantity < 0) client->inventory[index].quantity = 0;
                    send(client->socket, "OK\n", strlen("OK\n"), 0);
                }
            } else if (strcmp(command, "remove") == 0) {
                int result = remove_product(client, name);
                if (result == -1) {
                    send(client->socket, "Product not found\n", strlen("Product not found\n"), 0);
                } else if (result == -2) {
                    send(client->socket, "Quantity not zero\n", strlen("Quantity not zero\n"), 0);
                } else {
                    send(client->socket, "OK\n", strlen("OK\n"), 0);
                }
            } else if (strcmp(command, "create") == 0) {
                int result = add_product(client, name, amount);
                if (result == -1) {
                    send(client->socket, "Inventory Full\n", strlen("Inventory Full\n"), 0);
                } else if (result == -2) {
                    send(client->socket, "Product already exists\n", strlen("Product already exists\n"), 0);
                } else {
                    send(client->socket, "OK\n", strlen("OK\n"), 0);
                }
            }
        } else if (sscanf(buffer, "%s %s", command, name) == 2) {
            if (strcmp(command, "create") == 0) {
                int result = add_product(client, name, 0);
                if (result == -1) {
                    send(client->socket, "Inventory Full\n", strlen("Inventory Full\n"), 0);
                } else if (result == -2) {
                    send(client->socket, "Product already exists\n", strlen("Product already exists\n"), 0);
                } else {
                    send(client->socket, "OK\n", strlen("OK\n"), 0);
                }
            }
        } else if (strncmp(buffer, "list", 4) == 0) {
            char response[BUFFER_SIZE] = "";
            for (int i = 0; i < client->product_count; i++) {
                char product_info[100];
                sprintf(product_info, "%s: %d\n", client->inventory[i].name, client->inventory[i].quantity);
                strcat(response, product_info);
            }
            if (strlen(response) == 0) {
                strcpy(response, "No products in inventory\n");
            }
            send(client->socket, response, strlen(response), 0);
        } else if (strncmp(buffer, "quit", 4) == 0) {
            break;
        } else {
            send(client->socket, "Invalid command\n", strlen("Invalid command\n"), 0);
        }
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client->socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client->socket);
    printf("Client %s disconnected\n", client->name);
    free(client);
    pthread_exit(NULL);
}

// Server main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_socket);
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %s\n", argv[1]);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        char client_name[MAX_NAME_LENGTH];
        int bytes_received = recv(client_socket, client_name, MAX_NAME_LENGTH - 1, 0);
        if (bytes_received <= 0) {
            close(client_socket);
            continue;
        }
        client_name[bytes_received] = '\0';
        client_name[strcspn(client_name, "\n")] = 0;

        pthread_mutex_lock(&clients_mutex);
        if (find_client(client_name) != -1 || client_count >= MAX_CLIENTS) {
            send(client_socket, "Client name already in use or server full\n", strlen("Client name already in use or server full\n"), 0);
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        } else {
            // Send welcome message to confirm connection
            send(client_socket, "Connection successful\n", strlen("Connection successful\n"), 0);
        }

        Client *new_client = (Client *)malloc(sizeof(Client));
        if (!new_client) {
            perror("malloc");
            close(client_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }

        new_client->socket = client_socket;
        strcpy(new_client->name, client_name);
        new_client->product_count = 0;
        
        clients[client_count] = *new_client;
        client_count++;
        pthread_mutex_unlock(&clients_mutex);

        printf("Client %s connected\n", client_name);

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, new_client) != 0) {
            perror("pthread_create");
            close(client_socket);
            free(new_client);
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i].socket == client_socket) {
                    for (int j = i; j < client_count - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}