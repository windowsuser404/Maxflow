#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <omp.h>
#undef DEBUG
//#define DEBUG
#ifdef DEBUG
int check_counter=0;
#endif
int numNodes;
// Structure to represent an edge in the graph
typedef struct Edge {
    int v;              // The other endpoint of the edge
    int capacity;       // Capacity of the edge
    int flow;           // Current flow through the edge
    //struct Edge* next;  // Pointer to the next edge
} Edge;

// Structure to represent a node in the graph
typedef struct Node {
    Edge** edges;     // array of all edges from this node
    int size; //number of its neightbours
    int excessFlow; // Excess flow at the node
    int height;     // Height of the node
} Node;

// Function to create a new node
Node* createNode() {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->edges = NULL;
    newNode->size = 0;
    newNode->excessFlow = 0;
    newNode->height = 0;
    return newNode;
}

// Function to create a new edge
Edge* createEdge(int v, int capacity) {
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->v = v;
    newEdge->capacity = capacity;
    newEdge->flow = 0;
    return newEdge;
}

// Function to add an edge to the graph
void addEdge(Node* nodes[], int u, int v, int capacity) {
    // Create a new edge from u to v
   // Edge* newEdge = createEdge(v, capacity);
   // newEdge->next = nodes[u]->head;
   // nodes[u]->head = newEdge;
   if(nodes[u]->edges==NULL){
	   nodes[u]->edges = (Edge**)malloc(sizeof(Edge*));
   }
   nodes[u]->edges = realloc(nodes[u]->edges, (nodes[u]->size+1)*sizeof(Edge));
   nodes[u]->edges[nodes[u]->size] = createEdge(v, capacity);
   nodes[u]->size++;
   if(nodes[v]->edges==NULL){
	   nodes[v]->edges = (Edge**)malloc(sizeof(Edge*));
   }
   nodes[v]->edges = realloc(nodes[v]->edges, (nodes[v]->size+1)*sizeof(Edge));
   nodes[v]->edges[nodes[v]->size] = createEdge(u, capacity);
   nodes[v]->size++;

    // Create a new edge from v to u (for undirected graph)
 //   newEdge = createEdge(u, capacity);
 //   newEdge->next = nodes[v]->head;
 //   nodes[v]->head = newEdge;
}

// Function to perform breadth-first search (BFS) to find an augmenting path
bool bfs(Node* nodes[], int source, int sink, int parent[], int numNodes) {
#ifdef DEBUG
	printf("\n Visited %d times\n",++check_counter);
#endif
    bool visited[numNodes];
    for (int i = 0; i < numNodes; i++) {
        visited[i] = false;
    }

    visited[source] = true;
    parent[source] = -1;

    // Create a queue for BFS
    int queue[numNodes];
    int front = 0, rear = 0;
    queue[rear++] = source;
//can try tasks and put nowait cos u dont need to wait
    while (front < rear) {
        int u = queue[front++];
	Node* node = nodes[u];
	bool found=false;
//        Edge* edge = nodes[u]->head;
        for(int i=0;i<node->size;i++) {
            int v = node->edges[i]->v;
#ifdef DEBUG
	    printf("%d to %d has capacity:%d and flow:%d\n",u,v,edge->capacity,edge->flow);
#endif
	    if (!visited[v] && node->edges[i]->capacity > 0) {
                visited[v] = true;
                parent[v] = u;
                queue[rear++] = v;

                if (v == sink) {
#ifdef DEBUG
			printf("Found a augument in %d time\n",check_counter);
			for(int temp=0;temp<numNodes;temp++)
			{
				printf("%d ",parent[temp]);
			}
#endif
                    found=true;
		}
            }
	}
	if(found){
		return true;
	}

    }

    return false;
}

// Function to find the maximum flow in an undirected graph using Edmonds-Karp algorithm
int maxFlowEdmondsKarp(Node* nodes[], int source, int sink, int numNodes) {
    // Initialize the excess flow and height of all nodes
    for (int i = 0; i < numNodes; i++) {
        nodes[i]->excessFlow = 0;
        nodes[i]->height = 0;
    }

    // Initialize the flow to 0
    for (int u = 0; u < numNodes; u++) {
	Node* node = nodes[u];
	for(int i=0;i<node->size;i++){
		node->edges[i]->flow = 0;
	}
    }

    // Preprocess: Push as much flow as possible from the source
 /*   Edge* edge = nodes[source]->head;
    while (edge != NULL) {
        int v = edge->v;
        int capacity = edge->capacity;

        edge->flow = capacity;
        edge->capacity = 0;

        nodes[source]->excessFlow -= capacity;
        nodes[v]->excessFlow += capacity;

        edge = edge->next;
    }*/

    // Find the maximum flow using Edmonds-Karp algorithm
    while (true) {
        int parent[numNodes];
        if (!bfs(nodes, source, sink, parent, numNodes)) {
            break;
        }
	
        // Find the bottleneck capacity (minimum residual capacity) along the augmenting path
        int bottleneckCapacity = INT_MAX;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
	Node* node = nodes[u];
	    for(int i=0;i<node->size;i++){
		    if(node->edges[i]->v==v){
#ifdef DEBUG
		    printf("bottle neck was %d and residual between %d and %d is %d\n",bottleneckCapacity,u,v,residualCapacity);
#endif
			    if(node->edges[i]->capacity < bottleneckCapacity){
				    bottleneckCapacity = node->edges[i]->capacity;
			    }
			    break;
		    }
	    }

        }
#ifdef DEBUG
	printf("Bottle neck capa found:%d\n",bottleneckCapacity);
#endif
        // Update the flow and capacities along the augmenting path
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            Node* node = nodes[u];
            for(int i=0;i<node->size;i++) {
                if (node->edges[i]->v == v) {
                   node-> edges[i]->flow += bottleneckCapacity;
                    node->edges[i]->capacity -= bottleneckCapacity;
                }
            }
        }
    }
    // Calculate and return the maximum flow at the sink node
    int maxFlow = 0;
    Node *node = nodes[source];
#ifdef DEBUG
     Edge* temporary_edge;
for(int lmao=0;lmao<numNodes;lmao++){
	temporary_edge = nodes[lmao]->head;
	while(temporary_edge!=NULL){
		printf("%d has flow into %d :%d and remaining capapcity:%d \n",lmao,temporary_edge->v,temporary_edge->flow,temporary_edge->capacity);
		temporary_edge = temporary_edge->next;
	}
}
#endif
    for(int i=0;i<node->size;i++) {
        maxFlow += node->edges[i]->flow;
    }

    return maxFlow;
}

int main(int argc, char* argv[]) {
	FILE* file = fopen(argv[1],"r");
     numNodes = atoi(argv[2])+1;
   int source = atoi(argv[3]);
   int sink = atoi(argv[4]);
    if(file==NULL){
	    printf("File handling error");
	    exit(0);
    }

    Node* nodes[numNodes];
    for (int i = 0; i < numNodes; i++) {
        nodes[i] = createNode();
    }
    int src,dest,cap=1;
while(fscanf(file, "%d %d", &src, &dest) == 2){
	addEdge(nodes, src, dest, cap);

//	printf("Added %d and %d with capacity %d\n", src, dest, cap);
}
float t1,t2;
t1=omp_get_wtime();
    int maxFlow = maxFlowEdmondsKarp(nodes, source, sink, numNodes);
t2=omp_get_wtime();
    printf("Maximum Flow: %d\n", maxFlow);
    printf("Time in Serial is:%f\n",t2-t1);
    return 0;
}

