#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h> // Included for the final question to round up received number
#include <signal.h>

#define MAXPENDING 5
#define BUFFERSIZE 32

#define COND_EXIT(cond, exit_code, err_msg) \
    do { \
        if (cond) { \
            fprintf(stderr, "ERROR: %s\n", (err_msg)); \
            exit(exit_code); \
        } \
    } while (0)

int main(void) {
    // CREATE A TCP SOCKET
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    COND_EXIT(serverSocket < 0, 0, "Error while server socket creation");
    printf("Server Socket Created\n");

    // CONSTRUCT LOCAL ADDRESS STRUCTURE
    struct sockaddr_in serverAddress, clientAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");

    COND_EXIT(bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0, 0, "Error while binding");
    printf("Binding successful\n");

    COND_EXIT(listen(serverSocket, MAXPENDING) < 0, 0, "Error in listen");
    printf("Now Listening\n");

    float client_msg;
    int clientLength = sizeof(clientAddress);

    while (1) {
        printf("Awaiting client...\n");
        int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, (socklen_t *) &clientLength);
        COND_EXIT(clientLength < 0, 0, "Error in client socket");
        printf("Handling Client %s\n", inet_ntoa(clientAddress.sin_addr));

        COND_EXIT(recv(clientSocket, &client_msg, sizeof(client_msg), 0) < 0, 0, "Problem in reading client data");
        printf("RECEIVED FROM CLIENT: %f\n", client_msg);

        int server_msg = (int) ceil(client_msg);
        printf("ROUNDING UP TO: %d\n", server_msg);

        // blocking code to demonstrate multiple connection handling
        printf("Hit ENTER to send: ");
        char temp[20];
        scanf("%c", temp);

        COND_EXIT(send(clientSocket, &server_msg, sizeof(server_msg), 0) != sizeof(server_msg), 0, "Error while sending message to client");

        close(clientSocket);
        printf("Message sent, connection closed\n");
    }

    close(serverSocket);

    return 0;
}
