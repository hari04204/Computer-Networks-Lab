#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <unistd.h>

#define PORT 8040
#define MAXCLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAXCLIENTS];
struct sockaddr_in client_addrs[MAXCLIENTS];
int count = 0;

pthread_mutex_t clientMutex = PTHREAD_MUTEX_INITIALIZER;

int ss; // Server socket
struct sockaddr_in sadd; // Server address

void brodcast(char* buffer, int sender_index) {
    pthread_mutex_lock(&clientMutex);

    for (int i = 0; i < count; i++) {
        if (i != sender_index) {
            sendto(ss, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addrs[i], sizeof(client_addrs[i]));
        }
    }

    pthread_mutex_unlock(&clientMutex);
}

void* handle_recv(void* args) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int len = recvfrom(ss, buffer, sizeof(buffer) - 1, 0,(struct sockaddr*)&client_addr, &client_addr_len);
        if (len > 0) {
            buffer[len] = '\0'; // Null-terminate the string
            printf("Received message: %s\n", buffer);

            // Check if the sender is a new client
            int client_found = 0;
            pthread_mutex_lock(&clientMutex);
            
            for (int i = 0; i < count; i++) {
                if ( client_addrs[i].sin_addr.s_addr == client_addr.sin_addr.s_addr && client_addrs[i].sin_port == client_addr.sin_port) {
                    client_found = 1;
                    break;
                }
            }

            if (!client_found && count < MAXCLIENTS) {
                client_addrs[count] = client_addr;
                clients[count] = count; // Dummy ID for the client
                count++;
            }
            pthread_mutex_unlock(&clientMutex);

            brodcast(buffer, -1); // Broadcast to all clients
        }
    }

    return NULL;
}

void* handle_send(void* args) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        brodcast(buffer, -1); // Broadcast to all clients
    }

    return NULL;
}

int main() {
    socklen_t saddlen = sizeof(sadd);
    pthread_t sendthread, recvthread;

    ss = socket(AF_INET, SOCK_DGRAM, 0);
    if (ss < 0) {
        perror("error in creating socket");
        exit(1);
    }

    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = INADDR_ANY;

    if (bind(ss, (struct sockaddr*)&sadd, sizeof(sadd)) < 0) {
        perror("error in binding the socket");
        close(ss);
        exit(1);
    }

    if (pthread_create(&sendthread, NULL, handle_send, NULL) != 0) {
        perror("error in creating send thread");
        close(ss);
        exit(1);
    }

    if (pthread_create(&recvthread, NULL, handle_recv, NULL) != 0) {
        perror("error in creating recv thread");
        close(ss);
        exit(1);
    }

    pthread_join(sendthread, NULL);
    pthread_join(recvthread, NULL);

    close(ss);
    return 0;
}

