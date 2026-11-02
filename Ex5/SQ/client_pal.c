#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[1024];
    char result[1024];

    struct sockaddr_in server_addr;

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    // 1. Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // 2. Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // 3. Connect to server
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Connected to server.\n");

    // 4. Get input
    printf("Enter a string: ");
    fgets(buffer, sizeof(buffer), stdin);

    buffer[strcspn(buffer, "\n")] = '\0';

    // 5. Send string to server
    send(sockfd, buffer, strlen(buffer), 0);

    // 6. Receive result from server
    int n = recv(sockfd, result, sizeof(result) - 1, 0);

    if (n > 0)
    {
        result[n] = '\0';

        printf("Result from server: %s\n", result);
    }

    // 7. Close socket
    close(sockfd);

    return 0;
}