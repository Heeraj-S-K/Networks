#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int sockfd, choice;
    char username[50], password[50];
    char buffer[1024];

    struct sockaddr_in server_addr;

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // Connect
    connect(sockfd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr));

    printf("Connected to server.\n");

    // Login
    printf("\n===== LOGIN =====\n");

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    send(sockfd, username, strlen(username) + 1, 0);
    send(sockfd, password, strlen(password) + 1, 0);

    recv(sockfd, buffer, sizeof(buffer), 0);

    printf("Server: %s\n", buffer);

    if (strcmp(buffer, "Login successful") != 0)
    {
        close(sockfd);
        return 0;
    }

    // Menu
    while (1)
    {
        printf("\n========== MENU ==========\n");
        printf("1. Upload File\n");
        printf("2. Download File\n");
        printf("3. Server Date and Time\n");
        printf("4. Server System Information\n");
        printf("5. Exit\n");
        printf("==========================\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        send(sockfd, &choice, sizeof(choice), 0);

        // Upload
        if (choice == 1)
        {
            FILE *fp;
            int n;

            printf("Enter file name: ");
            scanf("%s", buffer);

            send(sockfd, buffer, strlen(buffer) + 1, 0);

            recv(sockfd, buffer, sizeof(buffer), 0);

            if (strcmp(buffer, "READY") != 0)
            {
                printf("Server: %s\n", buffer);
                continue;
            }

            fp = fopen(buffer, "rb");

            if (fp == NULL)
            {
                printf("File not found.\n");
                continue;
            }

            while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
            {
                send(sockfd, buffer, n, 0);
            }

            fclose(fp);

            send(sockfd, "DONE", 4, 0);

            recv(sockfd, buffer, sizeof(buffer), 0);

            printf("Server: %s\n", buffer);
        }

        // Download
        else if (choice == 2)
        {
            FILE *fp;
            int n;

            printf("Enter file name: ");
            scanf("%s", buffer);

            send(sockfd, buffer, strlen(buffer) + 1, 0);

            recv(sockfd, buffer, sizeof(buffer), 0);

            if (strcmp(buffer, "File not found") == 0)
            {
                printf("Server: File not found.\n");
                continue;
            }

            fp = fopen(buffer, "wb");

            if (fp == NULL)
            {
                printf("Cannot create file.\n");
                continue;
            }

            while (1)
            {
                n = recv(sockfd, buffer, sizeof(buffer), 0);

                if (n <= 0)
                    break;

                if (n == 4 && strcmp(buffer, "DONE") == 0)
                    break;

                fwrite(buffer, 1, n, fp);

                if (n < sizeof(buffer))
                    break;
            }

            fclose(fp);

            printf("File downloaded successfully.\n");
        }

        // Date and Time
        else if (choice == 3)
        {
            recv(sockfd, buffer, sizeof(buffer), 0);

            printf("\nServer Date and Time:\n%s", buffer);
        }

        // System Information
        else if (choice == 4)
        {
            recv(sockfd, buffer, sizeof(buffer), 0);

            printf("\nServer System Information:\n%s\n", buffer);
        }

        // Exit
        else if (choice == 5)
        {
            recv(sockfd, buffer, sizeof(buffer), 0);

            printf("Server: %s\n", buffer);

            break;
        }

        // Invalid
        else
        {
            recv(sockfd, buffer, sizeof(buffer), 0);

            printf("Server: %s\n", buffer);
        }
    }

    close(sockfd);

    return 0;
}