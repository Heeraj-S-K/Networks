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
    int sockfd;
    struct sockaddr_in server_addr;

    char data[MAX_ROWS][COLS + 1];
    int rowParity[MAX_ROWS];
    int colParity[COLS];

    int rows;

    // --------------------------------
    // 1. Check command-line arguments
    // --------------------------------
    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    // --------------------------------
    // 2. Create TCP socket
    // --------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // --------------------------------
    // 3. Specify server address
    // --------------------------------
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET,
              argv[1],
              &server_addr.sin_addr);

    // --------------------------------
    // 4. Connect to server
    // --------------------------------
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("Connected to server.\n\n");

    // --------------------------------
    // 5. Get number of rows
    // --------------------------------
    printf("Enter number of 7-bit rows: ");
    scanf("%d", &rows);

    if (rows <= 0 || rows > MAX_ROWS)
    {
        printf("Invalid number of rows.\n");
        close(sockfd);
        return 1;
    }

    // --------------------------------
    // 6. Get 7-bit binary input
    // --------------------------------
    for (int i = 0; i < rows; i++)
    {
        printf("Enter Row %d (7 bits): ", i + 1);
        scanf("%7s", data[i]);

        // Check length
        if (strlen(data[i]) != 7)
        {
            printf("Invalid input. Enter exactly 7 bits.\n");
            close(sockfd);
            return 1;
        }

        // Check whether input contains only 0 and 1
        for (int j = 0; j < COLS; j++)
        {
            if (data[i][j] != '0' && data[i][j] != '1')
            {
                printf("Invalid input. Only 0 and 1 are allowed.\n");
                close(sockfd);
                return 1;
            }
        }
    }

    // --------------------------------
    // 7. Calculate Row Parity
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

        rowParity[i] = getParity(count);
    }

    // --------------------------------
    // 8. Calculate Column Parity
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

        colParity[j] = getParity(count);
    }

    // --------------------------------
    // 9. Display 2D Parity Matrix
    // --------------------------------
    printf("\n========== 2D PARITY MATRIX ==========\n");

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            printf("%c ", data[i][j]);
        }

        printf("| %d\n", rowParity[i]);
    }

    printf("-------------------------------\n");

    for (int j = 0; j < COLS; j++)
    {
        printf("%d ", colParity[j]);
    }

    printf("\n");

    // --------------------------------
    // 10. Send number of rows
    // --------------------------------
    send(sockfd,
         &rows,
         sizeof(rows),
         0);

    // --------------------------------
    // 11. Send binary data
    // --------------------------------
    send(sockfd,
         data,
         sizeof(data),
         0);

    // --------------------------------
    // 12. Send row parity
    // --------------------------------
    send(sockfd,
         rowParity,
         sizeof(rowParity),
         0);

    // --------------------------------
    // 13. Send column parity
    // --------------------------------
    send(sockfd,
         colParity,
         sizeof(colParity),
         0);

    printf("\n2D parity data sent to server.\n");

    // --------------------------------
    // 14. Close socket
    // --------------------------------
    close(sockfd);

    return 0;
}