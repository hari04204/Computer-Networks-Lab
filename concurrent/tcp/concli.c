#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8040

int ss; // Global socket descriptor

// Thread function for sending data
void *send_thread(void *arg) {
    char sendbuffer[1024];
    while (1) {
        memset(sendbuffer, 0, sizeof(sendbuffer));
        //printf("Me: ");
        if (fgets(sendbuffer, sizeof(sendbuffer), stdin) == NULL) {
            perror("fgets failed");
            break;
        }
        if (send(ss, sendbuffer, strlen(sendbuffer), 0) < 0) {
            perror("send failed");
            break;
        }
    }
    return NULL;
}

// Thread function for receiving data
void *recv_thread(void *arg) {
    char recvbuffer[1024];
    while (1) {
        memset(recvbuffer, 0, sizeof(recvbuffer));
        int n = recv(ss, recvbuffer, sizeof(recvbuffer) - 1, 0);
        if (n < 0) {
            perror("recv failed");
            break;
        }
        recvbuffer[n] = '\0'; // Null-terminate the string
        printf("--> %s", recvbuffer);
    }
    return NULL;
}

int main() {
    struct sockaddr_in sadd;

    // Create socket
    if ((ss = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(ss, (struct sockaddr *)&sadd, sizeof(sadd)) < 0) {
        perror("connect failed");
        close(ss);
        exit(EXIT_FAILURE);
    }

    pthread_t send_tid, recv_tid;

    // Create threads for sending and receiving
    if (pthread_create(&send_tid, NULL, send_thread, NULL) != 0) {
        perror("Failed to create send thread");
        close(ss);
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&recv_tid, NULL, recv_thread, NULL) != 0) {
        perror("Failed to create recv thread");
        close(ss);
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish
    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);

    close(ss);
    return 0;
}

