#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8040
#define BUFFER_SIZE 1024

int ss;
struct sockaddr_in sadd; 

void* handle_send(void* args) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        // Send message to the server
        sendto(ss, buffer, strlen(buffer), 0, (struct sockaddr *)&sadd, sizeof(sadd));    
    }
    
    return NULL;
}

void* handle_recv(void* args) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(ss, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0'; // Null-terminate the string
            printf("\n--> %s", buffer);
        }
    }
    
    return NULL;
}

int main() {
    pthread_t sendthread, recvthread;

    // Create a UDP socket
    ss = socket(AF_INET, SOCK_DGRAM, 0);
    if (ss < 0) {
        perror("error in creating socket");
        exit(1); 
    }

    // Initialize the server address
    memset(&sadd, 0, sizeof(sadd));
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Create sending and receiving threads
    if (pthread_create(&sendthread, NULL, handle_send, NULL) != 0) {
        perror("error in creating sending thread");
        close(ss);
        exit(1); 
    }
    if (pthread_create(&recvthread, NULL, handle_recv, NULL) != 0) {
        perror("error in creating receiving thread");
        close(ss);
        exit(1);
    }

    // Wait for threads to finish
    pthread_join(sendthread, NULL);
    pthread_join(recvthread, NULL);

    // Close the socket
    close(ss); 

    return 0;
}

