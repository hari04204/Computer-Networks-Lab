#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Function that handles communication with the client
void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    printf("Client connected, socket: %d\n", client_socket);

    // Loop to receive and echo data
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the string
        printf("Received: %s\n", buffer);
        
        // Send back the same data (echo)
        send(client_socket, buffer, bytes_received, 0);
    }

    // Close the client socket when done
    close(client_socket);
    printf("Client disconnected, socket: %d\n", client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port and address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Main server loop to accept and handle clients
    while (1) {
        // Accept a client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Allocate memory for the client socket and create a new thread
        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_sock_ptr) != 0) {
            perror("Thread creation failed");
            close(client_socket);
            free(client_sock_ptr);
        } else {
            pthread_detach(thread_id);  // Detach thread to avoid memory leaks
        }
    }

    // Close the server socket (though we won't reach here in this example)
    close(server_socket);
    return 0;
}
