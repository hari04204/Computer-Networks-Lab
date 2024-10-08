#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8040
#define CN 10

int clients[CN];
int ccount = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *brodcast(char *buffer, int sendersock) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < ccount; i++) {
        if (clients[i] != sendersock) {
            send(clients[i], buffer, strlen(buffer), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
    return NULL;
}

void *handle_server(void *args) {
    char buffer[1024];

    while (1) {
    	memset(buffer,0,sizeof(buffer)) ; 
        //printf("\nServer: ");
        fgets(buffer, sizeof(buffer), stdin);
        brodcast(buffer, -1);
    }

    return NULL;
}

void *handle_clients(void *cs) {
    int socket = *(int *)cs;
    int n;
    char buffer[1024];

    while ((n = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("\nClient [socket: %d]: %s", socket, buffer);
        brodcast(buffer, socket);
    }

    close(socket);

/*
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < ccount; i++) {
        if (clients[i] == socket) {
            clients[i] = clients[--ccount];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
*/
    free(cs);
    return NULL;
}

int main() {
    int ss, cs;
    struct sockaddr_in sadd, cladd;
    socklen_t claddlen = sizeof(cladd);
    pthread_t soutthread, cthread;

    if ((ss = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = INADDR_ANY;

    if (bind(ss, (struct sockaddr *)&sadd, sizeof(sadd)) < 0) {
        perror("bind failed");
        close(ss);
        exit(EXIT_FAILURE);
    }

    if (listen(ss, CN) < 0) {
        perror("listen failed");
        close(ss);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&soutthread, NULL, handle_server, NULL) != 0) {
        perror("pthread_create failed");
        close(ss);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((cs = accept(ss, (struct sockaddr *)&cladd, &claddlen)) < 0) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (ccount < CN) {
            clients[ccount++] = cs;
            int *newsock = malloc(sizeof(int));
            *newsock = cs;

            if (pthread_create(&cthread, NULL, handle_clients, (void *)newsock) != 0) {
                perror("pthread_create failed");
                close(cs);
                free(newsock);
            }
        } else {
            close(cs);
            printf("Maximum client limit reached\n");
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(ss);
    return 0;
}

