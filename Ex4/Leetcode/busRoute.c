#include <stdlib.h>

typedef struct {
    int *data;
    int front;
    int rear;
} Queue;

void push(Queue *q, int value) {
    q->data[q->rear++] = value;
}

int pop(Queue *q) {
    return q->data[q->front++];
}

int empty(Queue *q) {
    return q->front == q->rear;
}

int numBusesToDestination(int** routes, int routesSize, int* routesColSize,
                          int source, int target) {
    if (source == target)
        return 0;

    int maxStop = source;
    if (target > maxStop)
        maxStop = target;

    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            if (routes[i][j] > maxStop)
                maxStop = routes[i][j];
        }
    }

    int **stop = malloc((maxStop + 1) * sizeof(int*));
    int *stopSize = calloc(maxStop + 1, sizeof(int));

    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            stopSize[routes[i][j]]++;
        }
    }

    for (int i = 0; i <= maxStop; i++) {
        if (stopSize[i] > 0)
            stop[i] = malloc(stopSize[i] * sizeof(int));
        else
            stop[i] = NULL;
    }

    for (int i = 0; i <= maxStop; i++)
        stopSize[i] = 0;

    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            int s = routes[i][j];
            stop[s][stopSize[s]] = i;
            stopSize[s]++;
        }
    }

    int *visitedBus = calloc(routesSize, sizeof(int));
    int *visitedStop = calloc(maxStop + 1, sizeof(int));

    Queue q;
    q.data = malloc((maxStop + 1) * sizeof(int));
    q.front = 0;
    q.rear = 0;
    push(&q, source);
    visitedStop[source] = 1;

    int buses = 0;
    while (!empty(&q)) {
        int size = q.rear - q.front;
        buses++;
        while (size--) {
            int current = pop(&q);

            for (int i = 0; i < stopSize[current]; i++) {
                int bus = stop[current][i];
                if (visitedBus[bus])
                    continue;
                visitedBus[bus] = 1;

                for (int j = 0; j < routesColSize[bus]; j++) {
                    int next = routes[bus][j];
                    if (next == target)
                        return buses;
                    if (!visitedStop[next]) {
                        visitedStop[next] = 1;
                        push(&q, next);
                    }
                }
            }
        }
    }
    return -1;
}