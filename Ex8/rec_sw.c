#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SIZE 20

struct Frame
{
    int seq;
    char data[SIZE];
    int checksum;
};

int checksum(char data[])
{
    int sum = 0;
    int i;

    for(i = 0; data[i] != '\0'; i++)
        sum = sum + data[i];

    return sum;
}

int main()
{
    int sock;
    struct sockaddr_in receiver, sender;
    struct Frame frame;

    char message[200] = "";
    char ack[20];

    int expected = 0;
    int len;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    bind(sock,
         (struct sockaddr *)&receiver,
         sizeof(receiver));

    printf("Receiver waiting...\n");

    while(1)
    {
        len = sizeof(sender);

        recvfrom(sock,
                 &frame,
                 sizeof(frame),
                 0,
                 (struct sockaddr *)&sender,
                 &len);

        printf("\n-------------------------\n");
        printf("Frame received\n");
        printf("Sequence Number: %d\n", frame.seq);
        printf("Data: %s\n", frame.data);

        if(checksum(frame.data) != frame.checksum)
        {
            printf("Frame has ERROR!\n");
            printf("Frame discarded.\n");

            continue;
        }

        if(frame.seq == expected)
        {
            printf("Correct frame received.\n");

            strcat(message, frame.data);

            sprintf(ack, "%d", 1 - expected);

            sendto(sock,
                   ack,
                   strlen(ack) + 1,
                   0,
                   (struct sockaddr *)&sender,
                   len);

            printf("ACK %s sent.\n", ack);

            expected = 1 - expected;

            printf("Message so far: %s\n", message);
        }
        else
        {

            printf("Duplicate / wrong sequence frame received.\n");
            printf("Frame not added again.\n");

            sprintf(ack, "%d", expected);

            sendto(sock,
                   ack,
                   strlen(ack) + 1,
                   0,
                   (struct sockaddr *)&sender,
                   len);

            printf("ACK %s sent again.\n", ack);
        }
    }

    close(sock);

    return 0;
}