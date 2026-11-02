#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

int add(int n1, int n2) { return n1 + n2; }
int sub(int n1, int n2) { return n1 - n2; }
int mul(int n1, int n2) { return n1 * n2; }
int div_op(int n1, int n2) { return (n2 != 0) ? (n1 / n2) : 0; }

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    int s;
    char buff[3];
    char out_payload[256];

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server, client;
    socklen_t len = sizeof(server);
    socklen_t client_len = sizeof(client);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr*)&server, len) < 0) {
        perror("Bind failed");
        exit(1);
    }

    printf("UDP RPC Server running on port %s...\n", argv[1]);
    printf("--------------------------------------------------\n");

    while(1) {
        client_len = sizeof(client); 
        int n = recvfrom(s, buff, sizeof(buff), 0, (struct sockaddr*)&client, &client_len);
        if (n < 0) continue;

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client.sin_port);

        // Prefixes removed from output strings
        printf("Incoming request from %s:%d\n", client_ip, client_port);
        printf("Received Inputs -> Num1: %c, Num2: %c, Op: %c\n", buff[0], buff[1], buff[2]);

        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
        }
        else if (pid == 0) { 
            int n1 = buff[0] - '0';
            int n2 = buff[1] - '0';
            char op = buff[2];
            
            int result = 0;
            int valid_op = 1;

            if (op == 'A') result = add(n1, n2);
            else if (op == 'S') result = sub(n1, n2);
            else if (op == 'M') result = mul(n1, n2);
            else if (op == 'D') {
                if (n2 == 0) valid_op = 1; 
                else result = div_op(n1, n2);
            }
            else valid_op = 0;

            time_t raw_time;
            struct tm *time_info;
            time(&raw_time);
            time_info = localtime(&raw_time);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

            if (valid_op) {
                if (op == '/' && n2 == 0) {
                    snprintf(out_payload, sizeof(out_payload), 
                             "Status: Math Error (Division by 0)\nTimestamp: %s\n", time_str);
                } else {
                    snprintf(out_payload, sizeof(out_payload), 
                             "Result: %d\nStatus: Operation done successfully\nTimestamp: %s\n", 
                             result, time_str);
                }
            } else {
                snprintf(out_payload, sizeof(out_payload), 
                         "Status: Operation not found or invalid\nTimestamp: %s\n", time_str);
            }

            sendto(s, out_payload, strlen(out_payload), 0, (struct sockaddr*)&client, client_len);
            
            printf("Successfully answered %s:%d\n\n",client_ip, client_port);
            exit(0); 
        }
    }
    
    close(s);
    return 0;
}
