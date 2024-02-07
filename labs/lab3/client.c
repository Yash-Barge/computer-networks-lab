#include <stdio.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h> // different address structures are declared here
#include <stdlib.h> // atoi() which convert string to integer
#include <string.h>
#include <unistd.h> // close() function

#define BUFFERSIZE 32

#define COND_EXIT(cond, exit_code, err_msg) \
    do { \
        if (cond) { \
            fprintf(stderr, "ERROR: %s\n", (err_msg)); \
            exit(exit_code); \
        } \
    } while (0)

int main(void) {
    /* CREATE A TCP SOCKET*/
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    COND_EXIT(sock < 0, 0, "Error in opening a socket");
    printf("Client Socket Created\n");

    /*CONSTRUCT SERVER ADDRESS STRUCTURE*/
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr)); // reset all bytes in serverAddr
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // You can change port number here
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Specify server's IP address here
    printf("Address assigned\n");

    /*ESTABLISH CONNECTION*/
    int c = connect(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    printf("%d\n",c);
    COND_EXIT(c < 0, 0, "Error while establishing connection");
    printf("Connection Established\n");

    // SEND DATA: Modified to send a float
    printf("ENTER FLOATING POINT NUMBER FOR SERVER: ");
    float msg;
    scanf("%f", &msg);
    COND_EXIT(send(sock, &msg, sizeof(msg), 0) != sizeof(msg), 0, "Error while sending the message");
    printf("Data Sent\n");

    // RECEIVE BYTES: Modified to recieve an integer
    int res;
    COND_EXIT(recv(sock, &res, sizeof(res), 0) < 0, 0, "Error while receiving data from server");
    printf("DATA FROM SERVER: %d\n",res);
    close(sock);

    return 0;
}
