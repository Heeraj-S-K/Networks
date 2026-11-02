#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define F_SZ 8

#define PPP_FLAG     0x7E
#define PPP_ADDRESS  0xFF
#define PPP_CONTROL  0x03
#define PPP_ESC      0x7D

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
    int i;
    int sum = 0;
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
    int i;
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (i = 0; i < 6; i++) printByte(x[i]);
    }
}

unsigned short computeChecksum(unsigned char *data, int length) {
    int i;
    unsigned long sum = 0;
    for (i = 0; i < length - 1; i += 2) {
        sum += (data[i] << 8) | data[i + 1];
    }
    if (length % 2 != 0) {
        sum += (data[length - 1] << 8);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (unsigned short)(~sum);
}

void showLayers(char msg[], int len, struct Node *dest, unsigned char *stream, int *streamLen) {
    int i;
    int idx = 0;
    int a, b, c, d;
    unsigned int x[6];

    for(i = 0; i < len; i++) {
        stream[idx++] = msg[i];
    }
    printf("\nMessage :\n");
    for(i = 0; i < idx; i++) printByte(stream[i]);
    printf("\n\n");

    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");

    stream[idx++] = (srcPort >> 8) & 0xFF;
    stream[idx++] = srcPort & 0xFF;
    stream[idx++] = (dest->port >> 8) & 0xFF;
    stream[idx++] = dest->port & 0xFF;

    printf("Stream           : ");
    for(i = 0; i < idx; i++) printByte(stream[i]);
    printf("\nTotal Bits       : %d bits\n\n", idx * 8);

    printf("========= Network Layer =========\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");

    if (sscanf(srcIP, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        stream[idx++] = a; stream[idx++] = b; stream[idx++] = c; stream[idx++] = d;
    }
    if (sscanf(dest->ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        stream[idx++] = a; stream[idx++] = b; stream[idx++] = c; stream[idx++] = d;
    }

    printf("Stream           : ");
    for(i = 0; i < idx; i++) printByte(stream[i]);
    printf("\nTotal Bits       : %d bits\n\n", idx * 8);

    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");

    if (sscanf(srcMAC, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (i = 0; i < 6; i++) stream[idx++] = x[i];
    }
    if (sscanf(dest->mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (i = 0; i < 6; i++) stream[idx++] = x[i];
    }

    printf("Stream           : ");
    for(i = 0; i < idx; i++) printByte(stream[i]);
    printf("\nTotal Bits       : %d bits\n\n", idx * 8);

    *streamLen = idx;
}

void pppSender(unsigned char dllStream[], int dllStreamLen, const char *outFileName, unsigned char *pppOutStream, int *pppOutLen, unsigned short selectedProtocol) {
    int i,pIdx = 0;

    unsigned char protoHi = (selectedProtocol >> 8) & 0xFF;
    unsigned char protoLo = selectedProtocol & 0xFF;

    printf("========= PPP PROTOCOL (SENDER SIDE fields) =========\n");
    printf("Header Flag      : "); printByte(PPP_FLAG); printf(" (0x%02X)\n", PPP_FLAG);
    printf("Address Field    : "); printByte(PPP_ADDRESS); printf(" (0x%02X)\n", PPP_ADDRESS);
    printf("Control Field    : "); printByte(PPP_CONTROL); printf(" (0x%02X)\n", PPP_CONTROL);
    printf("Protocol Field   : "); printByte(protoHi); printByte(protoLo); printf(" (0x%04X)\n", selectedProtocol);

    pppOutStream[pIdx++] = PPP_FLAG;
    pppOutStream[pIdx++] = PPP_ADDRESS;
    pppOutStream[pIdx++] = PPP_CONTROL;
    pppOutStream[pIdx++] = protoHi;
    pppOutStream[pIdx++] = protoLo;

    printf("Stuffed Payload  : ");
    for (i = 0; i < dllStreamLen; i++) {
        unsigned char currentByte = dllStream[i];
        if (currentByte == PPP_FLAG || currentByte == PPP_ESC) {
            pppOutStream[pIdx++] = PPP_ESC;
            pppOutStream[pIdx++] = currentByte;
            printByte(PPP_ESC); printByte(currentByte);
        } else {
            pppOutStream[pIdx++] = currentByte;
            printByte(currentByte);
        }
    }
    printf("\n");

    unsigned short checksum = computeChecksum(&pppOutStream[1], pIdx - 1);

    printf("Checksum Field   : "); printByte((checksum >> 8) & 0xFF); printByte(checksum & 0xFF); printf(" (0x%04X)\n", checksum);
    printf("Tail Field       : "); printByte(0x00); printf("\n");
    printf("Trailer Flag     : "); printByte(PPP_FLAG); printf(" (0x%02X)\n", PPP_FLAG);

    pppOutStream[pIdx++] = (checksum >> 8) & 0xFF;
    pppOutStream[pIdx++] = checksum & 0xFF;
    pppOutStream[pIdx++] = 0x00;
    pppOutStream[pIdx++] = PPP_FLAG;

    *pppOutLen = pIdx;

    printf("Complete Packet  : ");
    for (i = 0; i < pIdx; i++) printByte(pppOutStream[i]);
    printf("\nTotal Pack Size  : %d bits\n\n", pIdx * 8);

    FILE *fp = fopen(outFileName, "wb");
    if (!fp) {
        printf("Error: Could not save PPP stream to file!\n");
        return;
    }
    fwrite(pppOutStream, sizeof(unsigned char), pIdx, fp);
    fclose(fp);
}

void pppReceiver(const char *inFileName) {
    unsigned char receivedFrame[4000];
    unsigned char recoveredDLLStream[2000];
    int rIdx = 0, dIdx = 0, i;

    printf("========= PPP PROTOCOL (RECEIVER SIDE FIELDS) =========\n");

    FILE *fp = fopen(inFileName, "rb");
    if (!fp) {
        printf("Error: Receiver could not open file '%s'\n", inFileName);
        return;
    }
    while (fread(&receivedFrame[rIdx], sizeof(unsigned char), 1, fp) == 1) {
        rIdx++;
    }
    fclose(fp);

    if (rIdx < 9 || receivedFrame[0] != PPP_FLAG || receivedFrame[rIdx - 1] != PPP_FLAG) {
        printf("Frame Error: Invalid PPP framing structure.\n");
        return;
    }

    unsigned short parsedProtocol = (receivedFrame[3] << 8) | receivedFrame[4];

    printf("Parsed Flag      : "); printByte(receivedFrame[0]); printf("\n");
    printf("Parsed Address   : "); printByte(receivedFrame[1]); printf("\n");
    printf("Parsed Control   : "); printByte(receivedFrame[2]); printf("\n");
    printf("Parsed Protocol  : "); printByte(receivedFrame[3]); printByte(receivedFrame[4]);
    printf(" (0x%04X) -> ", parsedProtocol);

    if (parsedProtocol == 0xC021) printf("LCP Protocol Detected\n");
    else if (parsedProtocol == 0xC023 || parsedProtocol == 0xC223) printf("AP Protocol Detected\n");
    else if (parsedProtocol == 0x8021) printf("NCP Protocol Detected\n");
    else if (parsedProtocol == 0x0021) printf("Data Payload Protocol Detected\n");
    else printf("Unknown Custom Protocol Specified\n");

    unsigned short verification = computeChecksum(&receivedFrame[1], rIdx - 3);

    printf("Parsed Checksum  : "); printByte(receivedFrame[rIdx - 4]); printByte(receivedFrame[rIdx - 3]); printf("\n");
    if (verification == 0) {
        printf("Checksum Check   : PASS (0x0000)\n");
    } else {
        printf("Checksum Check   : FAIL (Error Detected!)\n");
    }

    int payloadEndIdx = rIdx - 4;
    for (i = 5; i < payloadEndIdx; i++) {
        if (receivedFrame[i] == PPP_ESC) {
            i++;
            recoveredDLLStream[dIdx++] = receivedFrame[i];
        } else {
            recoveredDLLStream[dIdx++] = receivedFrame[i];
        }
    }

    printf("Parsed Tail      : "); printByte(receivedFrame[rIdx - 2]); printf("\n");
    printf("Parsed End Flag  : "); printByte(receivedFrame[rIdx - 1]); printf("\n");

    printf("Destuffed Bits   : ");
    for (i = 0; i < dIdx; i++) printByte(recoveredDLLStream[i]);
    printf("\nTotal DLL Bytes  : %d\n", dIdx);

    int messageLen = dIdx - 24;
    if (messageLen > 0) {
        char cleanMsg[1000];
        int mIdx = 0;
        for (mIdx = 0; mIdx < messageLen && mIdx < 999; mIdx++) {
            cleanMsg[mIdx] = (char)recoveredDLLStream[mIdx];
        }
        cleanMsg[mIdx] = '\0';
        printf("\n>>> RECOVERED TEXT MESSAGE AT RECEIVER END: \"%s\" <<<\n", cleanMsg);
    } else {
        printf("\n>>> RECOVERED TEXT MESSAGE AT RECEIVER END: [Empty or Header Error] <<<\n");
    }
    printf("========================================================\n\n");
}

void showFrames(unsigned char stream[], int streamLen, struct Node *dest) {
    int i,j;
    int totalFrames = streamLen / F_SZ;
    if (streamLen % F_SZ != 0) totalFrames++;

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
            if(cur < streamLen) {
                printByte(stream[cur]);
            } else {
                printByte(0);
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
    unsigned short chosenProtocol = 0x0021;

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

    printf("\nAvailable Sources:\n");
    URLTable();
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

    printf("\n========= SELECT PPP PROTOCOL VALUE =========\n");
    printf("1. LCP  (0xC021)\n");
    printf("2. AP   (0xC023)\n");
    printf("3. NCP  (0x8021)\n");
    printf("4. Data (0x0021)\n");
    printf("Enter choice (1-4): ");
    int pChoice;
    scanf("%d", &pChoice);
    if(pChoice == 1) chosenProtocol = 0xC021;
    else if(pChoice == 2) chosenProtocol = 0xC023;
    else if(pChoice == 3) chosenProtocol = 0x8021;
    else chosenProtocol = 0x0021;

    printf("\n--- Packet Overview ---\n");
    printf("Source      : %s (%s)\n", srcURL, srcIP);
    printf("Destination : %s (%s)\n", dest->url, dest->ip);
    printf("Message     : %s\n", msg);

    unsigned char finalStream[2000];
    int streamLen = 0;

    unsigned char pppStream[4000];
    int pppStreamLen = 0;

    showLayers(msg, strlen(msg), dest, finalStream, &streamLen);

    const char *communicationFile = "transmitted_ppp.txt";
    pppSender(finalStream, streamLen, communicationFile, pppStream, &pppStreamLen, chosenProtocol);
    pppReceiver(communicationFile);

    showFrames(pppStream, pppStreamLen, dest);

    return 0;
}
