#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/utsname.h>

int main(int argc, char *argv[])
{
    int server_fd, client_fd, choice;
    char username[50], password[50], buffer[1024];
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind
    bind(server_fd, (struct sockaddr *)&server_addr,
         sizeof(server_addr));

    // Listen
    listen(server_fd, 1);

    printf("Server waiting for client...\n");

    // Accept one client
    len = sizeof(client_addr);
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &len);

    printf("Client connected.\n");
    printf("Client IP   : %s\n", inet_ntoa(client_addr.sin_addr));
    printf("Client Port : %d\n", ntohs(client_addr.sin_port));

    // Login
    recv(client_fd, username, sizeof(username), 0);
    recv(client_fd, password, sizeof(password), 0);

    if (strcmp(username, "admin") == 0 &&
        strcmp(password, "1234") == 0)
    {
        strcpy(buffer, "Login successful");
        send(client_fd, buffer, strlen(buffer) + 1, 0);

        printf("Login successful.\n");
    }
    else
    {
        strcpy(buffer, "Login failed");
        send(client_fd, buffer, strlen(buffer) + 1, 0);

        printf("Login failed.\n");

        close(client_fd);
        close(server_fd);
        return 0;
    }

    // Services
    while (1)
    {
        recv(client_fd, &choice, sizeof(choice), 0);

        // 1. Upload
        if (choice == 1)
        {
            FILE *fp;
            int n;

            recv(client_fd, buffer, sizeof(buffer), 0);

            fp = fopen(buffer, "wb");

            if (fp == NULL)
            {
                strcpy(buffer, "Cannot create file");
                send(client_fd, buffer, strlen(buffer) + 1, 0);
                continue;
            }

            strcpy(buffer, "READY");
            send(client_fd, buffer, strlen(buffer) + 1, 0);

            while ((n = recv(client_fd, buffer,
                             sizeof(buffer), 0)) > 0)
            {
                if (n == 4 && strcmp(buffer, "DONE") == 0)
                    break;

                fwrite(buffer, 1, n, fp);

                if (n < sizeof(buffer))
                    break;
            }

            fclose(fp);

            strcpy(buffer, "File uploaded successfully");
            send(client_fd, buffer, strlen(buffer) + 1, 0);
        }

        // 2. Download
        else if (choice == 2)
        {
            FILE *fp;
            int n;

            recv(client_fd, buffer, sizeof(buffer), 0);

            fp = fopen(buffer, "rb");

            if (fp == NULL)
            {
                strcpy(buffer, "File not found");
                send(client_fd, buffer, strlen(buffer) + 1, 0);
                continue;
            }

            strcpy(buffer, "READY");
            send(client_fd, buffer, strlen(buffer) + 1, 0);

            while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
            {
                send(client_fd, buffer, n, 0);
            }

            fclose(fp);

            strcpy(buffer, "DONE");
            send(client_fd, buffer, strlen(buffer) + 1, 0);
        }

        // 3. Date and Time
        else if (choice == 3)
        {
            time_t now;

            time(&now);

            strcpy(buffer, ctime(&now));

            send(client_fd, buffer, strlen(buffer) + 1, 0);
        }

        // 4. System Information
        else if (choice == 4)
        {
            struct utsname info;

            uname(&info);

            sprintf(buffer,
                    "System: %s\n"
                    "Machine: %s\n"
                    "Release: %s",
                    info.sysname,
                    info.machine,
                    info.release);

            send(client_fd, buffer, strlen(buffer) + 1, 0);
        }

        // 5. Exit
        else if (choice == 5)
        {
            strcpy(buffer, "Session terminated");

            send(client_fd, buffer, strlen(buffer) + 1, 0);

            break;
        }

        // Invalid choice
        else
        {
            strcpy(buffer, "Invalid choice");

            send(client_fd, buffer, strlen(buffer) + 1, 0);
        }
    }

    close(client_fd);
    close(server_fd);

    printf("Server closed.\n");

    return 0;
}