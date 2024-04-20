#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PDR 10

#define PORT 8888
#define MAX_DATA_SEGMENT_SIZE 1024
#define RANDOM_FAIL ((rand() % 100) < PDR)

void die(char *err_msg) {
    perror(err_msg);
    exit(1);
}

struct packet {
    unsigned int payload_size;
    unsigned int seq_no;
    unsigned int last_pkt;
    unsigned int ack_flag;
    char data[];
};

int main(void) {
    FILE *out_file = fopen("output.txt", "w");

    srand(time(NULL));

    struct sockaddr_in si_me, si_other;
    memset(&si_me, 0, sizeof si_me);
    int socket_fd, si_other_len = sizeof si_other;
    struct packet *recv_pkt = malloc(sizeof(*recv_pkt) + MAX_DATA_SEGMENT_SIZE);
    memset(recv_pkt->data, 0, MAX_DATA_SEGMENT_SIZE);
    struct packet ack_pkt = { .ack_flag = 1, .last_pkt = 1, .payload_size = 0 };

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket creation");
    
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_fd, (struct sockaddr *) &si_me, sizeof si_me) == -1)
        die("port binding");

    int recv_len, state = 0, finished = 0;

    while (!finished) {
        switch (state) {
            case 0:
                recv_len = recvfrom(socket_fd, recv_pkt, sizeof(*recv_pkt) + MAX_DATA_SEGMENT_SIZE, 0, (struct sockaddr *) &si_other, &si_other_len);

                if (recv_len == -1)
                    die("recvfrom() state 0");

                if (RANDOM_FAIL) {
                    printf("DROP DATA: Seq. No. %1d of size %4d bytes\n", recv_pkt->seq_no, recv_len);
                    continue;
                } else
                    printf("RCVD DATA: Seq. No. %1d of size %4d bytes\n", recv_pkt->seq_no, recv_len);
                
                if (recv_pkt->seq_no != 0) {
                    ack_pkt.seq_no = 1;
                    if (sendto(socket_fd, &ack_pkt, sizeof ack_pkt, 0, (struct sockaddr *) &si_other, si_other_len) == -1)
                        die("sendto()");
                } else {
                    fwrite(recv_pkt->data, 1, recv_pkt->payload_size, out_file);
                    ack_pkt.seq_no = 0;
                    state = 1;
                    if (sendto(socket_fd, &ack_pkt, sizeof ack_pkt, 0, (struct sockaddr *) &si_other, si_other_len) == -1)
                        die("sendto()");
                    
                    printf("SENT ACK: for PKT with Seq. no. %1d\n", ack_pkt.seq_no);

                    if (recv_pkt->last_pkt)
                        finished = 1;
                }

                break;
            case 1:
                recv_len = recvfrom(socket_fd, recv_pkt, sizeof(*recv_pkt) + MAX_DATA_SEGMENT_SIZE, 0, (struct sockaddr *) &si_other, &si_other_len);


                if (recv_len == -1)
                    die("recvfrom() state 0");

                if (RANDOM_FAIL) {
                    printf("DROP DATA: Seq. No. %1d of size %4d bytes\n", recv_pkt->seq_no, recv_len);
                    continue;
                } else
                    printf("RCVD DATA: Seq. No. %1d of size %4d bytes\n", recv_pkt->seq_no, recv_len);
                
                if (recv_pkt->seq_no != 1) {
                    ack_pkt.seq_no = 0;
                    if (sendto(socket_fd, &ack_pkt, sizeof ack_pkt, 0, (struct sockaddr *) &si_other, si_other_len) == -1)
                        die("sendto()");
                } else {
                    fwrite(recv_pkt->data, 1, recv_pkt->payload_size, out_file);
                    ack_pkt.seq_no = 1;
                    state = 0;
                    if (sendto(socket_fd, &ack_pkt, sizeof ack_pkt, 0, (struct sockaddr *) &si_other, si_other_len) == -1)
                        die("sendto()");
                    
                    printf("SENT ACK: for PKT with Seq. no. %1d\n", ack_pkt.seq_no);

                    if (recv_pkt->last_pkt)
                        finished = 1;
                }

                break;
        }
    }

    close(socket_fd);
    fclose(out_file);

    return 0;
}
