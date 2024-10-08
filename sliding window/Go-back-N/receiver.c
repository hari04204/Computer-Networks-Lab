#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define PORT 8080

typedef struct {
    int seq_num;
    char data[MAX_BUFFER_SIZE];
} Packet;

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    Packet packet;
    int expected_seq_num = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    while (1) {
        recvfrom(sockfd, &packet, sizeof(Packet), 0, (struct sockaddr*)&client_addr, &addr_len);
        printf("Received: %d - %s\n", packet.seq_num, packet.data);

        // Send ACK
        if (packet.seq_num == expected_seq_num) {
            sendto(sockfd, &expected_seq_num, sizeof(int), 0, (struct sockaddr*)&client_addr, addr_len);
            printf("Sent ACK: %d\n", expected_seq_num);
            expected_seq_num++;
        } else {
            // If it's not the expected packet, resend the last ACK
            sendto(sockfd, &expected_seq_num, sizeof(int), 0, (struct sockaddr*)&client_addr, addr_len);
        }
    }

    close(sockfd);
    return 0;
}
