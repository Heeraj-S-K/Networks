=========================================================
                  Hamming Code SENDER
=========================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_BITS 7
#define CODEWORD_SIZE 12

void genHamm(const int data[DATA_BITS], int codeword[CODEWORD_SIZE]) {
    int i, j;
    for (i = 0; i < CODEWORD_SIZE; i++) codeword[i] = 0;

    int dataIdx = 0;
    for (i = 1; i <= 11; i++) {
        if ((i & (i - 1)) != 0) {
            codeword[i] = data[dataIdx++];
        }
    }

    for (i = 0; i < 4; i++) {
        int parityPos = 1 << i;
        int sum = 0;
        for (j = 1; j <= 11; j++) {
            if ((j & parityPos) != 0) {
                sum += codeword[j];
            }
        }
        codeword[parityPos] = sum % 2;
    }
}

void calcParity(int codeword[CODEWORD_SIZE]) {
    int i;
    int totalOnes = 0;
    for (i = 1; i <= 11; i++) {
        if (codeword[i] == 1) totalOnes++;
    }
    codeword[0] = totalOnes % 2;
}

int readInput(const char *fn, int arr[][DATA_BITS], char textStr[]) {
    int b;
    FILE *file = fopen(fn, "r");
    if (!file) {
        file = fopen(fn, "w");
        if (file) {
            fprintf(file, "Hello");
            fclose(file);
            file = fopen(fn, "r");
        } else {
            return 0;
        }
    }

    int ch, charCount = 0;
    while ((ch = fgetc(file)) != EOF && charCount < 100) {
        if (ch == '\r' || ch == '\n') continue;

        textStr[charCount] = (char)ch;

        for (b = 6; b >= 0; b--) {
            arr[charCount][6 - b] = (ch >> b) & 1;
        }
        charCount++;
    }
    textStr[charCount] = '\0';
    fclose(file);
    return charCount;
}

void writeFile(const char *fn, int matrix[][CODEWORD_SIZE], int numChars) {
    int i, j;
    FILE *file = fopen(fn, "w");
    if (!file) return;

    fprintf(file, "%d\n", numChars);
    for (i = 0; i < numChars; i++) {
        for (j = 0; j < CODEWORD_SIZE; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
    printf("[SENDER] Saved Hamming Codewords payload to '%s'.\n", fn);
}

int main() {
    int arr[100][DATA_BITS];
    int matrix[100][CODEWORD_SIZE];
    char textStr[101];
    int i, j;

    int numChars = readInput("input.txt", arr, textStr);
    if (numChars <= 0) {
        printf("Error loading input file.\n");
        return 1;
    }

    printf("Loaded Message: \"%s\"\n", textStr);
    printf("Total Characters: %d\n", numChars);

    printf("Binary Value of Message:\n");
    for (i = 0; i < numChars; i++) {
        printf(" '%c': ", textStr[i]);
        for (j = 0; j < DATA_BITS; j++) {
            printf("%d", arr[i][j]);
        }
        printf("\n");
    }

    for (i = 0; i < numChars; i++) {
        genHamm(arr[i], matrix[i]);
        calcParity(matrix[i]);
    }

    writeFile("transmitted_ham.txt", matrix, numChars);

    return 0;
}


=========================================================
                  Hamming Code RECEIVER
=========================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_BITS 7
#define CODEWORD_SIZE 12

char bitsToChar(const int bits[DATA_BITS]) {
    char ch = 0;
    int i;
    for (i = 0; i < DATA_BITS; i++) {
        ch = (ch << 1) | bits[i];
    }
    return ch;
}

void writeFile(const char *fn, int matrix[][CODEWORD_SIZE], int nc) {
    int i, j;
    FILE *file = fopen(fn, "w");
    if (!file) return;

    fprintf(file, "%d\n", nc);
    for (i = 0; i < nc; i++) {
        for (j = 0; j < CODEWORD_SIZE; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

int readFile(const char *fn, int matrix[][CODEWORD_SIZE]) {
    int i, j;
    FILE *file = fopen(fn, "r");
    if (!file) return 0;

    int nc = 0;
    if (fscanf(file, "%d", &nc) != 1) {
        fclose(file);
        return 0;
    }

    for (i = 0; i < nc; i++) {
        for (j = 0; j < CODEWORD_SIZE; j++) {
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
    fclose(file);
    return nc;
}

void flipBit(const char *fn, int charIndex, int bitPos) {
    int matrix[100][CODEWORD_SIZE];
    int nc = readFile(fn, matrix);

    if (charIndex >= 0 && charIndex < nc && bitPos >= 0 && bitPos < CODEWORD_SIZE) {
        matrix[charIndex][bitPos] ^= 1;
        printf("[NET SIM] Injected bit error into Char #%d, Codeword Bit Index %d.\n", charIndex + 1, bitPos);
        writeFile(fn, matrix, nc);
    } else {
        printf("[NET SIM] Invalid target position!\n");
    }
}

void recvProcess(const char *fn) {
    int r, i, j;
    int matrix[100][CODEWORD_SIZE];
    int nc = readFile(fn, matrix);

    if (nc <= 0) {
        printf("Error reading '%s'. Run the sender first!\n", fn);
        return;
    }

    printf("\n[RECEIVER] Running Extended Hamming Syndrome Verification...\n");

    for (r = 0; r < nc; r++) {
        int *codeword = matrix[r];

        int errorPos = 0;
        for (i = 0; i < 4; i++) {
            int parityPos = 1 << i;
            int sum = 0;
            for (j = 1; j <= 11; j++) {
                if ((j & parityPos) != 0) {
                    sum += codeword[j];
                }
            }
            if (sum % 2 != 0) {
                errorPos += parityPos;
            }
        }

        int totalSum = 0;
        for (i = 0; i < CODEWORD_SIZE; i++) {
            totalSum += codeword[i];
        }
        int overallParityError = (totalSum % 2 != 0);

        if (errorPos != 0 && overallParityError) {
            printf(" -> Syndrome Mismatch on Char #%d: Calculated Error Position = %d\n", r + 1, errorPos);
            printf("[ERROR DETECTED] Single-bit error located at Codeword Bit Index %d.\n", errorPos);
            printf("[CORRECTING] Flipping bit at index [%d] from '%d' to ", errorPos, codeword[errorPos]);
            codeword[errorPos] ^= 1;
            printf("'%d'.\n", codeword[errorPos]);
            printf("[STATUS] Error successfully corrected via Hamming Syndrome!\n");
        } else if (errorPos != 0 && !overallParityError) {
            printf("[ERROR DETECTED] Uncorrectable double-bit error on Char #%d!\n", r + 1);
        }
    }

    char decodedText[101];
    printf("\n[DECODED MESSAGE]: ");
    for (i = 0; i < nc; i++) {
        int bits[DATA_BITS] = {
            matrix[i][3], matrix[i][5], matrix[i][6], matrix[i][7],
            matrix[i][9], matrix[i][10], matrix[i][11]
        };
        decodedText[i] = bitsToChar(bits);
        printf("%c", decodedText[i]);
    }
    decodedText[nc] = '\0';
    printf("\n");

    printf("\n[EXTENDED HAMMING CODEWORDS (12 Bits per Char)]:\n");
    printf(" Index Key: [P0 P1 P2 D1 P4 D2 D3 D4 P8 D5 D6 D7]\n");
    for (i = 0; i < nc; i++) {
        printf(" Char #%d ('%c'): ", i + 1, decodedText[i]);
        for (j = 0; j < CODEWORD_SIZE; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main() {
    char choice;
    int dummyMatrix[100][CODEWORD_SIZE];

    int nc = readFile("transmitted_ham.txt", dummyMatrix);
    if (nc <= 0) {
        printf("Error reading 'transmitted.txt'. Run the sender program first!\n");
        return 1;
    }

    printf("Do you want to inject a single-bit error? (y/n): ");
    if (scanf(" %c", &choice) == 1 && (choice == 'y' || choice == 'Y')) {
        int charIdx, bitIdx;
        printf("Enter Character Number (1 to %d): ", nc);
        scanf("%d", &charIdx);
        printf("Enter Hamming Codeword Bit Index (0 to 11): ");
        scanf("%d", &bitIdx);

        flipBit("transmitted_ham.txt", charIdx - 1, bitIdx);
    } else {
        printf("[NET SIM] Transmitting payload cleanly...\n");
    }

    recvProcess("transmitted_ham.txt");

    return 0;
}
