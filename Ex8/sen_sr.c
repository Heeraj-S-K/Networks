#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdlib.h>

#define MAX 20
#define SIZE 20
#define WINDOW 3
#define SEQ 8

struct Frame
{
    int seq;
    char data[SIZE];
    int checksum;
};

struct Queue
{
    struct Frame frame[MAX];
    int front;
    int rear;
    int count;
};

void initQueue(struct Queue *q)
{
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

void enqueue(struct Queue *q, struct Frame f)
{
    q->rear = (q->rear + 1) % MAX;
    q->frame[q->rear] = f;
    q->count++;
}

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

    struct Queue q;
    struct Frame frame;
    struct Frame temp;

    char word[100];
    char response[20];

    int acked[MAX] = {0};

    int totalFrames = 0;
    int base = 0;
    int next = 0;

    int i;
    int len;
    int choice;
    int specialFrame;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    initQueue(&q);

    printf("Enter word: ");
    scanf("%s", word);

    /*
       Divide word into one-character frames.
    */

    for(i = 0; i < strlen(word); i++)
    {
        frame.seq = totalFrames % SEQ;

        frame.data[0] = word[i];
        frame.data[1] = '\0';

        frame.checksum = checksum(frame.data);

        enqueue(&q, frame);

        totalFrames++;
    }

    printf("\nWord divided into %d frames.\n", totalFrames);
    printf("Window size = %d\n", WINDOW);

    printf("\n1. Normal Transmission\n");
    printf("2. Intentional Timeout\n");
    printf("3. Frame Lost / Error\n");

    printf("Enter choice: ");
    scanf("%d", &choice);

    specialFrame = -1;

    if(choice == 2)
    {
        printf("Enter frame number for timeout: ");
        scanf("%d", &specialFrame);
        specialFrame--;
    }

    if(choice == 3)
    {
        printf("Enter frame number for loss/error: ");
        scanf("%d", &specialFrame);
        specialFrame--;
    }

    /*
       Selective Repeat
    */

    while(base < totalFrames)
    {
        /*
           Send frames until the window is full.
        */

        while(next < totalFrames && next < base + WINDOW)
        {
            frame = q.frame[(q.front + next) % MAX];

            /*
               Intentional Timeout
            */

            if(choice == 2 && next == specialFrame)
            {
                printf("\nFrame %d : %c\n",
                       next + 1, frame.data[0]);

                printf("Intentional Timeout!\n");
                printf("Frame is not sent.\n");

                specialFrame = -1;
            }
            else
            {
                temp = frame;

                /*
                   Frame Error
                   Send corrupted frame once.
                */

                if(choice == 3 && next == specialFrame)
                {
                    printf("\nFrame %d : %c\n",
                           next + 1, frame.data[0]);

                    printf("Frame has ERROR!\n");
                    printf("Sending corrupted frame...\n");

                    temp.data[0] = 'X';

                    specialFrame = -1;
                }

                printf("\nSending Frame %d\n",
                       next + 1);

                printf("Sequence Number: %d\n",
                       temp.seq);

                printf("Data: %s\n",
                       temp.data);

                sendto(sock,
                       &temp,
                       sizeof(temp),
                       0,
                       (struct sockaddr *)&receiver,
                       sizeof(receiver));

                printf("Frame sent.\n");
            }

            next++;
        }

        /*
           Wait for ACK or NAK.
           Timeout = 3 seconds.
        */

        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        int result = select(sock + 1,
                            &readfds,
                            NULL,
                            NULL,
                            &timeout);

        /*
           TIMEOUT
        */

        if(result == 0)
        {
            printf("\nTIMEOUT!\n");

            /*
               Resend only unacknowledged frames.
            */

            for(i = base; i < next; i++)
            {
                if(acked[i] == 0)
                {
                    frame = q.frame[(q.front + i) % MAX];

                    printf("Resending Frame %d\n",
                           i + 1);

                    printf("Sequence Number: %d\n",
                           frame.seq);

                    printf("Data: %s\n",
                           frame.data);

                    sendto(sock,
                           &frame,
                           sizeof(frame),
                           0,
                           (struct sockaddr *)&receiver,
                           sizeof(receiver));
                }
            }
        }
        else
        {
            len = sizeof(receiver);

            recvfrom(sock,
                     response,
                     sizeof(response),
                     0,
                     (struct sockaddr *)&receiver,
                     &len);

            int number = atoi(response + 1);

            /*
               ACK received
            */

            if(response[0] == 'A')
            {
                printf("\nACK received for Frame %d\n",
                       number + 1);

                acked[number] = 1;

                /*
                   Slide the window.
                   If the first frame is ACKed,
                   move the base forward.
                */

                while(base < totalFrames &&
                      acked[base] == 1)
                {
                    base++;
                }

                /*
                   Send the newly available frame.
                */

                if(next < totalFrames &&
                   next < base + WINDOW)
                {
                    frame = q.frame[(q.front + next) % MAX];

                    printf("\nWindow moved.\n");
                    printf("Sending Frame %d\n",
                           next + 1);

                    printf("Sequence Number: %d\n",
                           frame.seq);

                    printf("Data: %s\n",
                           frame.data);

                    sendto(sock,
                           &frame,
                           sizeof(frame),
                           0,
                           (struct sockaddr *)&receiver,
                           sizeof(receiver));

                    printf("Frame sent.\n");

                    next++;
                }
            }

            /*
               NAK received.
               Resend ONLY the errored frame.
            */

            else if(response[0] == 'N')
            {
                printf("\nNAK received for Frame %d\n",
                       number + 1);

                frame = q.frame[(q.front + number) % MAX];

                printf("Frame %d has error.\n",
                       number + 1);

                printf("Resending ONLY Frame %d\n",
                       number + 1);

                printf("Sequence Number: %d\n",
                       frame.seq);

                printf("Data: %s\n",
                       frame.data);

                sendto(sock,
                       &frame,
                       sizeof(frame),
                       0,
                       (struct sockaddr *)&receiver,
                       sizeof(receiver));

                printf("Corrected Frame %d sent again.\n",
                       number + 1);
            }
        }
    }

    printf("\n-----------------------------\n");
    printf("All frames transmitted successfully.\n");

    close(sock);

    return 0;
}