//Framing a message and displaying the intermediate layers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define F_SZ 8

struct Node {
    char url[50], ip[20], mac[20];
    int port;
    struct Node *next;
};

struct Node *table[SIZE] = {NULL};

char srcURL[50] = "Default Source";
char srcIP[20] = "192.168.1.10";
char srcMAC[20] = "11:22:33:44:55:66";
int srcPort = 51309;

int hash(char url[]) {
    int i,sum = 0;
    for(i = 0; url[i]; i++) sum += url[i];
    return sum % SIZE;
}

void insert(char url[], char ip[], char mac[], int port) {
    int idx = hash(url);
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(newNode->url, url);
    strcpy(newNode->ip, ip);
    strcpy(newNode->mac, mac);
    newNode->port = port;
    newNode->next = table[idx];
    table[idx] = newNode;
}

struct Node* search(char url[]) {
    struct Node *tmp = table[hash(url)];
    while(tmp) {
        if(strcmp(tmp->url, url) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

void delete(char url[]) {
    int idx = hash(url);
    struct Node *tmp = table[idx];
    struct Node *prev = NULL;

    while (tmp != NULL && strcmp(tmp->url, url) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL) {
        printf("URL '%s' not found in the table.\n", url);
        return;
    }

    if (prev == NULL) {
        table[idx] = tmp->next;
    } else {
        prev->next = tmp->next;
    }

    free(tmp);
    printf("Successfully deleted '%s' from the table.\n", url);
}

void URLTable() {
    int i;
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-19s %-5s\n", "URL", "IP", "MAC", "PORT");
    for(i = 0; i < SIZE; i++) {
        struct Node *tmp = table[i];
        while(tmp) {
            printf("%-20s %-18s %-19s %-5d\n", tmp->url, tmp->ip, tmp->mac, tmp->port);
            tmp = tmp->next;
        }
    }
    printf("===========================================\n");
}

void preload() {
    insert("www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25);
    insert("www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443);
    insert("www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 80);
    insert("www.google.com", "142.250.190.47", "AA:BB:CC:DD:EE:04", 443);
}

void printByte(unsigned char n) {
    int i;
    for (i = 7; i >= 0; i--) printf("%d", (n >> i) & 1);
}

void printPort(int port) {
    int i;
    for (i = 15; i >= 0; i--) printf("%d", (port >> i) & 1);
}

void printIP(char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByte(a); printByte(b); printByte(c); printByte(d);
    }
}

void printMAC(char mac[]) {
    unsigned int x[6];
    int i;
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (i = 0; i < 6; i++) printByte(x[i]);
    }
}

void showLayers(char msg[], int len, struct Node *dest) {
    int i,j,k,l;
    printf("\nMessage :\n");
    for(i = 0; i < len; i++) printByte(msg[i]);
    printf("\n\n");

    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");
    printf("Stream           : ");
    for(j = 0; j < len; j++) printByte(msg[j]);
    printPort(srcPort); printPort(dest->port);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32);

    printf("========= Network Layer =========\n\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");
    printf("Stream           : ");
    for(k = 0; k < len; k++) printByte(msg[k]);
    printPort(srcPort); printPort(dest->port);
    printIP(srcIP); printIP(dest->ip);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64);

    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
    printf("Stream           : ");
    for(l = 0; l < len; l++) printByte(msg[l]);
    printPort(srcPort); printPort(dest->port);
    printIP(srcIP); printIP(dest->ip);
    printMAC(srcMAC); printMAC(dest->mac);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64 + 96);
}

void showFrames(char msg[], int len, int totalFrames, struct Node *dest) {
    int i,j;
    printf("====Frame Contents======\n");
    for(i = 0; i < totalFrames; i++) {
        printf("\n-----------------------------------------\n");
        int framesPerPacket = 2;
        int packetNo = (i / framesPerPacket) + 1;
        printf("Packet No : %d\n", packetNo);
        printf("Frame No  : %d\n", i + 1);
        printf("Source Port      : "); printPort(srcPort); printf("\n");
        printf("Destination Port : "); printPort(dest->port); printf("\n\n");
        printf("Source IP      : "); printIP(srcIP); printf("\n");
        printf("Destination IP : "); printIP(dest->ip); printf("\n");
        printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
        printf("Destination MAC: "); printMAC(dest->mac); printf("\n");

        printf("Frame Data     : ");
        for(j = 0; j < F_SZ; j++) {
            int cur = (i * F_SZ) + j;
            if(cur < len) {
                printByte(msg[cur]);
            } else {
                printByte(0); // Framing trailing zeros inside individual chunks
            }
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}

int main() {
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0;
    int m, cho;

    preload();

    while(1) {
        URLTable();
        printf("\n========= MAIN MENU =========\n");
        printf("1. Hash Table Management (Add/Delete/View)\n");
        printf("2. Proceed to Data Framing/Layers Process\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &m) != 1) return 0;

        if (m == 1) {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back to Main Menu\n");
            printf("Enter choice: ");
            scanf("%d", &cho);

            if (cho == 1) {
                char newUrl[50], newIp[20], newMac[20];
                int newPort;
                printf("Enter URL: "); scanf("%49s", newUrl);
                printf("Enter IP: "); scanf("%19s", newIp);
                printf("Enter MAC: "); scanf("%19s", newMac);
                printf("Enter Port: "); scanf("%d", &newPort);
                insert(newUrl, newIp, newMac, newPort);
                printf("Successfully Added!\n");
            }
            else if (cho == 2) {
                char delUrl[50];
                printf("Enter URL to delete: "); scanf("%49s", delUrl);
                delete(delUrl);
            }
        }
        else if (m == 2) {
            break;
        }
        else {
            printf("Exiting program...\n");
            return 0;
        }
    }

   /* printf("\nAvailable Sources:\n");
    URLTable();*/
    printf("\nEnter Source URL from table: ");
    scanf("%99s", url);
    struct Node *srcNode = search(url);
    if(!srcNode) {
        printf("Source URL Not Found! Reverting to default fallback configuration.\n");
    } else {
        strcpy(srcURL, srcNode->url);
        strcpy(srcIP, srcNode->ip);
        strcpy(srcMAC, srcNode->mac);
        srcPort = srcNode->port;
    }

    printf("Enter Destination URL: ");
    scanf("%99s", url);
    struct Node *dest = search(url);
    if(!dest) {
        printf("Destination URL Not Found!\n");
        return 0;
    }

    printf("Enter File Name to read message from: ");
    scanf("%49s", fn);

    fp = fopen(fn, "r");
    if(!fp) {
        printf("Unable to Open File!\n");
        return 0;
    }

    while((ch = fgetc(fp)) != EOF && idx < 999) {
        msg[idx++] = (char)ch;
    }
    msg[idx] = '\0';
    fclose(fp);

    int len = strlen(msg);
    int totalFrames = len / F_SZ;
    if (len % F_SZ != 0) totalFrames++;

    printf("\n--- Packet Overview ---\n");
    printf("Source      : %s (%s)\n", srcURL, srcIP);
    printf("Destination : %s (%s)\n", dest->url, dest->ip);
    printf("Message     : %s\n", msg);

    showLayers(msg, len, dest);
    showFrames(msg, len, totalFrames, dest);

    return 0;
}
