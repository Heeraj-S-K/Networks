#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    char buffer[BUF_SIZE];

    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    // Listen
    listen(server_fd, 5);

    printf("File transfer server running on port %s...\n", argv[1]);

    while (1)
    {
        len = sizeof(client_addr);

        // Accept client
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        // Create child process to handle this client
        if (fork() == 0)
        {
            close(server_fd);

            // ---- Step 1: read the filename (ends with '\n') ----
            char filename[256];
            int i = 0;
            char c;

            while (recv(client_fd, &c, 1, 0) > 0 && c != '\n' && i < 255)
                filename[i++] = c;
            filename[i] = '\0';

            char save_path[300];
            snprintf(save_path, sizeof(save_path), "received_%s", filename);

            FILE *fp = fopen(save_path, "wb");
            if (fp == NULL)
            {
                perror("fopen");
                close(client_fd);
                exit(1);
            }

            // ---- Step 2: receive the file content until client closes ----
            int n;
            long total = 0;

            while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
            {
                fwrite(buffer, 1, n, fp);
                total += n;
            }

            fclose(fp);

            printf("Received file '%s' (%ld bytes) -> saved as '%s'\n",
                   filename, total, save_path);

            close(client_fd);
            exit(0);
        }

        // Parent doesn't need the connected socket
        close(client_fd);
    }

    close(server_fd);

    return 0;
}