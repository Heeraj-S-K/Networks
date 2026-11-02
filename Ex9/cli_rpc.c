#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <Server_IP> <Port>\n", argv[0]);
        exit(1);
    }

    int s;
    char buff[3];
    char res_buff[256];

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    printf("Enter the first number: ");
    scanf(" %c", &buff[0]);
    
    printf("Enter the second number: ");
    scanf(" %c", &buff[1]);

    printf("Enter the operation as ADD, SUB, MUL, DIV: ");
    scanf(" %s", &buff[2]);

    sendto(s, buff, sizeof(buff), 0, (struct sockaddr*)&server, len);

    int n = recvfrom(s, res_buff, sizeof(res_buff) - 1, 0, (struct sockaddr*)&server, &len);
    if (n > 0) {
        res_buff[n] = '\0'; // Ensure string termination
        printf("\n[Server Response]:\n%s", res_buff);
    }

    close(s);
    return 0;
}
