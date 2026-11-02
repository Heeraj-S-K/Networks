#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#define WINDOW 3
#define MAX 20
#define SIZE 20
#define SEQ 8

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

    struct sockaddr_in receiver;
    struct sockaddr_in sender;

    struct Frame frame;
    struct Frame buffer[SEQ];

    char message[MAX * SIZE] = "";

    int received[SEQ] = {0};

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
    printf("Receiver window size = %d\n", WINDOW);

    while(1)
    {
        len = sizeof(sender);

        recvfrom(sock,
                 &frame,
                 sizeof(frame),
                 0,
                 (struct sockaddr *)&sender,
                 &len);

        printf("\n-----------------------------\n");

        printf("Frame received\n");
        printf("Sequence Number: %d\n", frame.seq);
        printf("Data: %s\n", frame.data);

        /*
           Check FRAME ERROR
        */

        if(checksum(frame.data) != frame.checksum)
        {
            char nak[20];

            printf("Frame has ERROR!\n");

            sprintf(nak, "N%d", frame.seq);

            sendto(sock,
                   nak,
                   strlen(nak) + 1,
                   0,
                   (struct sockaddr *)&sender,
                   len);

            printf("NAK sent for Frame %d\n",
                   frame.seq);

            continue;
        }

        /*
           Check if frame is inside
           the receiving window.
        */

        if(frame.seq >= expected &&
           frame.seq < expected + WINDOW)
        {
            /*
               Store frame in receiver buffer.
            */

            if(received[frame.seq] == 0)
            {
                buffer[frame.seq] = frame;
                received[frame.seq] = 1;

                printf("Frame accepted.\n");
                printf("Frame stored in receiver buffer.\n");
            }
            else
            {
                printf("Duplicate frame.\n");
            }

            /*
               Send ACK.
            */

            char ack[20];

            sprintf(ack, "A%d", frame.seq);

            sendto(sock,
                   ack,
                   strlen(ack) + 1,
                   0,
                   (struct sockaddr *)&sender,
                   len);

            printf("ACK sent for Frame %d\n",
                   frame.seq);

            /*
               Deliver frames in order.
            */

            while(received[expected] == 1)
            {
                strcat(message,
                       buffer[expected].data);

                printf("Frame %d delivered to message.\n",
                       expected + 1);

                received[expected] = 0;

                expected++;

                if(expected == SEQ)
                    expected = 0;
            }
        }
        else
        {
            /*
               Duplicate frame from an earlier window.
            */

            if(frame.seq < expected)
            {
                char ack[20];

                sprintf(ack, "A%d", frame.seq);

                sendto(sock,
                       ack,
                       strlen(ack) + 1,
                       0,
                       (struct sockaddr *)&sender,
                       len);

                printf("Duplicate frame.\n");
                printf("ACK sent again for Frame %d\n",
                       frame.seq);
            }
            else
            {
                printf("Frame is outside receiving window.\n");
            }
        }

        printf("Message so far: %s\n",
               message);
    }

    close(sock);

    return 0;
}