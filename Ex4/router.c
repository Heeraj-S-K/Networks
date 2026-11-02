#include <stdio.h>
#include <string.h>

#define INF 999999
#define MAX 100
#define MSG_SIZE 1000


typedef struct {
    int id;
    char data[50];
} Node;

typedef struct {
    int from;
    int to;
    int weight;
} Edge;

int V, E;
Node nodes[MAX];
Edge edges[MAX];
int dist[MAX], parent[MAX];
int s;
int d;
int hasNegativeCycle;
char message[MSG_SIZE];

void initialize();
void relaxEdges();
void checkNegativeCycle();
void printResult();
int  buildPath(int path[]);
void routeMessage();

int main() {
    int i;
    printf("Enter number of nodes: ");
    scanf("%d", &V);

    printf("Enter data for each node:\n");
    for (i = 0; i < V; i++) {
        nodes[i].id = i;
        printf("Node %d data (e.g. router name): ", i);
        scanf("%s", nodes[i].data);
    }

    printf("Enter number of edges: ");
    scanf("%d", &E);

    printf("Enter each edge as: fromNode toNode weight\n");
    printf("(weight can be negative, e.g. 1 4 -4 means Node1--(-4)-->Node4)\n");
    for (i = 0; i < E; i++) {
        printf("Edge %d: ", i + 1);
        scanf("%d %d %d", &edges[i].from, &edges[i].to, &edges[i].weight);
    }

    printf("Enter source node: ");
    scanf("%d", &s);

    printf("Enter destination node: ");
    scanf("%d", &d);

    char filename[100];
    printf("Enter message file name: ");
    scanf("%s", filename);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open file: %s\n", filename);
        return 1;
    }
    fgets(message, MSG_SIZE, fp);
    fclose(fp);

    int len = strlen(message);
    if (len > 0 && message[len - 1] == '\n') {
        message[len - 1] = '\0';
    }

    initialize();
    relaxEdges();
    checkNegativeCycle();
    printResult();
    routeMessage();

    return 0;
}

void initialize() {
    int v;
    for (v = 0; v < V; v++) {
        dist[v] = INF;
        parent[v] = -1;   // -1 represents nil
    }
    dist[s] = 0;
    parent[s] = s;
}

void relaxEdges() {
    int i, j, k;
    for (i =1; i<=V-1; i++) {

        printf("\n================================================");
        printf("\nIteration %d", i);
        printf("\n================================================\n");

        int updated = 0;

        for (j=0; j<E; j++) {

            int u = edges[j].from;
            int v = edges[j].to;
            int w = edges[j].weight;

            printf("\nChecking Edge %d -> %d (weight = %d)\n", u, v, w);

            if(dist[u] == INF){
                printf("Source node %d is unreachable. Skip.\n", u);
                continue;
            }

            printf("Current Distance[%d] = ", v);

            if(dist[v] == INF)
                printf("INF\n");
            else
                printf("%d\n", dist[v]);

            printf("New Possible Distance = %d + (%d) = %d\n",
                    dist[u], w, dist[u]+w);

            if(dist[u]+w < dist[v]){

                printf("Updated! %d --> %d\n",
                        dist[v]==INF?-1:dist[v],
                        dist[u]+w);

                dist[v]=dist[u]+w;
                parent[v]=u;
            }
            else{

                printf("No Update\n");
            }
        }

        printf("\nDistance Table After Iteration %d\n",i);
        printf("--------------------------------------------\n");

        printf("Node\tDistance\tParent\n");

        for(k=0;k<V;k++){

            printf("%d(%s)\t",k,nodes[k].data);

            if(dist[k]==INF)
                printf("INF\t\t");
            else
                printf("%d\t\t",dist[k]);

            if(parent[k]==-1)
                printf("NIL");
            else
                printf("%d(%s)",parent[k],nodes[parent[k]].data);

            printf("\n");
        }
    }
}

void checkNegativeCycle() {
    int j;
    hasNegativeCycle = 0;
    for (j = 0; j < E; j++) {
        int u = edges[j].from;
        int v = edges[j].to;
        int w = edges[j].weight;

        if (dist[u] != INF && dist[u] + w < dist[v]) {
            hasNegativeCycle = 1;
            break;
        }
    }
}

void printResult() {
    int v;
    if (hasNegativeCycle) {
        printf("\nNegative weight cycle exists! Shortest paths are undefined.\n");
    } else {
        printf("\nShortest distances from source node %d (%s):\n", s, nodes[s].data);
        for (v = 0; v < V; v++) {
            if (dist[v] == INF)
                printf("Node %d (%s): unreachable\n", v, nodes[v].data);
            else
                printf("Node %d (%s): distance = %d, parent = %d (%s)\n",
                       v, nodes[v].data, dist[v], parent[v], nodes[parent[v]].data);
        }
    }
}

int buildPath(int path[]) {
    int i;
    if (dist[d] == INF) return 0;

    int temp[MAX], len = 0;
    int v = d;
    while (v != s) {
        temp[len++] = v;
        v = parent[v];
    }
    temp[len++] = s;

    for (i = 0; i < len; i++) {
        path[i] = temp[len - 1 - i];
    }
    return len;
}

void routeMessage() {
    int i;
    if (hasNegativeCycle) {
        printf("\nCannot route message: negative weight cycle in network.\n");
        return;
    }

    int path[MAX];
    int len = buildPath(path);

    if (len == 0) {
        printf("\nDestination %d is unreachable from source %d. Message dropped.\n", d, s);
        return;
    }

    printf("\nRouting message: \"%s\"\n", message);
    printf("Path (total cost = %d):\n", dist[d]);
    for (i = 0; i < len; i++) {
        int nid = path[i];
        if (i == 0)
            printf("Node %d (%s) [source]\n", nid, nodes[nid].data);
        else if (i == len - 1)
            printf("  --> Node %d (%s) [destination]\n", nid, nodes[nid].data);
        else
            printf("  --> Node %d (%s)\n", nid, nodes[nid].data);
    }
    printf("\nMessage \"%s\" successfully delivered to Node %d (%s)\n",
           message, d, nodes[d].data);
}
