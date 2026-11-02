#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_ROWS 10
#define COLS 7

int getParity(int count)
{
    return count % 2;
}

int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    char data[MAX_ROWS][COLS + 1];

    int rrp[MAX_ROWS];
    int rcp[COLS];

    int crp[MAX_ROWS];
    int ccp[COLS];

    int rows;
    int correct = 1;

    // --------------------------------
    // 1. Check command-line argument
    // --------------------------------
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // --------------------------------
    // 2. Create TCP socket
    // --------------------------------
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    // --------------------------------
    // 3. Specify server address
    // --------------------------------
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // --------------------------------
    // 4. Bind
    // --------------------------------
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // --------------------------------
    // 5. Listen
    // --------------------------------
    listen(server_fd, 1);

    printf("TCP 2D Parity Server waiting for client...\n");

    // --------------------------------
    // 6. Accept ONE client
    // --------------------------------
    len = sizeof(client_addr);

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &len);

    if (client_fd < 0)
    {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    // --------------------------------
    // 7. Receive number of rows
    // --------------------------------
    recv(client_fd,
         &rows,
         sizeof(rows),
         0);

    // --------------------------------
    // 8. Receive binary data
    // --------------------------------
    recv(client_fd,
         data,
         sizeof(data),
         0);

    // --------------------------------
    // 9. Receive row parity
    // --------------------------------
    recv(client_fd,
         rrp,
         sizeof(rrp),
         0);

    // --------------------------------
    // 10. Receive column parity
    // --------------------------------
    recv(client_fd,
         rcp,
         sizeof(rcp),
         0);

    // --------------------------------
    // 11. Display received data
    // --------------------------------
    printf("\n========== RECEIVED DATA ==========\n");

    for (int i = 0; i < rows; i++)
    {
        printf("Row %d: ", i + 1);

        for (int j = 0; j < COLS; j++)
        {
            printf("%c ", data[i][j]);
        }

        printf("| Parity = %d\n", rrp[i]);
    }

    printf("-------------------------------\n");

    printf("Column Parity: ");

    for (int j = 0; j < COLS; j++)
    {
        printf("%d ", rcp[j]);
    }

    printf("\n");

    // --------------------------------
    // 12. Calculate Row Parity again
    // --------------------------------
    for (int i = 0; i < rows; i++)
    {
        int count = 0;

        for (int j = 0; j < COLS; j++)
        {
            if (data[i][j] == '1')
            {
                count++;
            }
        }

        crp[i] = getParity(count);
    }

    // --------------------------------
    // 13. Calculate Column Parity again
    // --------------------------------
    for (int j = 0; j < COLS; j++)
    {
        int count = 0;

        for (int i = 0; i < rows; i++)
        {
            if (data[i][j] == '1')
            {
                count++;
            }
        }

        ccp[j] = getParity(count);
    }

    // --------------------------------
    // 14. Display verification
    // --------------------------------
    printf("\n========== PARITY VERIFICATION ==========\n");

    printf("\nRow Parity:\n");

    printf("Received   : ");

    for (int i = 0; i < rows; i++)
    {
        printf("%d ", rrp[i]);
    }

    printf("\nCalculated : ");

    for (int i = 0; i < rows; i++)
    {
        printf("%d ", crp[i]);
    }

    printf("\n");

    // Compare row parity
    for (int i = 0; i < rows; i++)
    {
        if (rrp[i] != crp[i])
        {
            correct = 0;
        }
    }

    printf("\nColumn Parity:\n");

    printf("Received   : ");

    for (int j = 0; j < COLS; j++)
    {
        printf("%d ", rcp[j]);
    }

    printf("\nCalculated : ");

    for (int j = 0; j < COLS; j++)
    {
        printf("%d ", ccp[j]);
    }

    printf("\n");

    // Compare column parity
    for (int j = 0; j < COLS; j++)
    {
        if (rcp[j] != ccp[j])
        {
            correct = 0;
        }
    }

    // --------------------------------
    // 15. Final verification result
    // --------------------------------
    if (correct == 1)
    {
        printf("\nMessage is CORRECT.\n");
    }
    else
    {
        printf("\nMessage is INCORRECT.\n");
    }

    // --------------------------------
    // 16. Close sockets
    // --------------------------------
    close(client_fd);
    close(server_fd);

    return 0;
}