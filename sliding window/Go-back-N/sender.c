#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define PACKET_SIZE sizeof(Packet)
#define PORT 8080
#define TIMEOUT 2

typedef struct {
    int seq_num;
    char data[MAX_BUFFER_SIZE];
} Packet;

// Hypothetical values for demonstration
#define BANDWIDTH 1000000 // 1 Mbps
#define RTT 0.1          // 100 ms
#define PACKET_SIZE_BYTES MAX_BUFFER_SIZE

int calculate_window_size() {
    // Calculate the BDP in packets
    int bdp = (BANDWIDTH * RTT) / (8 * PACKET_SIZE_BYTES);
    return bdp > 0 ? bdp : 1; // Ensure at least a window size of 1
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    Packet *packets;
    int base = 0, next_seq_num = 0;
    int ack_num;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int window_size = calculate_window_size();
    packets = malloc(window_size * PACKET_SIZE_BYTES);

    // Prepare packets
    while (next_seq_num < 100) { // Assuming you want to send 100 packets
        for (int i = 0; i < window_size && next_seq_num < 100; i++) {
            packets[i].seq_num = next_seq_num;

            snprintf(packets[i].data, MAX_BUFFER_SIZE, "Packet %d", next_seq_num); // writing data to packet
            sendto(sockfd, &packets[i], PACKET_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
            printf("Sent: %d\n", packets[i].seq_num);
            next_seq_num++;
        }

        // Wait for ACK
        fd_set readfds;
        struct timeval tv;
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) > 0) {
            recvfrom(sockfd, &ack_num, sizeof(int), 0, NULL, NULL);
            printf("Received ACK: %d\n", ack_num);
            if (ack_num >= base) {
                base = ack_num + 1; // Slide the window
                // Optionally recalculate the window size
                window_size = calculate_window_size();
                packets = realloc(packets, window_size * PACKET_SIZE_BYTES); // Resize packets array if needed
            }
        } else {
            printf("Timeout, resending packets...\n");
            // Resend the packets starting from the base
        }
    }

    free(packets);
    close(sockfd);
    return 0;
}
