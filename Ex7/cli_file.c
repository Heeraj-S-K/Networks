#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libgen.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[BUF_SIZE];
    char filepath[256];

    struct sockaddr_in server_addr;

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    // Ask the user for the file to send
    printf("Enter path of file to send: ");
    fgets(filepath, sizeof(filepath), stdin);
    filepath[strcspn(filepath, "\n")] = '\0'; // strip newline

    // Open the file to send
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    // ---- Step 1: send the filename, ending with '\n' ----
    char *filename = basename(filepath);
    char header[300];
    snprintf(header, sizeof(header), "%s\n", filename);
    send(sockfd, header, strlen(header), 0);

    // ---- Step 2: send the file content in chunks ----
    int n;
    long total = 0;

    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        send(sockfd, buffer, n, 0);
        total += n;
    }

    fclose(fp);

    printf("Sent file '%s' (%ld bytes) to server\n", filename, total);

    close(sockfd);

    return 0;
}