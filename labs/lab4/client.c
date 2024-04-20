#include <stdio.h>     // printf
#include <string.h>    // memset
#include <stdlib.h>    // exit
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>    // close
 
#define BUFLEN 512  // Max length of buffer
#define PORT 8888   // The port on which to send data
 
void die(char *s) {
    fprintf(stderr, "%s\n", s);
    exit(1);

    return;
}

int main(void) {
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other);
 
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket");
 
    memset(&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    while (1) {
        int guessed_num;
        char buf[BUFLEN];

        printf("Guess a number from 1-6: ");
        scanf("%d", &guessed_num);
         
        // send the guessed number
        if (sendto(s, &guessed_num, sizeof(guessed_num), 0, (struct sockaddr *) &si_other, slen) == -1)
            die("sendto()");

        // receive a reply and print it
        // clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);

        // try to receive some data, this is a blocking call
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
            die("recvfrom()");
         
        printf("%s\n", buf);
    }
 
    close(s);

    return 0;
}
