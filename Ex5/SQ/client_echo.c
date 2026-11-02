#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int sockfd;
    int n;
    char buffer[1024];

    struct sockaddr_in server_addr;

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Connected to server.\n");

    printf("Enter a message: ");

    fgets(buffer, sizeof(buffer), stdin);

    buffer[strcspn(buffer, "\n")] = '\0';

    send(sockfd,buffer,strlen(buffer),0);

    n = recv(sockfd, buffer,sizeof(buffer) - 1,0);

    if (n > 0)
    {
        buffer[n] = '\0';

        printf("Echo from server: %s\n", buffer);
    }

    close(sockfd);

    return 0;
}