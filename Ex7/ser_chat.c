#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int port;
    socklen_t addr_len;

    /* Validate command line arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /* Set up server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* Bind socket */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Listen */
    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TCP Chat Server listening on port %d...\n", port);
    printf("Waiting for clients...\n\n");

    int client_number = 0;

    while (1)
    {
        addr_len = sizeof(client_addr);

        /* Accept client */
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        if (client_fd < 0)
        {
            perror("accept failed");
            continue;
        }

        client_number++;

        printf("Client %d connected.\n", client_number);

        /* Create child process for client */
        if (fork() == 0)
        {
            close(server_fd);

            char message[BUFFER_SIZE];

            /* Tell client its number */
            sprintf(message,
                    "You are Client %d",
                    client_number);

            send(client_fd,
                 message,
                 strlen(message),
                 0);

            while (1)
            {
                /* Receive message from client */
                int bytes_received = recv(client_fd,
                                          buffer,
                                          BUFFER_SIZE - 1,
                                          0);

                if (bytes_received <= 0)
                {
                    break;
                }

                buffer[bytes_received] = '\0';

                /* Remove newline */
                buffer[strcspn(buffer, "\n")] = '\0';

                /* Client wants to quit */
                if (strcmp(buffer, "quit") == 0 ||
                    strcmp(buffer, "/quit") == 0)
                {
                    break;
                }

                /* Display client message */
                printf("Client %d: %s\n",
                       client_number,
                       buffer);

                printf("You(Server): ");

                /* Server reply */
                if (fgets(message,
                          BUFFER_SIZE,
                          stdin) == NULL)
                {
                    strcpy(message, "quit");
                }

                message[strcspn(message, "\n")] = '\0';

                /* Send reply only to this client */
                char output[BUFFER_SIZE];

                sprintf(output,
                        "You(Server): %s",
                        message);

                send(client_fd,
                     output,
                     strlen(output),
                     0);

                /* Server wants to quit this conversation */
                if (strcmp(message, "quit") == 0 ||
                    strcmp(message, "/quit") == 0)
                {
                    break;
                }
            }

            printf("Client %d has left the chat.\n",
                   client_number);

            close(client_fd);

            exit(0);
        }

        /* Parent closes its copy */
        close(client_fd);

        /* Remove finished child processes */
        waitpid(-1, NULL, WNOHANG);
    }

    close(server_fd);

    return 0;
}