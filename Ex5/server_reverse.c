#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    char buffer[1024];
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    listen(server_fd, 1);
    printf("Server waiting for client...\n");

    len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
    if (client_fd < 0)
    {
        perror("accept");
        return 1;
    }

    printf("Client connected.\n");

    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        int i;
        buffer[n] = '\0';
        printf("Original string received: %s\n", buffer);

        int len_str = strlen(buffer);
        for (i = 0; i < len_str / 2; i++)
        {
            char temp = buffer[i];
            buffer[i] = buffer[len_str - i - 1];
            buffer[len_str - i - 1] = temp;
        }

        printf("Reversed string: %s\n", buffer);

        // --- NEW: Send the reversed string back to the client ---
        send(client_fd, buffer, strlen(buffer), 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}