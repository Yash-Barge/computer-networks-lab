#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8888
#define MAX_DATA_SEGMENT_SIZE 1024
#define TIMEOUT_IN_SECONDS 2

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
    struct sockaddr_in si_other;
    memset(&si_other, 0, sizeof si_other);
    int socket_fd, si_other_len = sizeof si_other;
    struct packet *send_pkt = malloc(sizeof(*send_pkt) + MAX_DATA_SEGMENT_SIZE);
    memset(send_pkt->data, 0, MAX_DATA_SEGMENT_SIZE);

    send_pkt->last_pkt = 0;
    send_pkt->ack_flag = 0;

    struct packet recv_ack_pkt;

    fd_set fdset;
    struct timeval timeout = { .tv_usec = 0 };

    FILE *in_file = fopen("input.txt", "r");

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket creation");
    
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");

    int recv_len, state = 0, finished = 0, final_pkt_sent = 0, select_ret_val;

    while (!finished) {
        char *temp;
        switch (state) {
            case 0:
                temp = fgets(send_pkt->data, MAX_DATA_SEGMENT_SIZE, in_file);
                send_pkt->payload_size = strlen(send_pkt->data);
                send_pkt->seq_no = 0;

                if (temp == NULL) {
                    final_pkt_sent = 1;
                    send_pkt->last_pkt = 1;
                    send_pkt->payload_size = 0;
                }

                printf("Seq. No. %1d of size %4d bytes\n", send_pkt->seq_no, sizeof(*send_pkt) + send_pkt->payload_size);
            case 10:
                if (sendto(socket_fd, send_pkt, sizeof(*send_pkt) + send_pkt->payload_size, 0, (struct sockaddr *) &si_other, si_other_len) == -1)
                    die("sendto()");
                
                state = 1;

                break;
            case 1:
                FD_ZERO(&fdset);
                FD_SET(socket_fd, &fdset);
                timeout.tv_sec = TIMEOUT_IN_SECONDS;

                select_ret_val = select(socket_fd + 1, &fdset, NULL, NULL, &timeout);

                if (select_ret_val == -1)
                    die("select state 1");
                else if (select_ret_val == 0) {
                    printf("RESENT DATA: Seq. No. %1d of size %4d bytes\n", send_pkt->seq_no, sizeof(*send_pkt) + send_pkt->payload_size);
                    state = 10;
                    break;
                }

                recv_len = recvfrom(socket_fd, &recv_ack_pkt, sizeof(recv_ack_pkt), 0, (struct sockaddr *) &si_other, &si_other_len);

                if (recv_len == -1)
                    die("recvfrom()");

                if (recv_ack_pkt.seq_no != 0) {
                    break;
                } else {
                    printf("RCVD ACK: for PKT with Seq. No. %1d\n", recv_ack_pkt.seq_no);

                    state = 2;

                    if (final_pkt_sent)
                        finished = 1;
                }

                break;
            case 2:
                temp = fgets(send_pkt->data, MAX_DATA_SEGMENT_SIZE, in_file);
                send_pkt->payload_size = strlen(send_pkt->data);
                send_pkt->seq_no = 1;

                if (temp == NULL) {
                    final_pkt_sent = 1;
                    send_pkt->last_pkt = 1;
                    send_pkt->payload_size = 0;
                }

                printf("Seq. No. %1d of size %4d bytes\n", send_pkt->seq_no, sizeof(*send_pkt) + send_pkt->payload_size);
            case 11:
                if (sendto(socket_fd, send_pkt, sizeof(*send_pkt) + send_pkt->payload_size, 0, (struct sockaddr *) &si_other, si_other_len) == -1)
                    die("sendto()");
                
                state = 3;
                
                break;
            case 3:
                FD_ZERO(&fdset);
                FD_SET(socket_fd, &fdset);
                timeout.tv_sec = TIMEOUT_IN_SECONDS;

                select_ret_val = select(socket_fd + 1, &fdset, NULL, NULL, &timeout);

                if (select_ret_val == -1)
                    die("select state 1");
                else if (select_ret_val == 0) {
                    printf("RESENT DATA: Seq. No. %1d of size %4d bytes\n", send_pkt->seq_no, sizeof(*send_pkt) + send_pkt->payload_size);
                    state = 11;
                    break ;
                }

                recv_len = recvfrom(socket_fd, &recv_ack_pkt, sizeof(recv_ack_pkt), 0, (struct sockaddr *) &si_other, &si_other_len);

                if (recv_len == -1)
                    die("recvfrom()");

                if (recv_ack_pkt.seq_no != 1) {
                    break;
                } else {
                    printf("RCVD ACK: for PKT with Seq. No. %1d\n", recv_ack_pkt.seq_no);
                    
                    state = 0;

                    if (final_pkt_sent)
                        finished = 1;
                }
                break;
        }
    }

    return 0;
}
