#include <stdio.h>     // printf
#include <string.h>    // memset
#include <stdlib.h>    // exit
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>    // close
#include <time.h>      // for random num generator
 
#define BUFLEN 512  // Max length of buffer
#define PORT 8888   // The port on which to listen for incoming data
 
void die(char *s) {
    fprintf(stderr, "%s\n", s);
    exit(1);

    return;
}
 
int main(void) {
    struct sockaddr_in si_me;
    int s;
     
    // create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket");
     
    // zero out the structure
    memset(&si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    // bind socket to port
    if (bind(s, (struct sockaddr*) &si_me, sizeof(si_me)) == -1)
        die("bind");

    // initialize random num generator
    srand(time(NULL));
     
    // keep listening for data
    while (1) {
        struct sockaddr_in si_other;
        int recv_len, random_num, slen = sizeof(si_other);
        int guessed_num;
        char win_msg[] = "Congratulations, you guessed right!";
        char lose_msg[] = "The answer was 0, better luck next time!";

        printf("Waiting for client...\n");
         
        // try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, &guessed_num, sizeof(guessed_num), 0, (struct sockaddr *) &si_other, &slen)) == -1)
            die("recvfrom()");
         
        // print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Guessed number: %d\n", guessed_num);
        printf("Generated number: %d\n", (random_num = rand()%6 + 1));

        if (random_num == guessed_num) {
            printf("Client won, sending win message...\n");
            if (sendto(s, win_msg, sizeof(win_msg), 0, (struct sockaddr*) &si_other, slen) == -1)
                die("sendto() (win condition)");
        } else {
            printf("Client lost, sending lose message...\n");
            lose_msg[15] = '0' + random_num;
            if (sendto(s, lose_msg, sizeof(lose_msg), 0, (struct sockaddr*) &si_other, slen) == -1)
                die("sendto() (win condition)");
        }
        
    }

    close(s);

    return 0;
}
