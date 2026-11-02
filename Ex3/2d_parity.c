=========================================================
                  2D_Parity SENDER
=========================================================
  
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define R 128
#define C 8

int getParity(int n) {
    return (n % 2 == 0) ? 0 : 1;
}

char binToChar(char b[C]) {
    char ch = 0; int j;
    for (j = 0; j < C; j++) {
        ch = (ch << 1) | (b[j] - '0');
    }
    return ch;
}

void printBinaryMatrix(char d[R][C], int n) {
    int i,j;
    printf("[BINARY DATA BITS]:\n");
    for (i = 0; i < n; i++) {
        printf(" Row %2d ('%c'): ", i + 1, binToChar(d[i]));
        for (j = 0; j < C; j++) {
            printf("%c ", d[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int readInput(const char *fn, char d[R][C], char t[R]) {
    FILE *f = fopen(fn, "r");
    if (!f) return -1;

    int r = 0, ch, b;
    while ((ch = fgetc(f)) != EOF && r < R) {
        if (ch == '\r' || ch == '\n') continue;

        t[r] = (char)ch;
        for (b = 7; b >= 0; b--) {
            d[r][7 - b] = ((ch >> b) & 1) ? '1' : '0';
        }
        r++;
    }

    fclose(f);
    t[r] = '\0';
    return r;
}

void genParity(char d[R][C], int n, int rp[], int cp[], int *ip) {
    int i, j;
    for (i = 0; i < n; i++) {
        int cnt = 0;
        for (j = 0; j < C; j++) {
            if (d[i][j] == '1') cnt++;
        }
        rp[i] = getParity(cnt);
    }

    for (j = 0; j < C; j++) {
        int cnt = 0;
        for (i = 0; i < n; i++) {
            if (d[i][j] == '1') cnt++;
        }
        cp[j] = getParity(cnt);
    }

    int cnt = 0;
    for (i = 0; i < n; i++) {
        if (rp[i] == 1) cnt++;
    }
    *ip = getParity(cnt);
}

void sendData(const char *fn, char d[R][C], int n, int rp[], int cp[], int ip) {
    int i,j;
    FILE *f = fopen(fn, "w");
    if (!f) return;

    fprintf(f, "%d\n", n);

    for (i = 0; i < n; i++) {
        for (j = 0; j < C; j++) {
            fprintf(f, "%c ", d[i][j]);
        }
        fprintf(f, "%d\n", rp[i]);
    }

    for (j = 0; j < C; j++) {
        fprintf(f, "%d ", cp[j]);
    }
    fprintf(f, "%d\n", ip);

    fclose(f);
    printf("[SENDER] Saved payload to '%s'.\n", fn);
}

int main() {
    char d[R][C];
    char t[R] = {0};
    int rp[R], cp[C], ip = 0;

    int n = readInput("input.txt", d, t);
    if (n <= 0) {
        printf("Error reading 'input.txt'. Make sure the file exists.\n");
        return 1;
    }

    printf("Loaded Message: \"%s\"\n", t);
    printf("Matrix: %d Rows x %d Columns\n", n, C);
    printBinaryMatrix(d, n);

    genParity(d, n, rp, cp, &ip);
    sendData("transmitted_2d.txt", d, n, rp, cp, ip);

    return 0;
}


=========================================================
                  2D_Parity RECEIVER
=========================================================

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define R 128
#define C 8

int getParity(int n) {
    return (n % 2 == 0) ? 0 : 1;
}

char binToChar(char b[C]) {
    char ch = 0; int j;
    for (j = 0; j < C; j++) {
        ch = (ch << 1) | (b[j] - '0');
    }
    return ch;
}

int recvData(const char *fn, char d[R][C], int rp[], int cp[], int *ip) {
    int i, j;
    FILE *f = fopen(fn, "r");
    if (!f) return -1;

    int n = 0;
    if (fscanf(f, "%d", &n) != 1) {
        fclose(f);
        return -1;
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < C; j++) {
            fscanf(f, " %c", &d[i][j]);
        }
        fscanf(f, "%d", &rp[i]);
    }

    for (j = 0; j < C; j++) {
        fscanf(f, "%d ", &cp[j]);
    }

    fscanf(f, "%d", ip);

    fclose(f);
    return n;
}

void sendData(const char *fn, char d[R][C], int n, int rp[], int cp[], int ip) {
    int i, j;
    FILE *f = fopen(fn, "w");
    if (!f) return;

    fprintf(f, "%d\n", n);

    for (i = 0; i < n; i++) {
        for (j = 0; j < C; j++) {
            fprintf(f, "%c ", d[i][j]);
        }
        fprintf(f, "%d\n", rp[i]);
    }

    for (j = 0; j < C; j++) {
        fprintf(f, "%d ", cp[j]);
    }
    fprintf(f, "%d\n", ip);

    fclose(f);
}

void checkAndFix(char d[R][C], int n, int rp[], int cp[], int ip) {
    int er = -1, ec = -1, i, j;

    printf("\n[RECEIVER] Verifying payload...\n");

    for (i = 0; i < n; i++) {
        int cnt = 0;
        for (j = 0; j < C; j++) {
            if (d[i][j] == '1') cnt++;
        }
        int cr = getParity(cnt);
        if (cr != rp[i]) {
            er = i;
            printf(" -> Mismatch at Row %d (Recv Parity: %d, Calc Parity: %d)\n", i + 1, rp[i], cr);
        }
    }

    for (j = 0; j < C; j++) {
        int cnt = 0;
        for (i = 0; i < n; i++) {
            if (d[i][j] == '1') cnt++;
        }
        int cc = getParity(cnt);
        if (cc != cp[j]) {
            ec = j;
            printf(" -> Mismatch at Column %d (Recv Parity: %d, Calc Parity: %d)\n", j + 1, cp[j], cc);
        }
    }

    if (er != -1 && ec != -1) {
        printf("\n[ERROR DETECTED] Located at Row %d, Column %d.\n", er + 1, ec + 1);
        printf("[CORRECTING] Flipping bit at [%d][%d] from '%c' to ", er, ec, d[er][ec]);
        d[er][ec] = (d[er][ec] == '1') ? '0' : '1';
        printf("'%c'.\n", d[er][ec]);
        printf("[STATUS] Error successfully corrected!\n");
    } else if (er == -1 && ec == -1) {
        printf("[STATUS] Transmission verified WITHOUT error.\n");
    } else {
        printf("[STATUS] Unresolvable parity error pattern.\n");
    }

    printf("\n[DECODED MESSAGE]: ");
    for (i = 0; i < n; i++) {
        printf("%c", binToChar(d[i]));
    }
    printf("\n");
}

void flipBit(const char *fn, int tr, int tc) {
    char d[R][C];
    int rp[R], cp[C], ip;

    int n = recvData(fn, d, rp, cp, &ip);
    if (n <= 0) return;

    if (tr >= 0 && tr < n && tc >= 0 && tc < C) {
        d[tr][tc] = (d[tr][tc] == '1') ? '0' : '1';
        printf("[NET SIM] Injected bit error at Row %d, Column %d.\n", tr + 1, tc + 1);
        sendData(fn, d, n, rp, cp, ip);
    } else {
        printf("[NET SIM] Invalid Row or Column position!\n");
    }
}

int main() {
    char d[R][C];
    int rp[R], cp[C], ip = 0;
    char choice;

    int n = recvData("transmitted_2d.txt", d, rp, cp, &ip);
    if (n <= 0) {
        printf("Error reading 'transmitted.txt'. Run the sender first!\n");
        return 1;
    }

    printf("Do you want to inject a single-bit error? (y/n): ");
    if (scanf(" %c", &choice) == 1 && (choice == 'y' || choice == 'Y')) {
        int targetRow, targetCol;

        printf("Enter Row (1 to %d): ", n);
        scanf("%d", &targetRow);
        printf("Enter Column (1 to %d): ", C);
        scanf("%d", &targetCol);

        flipBit("transmitted_2d.txt", targetRow - 1, targetCol - 1);

        n = recvData("transmitted_2d.txt", d, rp, cp, &ip);
    } else {
        printf("[NET SIM] Processing clean data...\n");
    }

    checkAndFix(d, n, rp, cp, ip);

    return 0;
}
