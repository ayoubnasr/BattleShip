#ifndef BINOMHEAP


#define BINOMHEAP

typedef struct Node{

    void * value;
    
    int degree;

    struct Node * leftChild;
    struct Node * rightSibling;
    struct Node * parent;

} Node;

typedef struct {

    Node * head;

    int (*compare)(void*, void*);


} BinomialHeap;

BinomialHeap * ConstructBinomialHeap(void ** array, int arrayLen, int (*compare)(void*, void*));

int insert(BinomialHeap * binomHeap, void * data);

Node * LinkNodes(Node* node1, Node* node2, Node* leftSibling, Node* rightSibling, int (*compare)(void*, void*));
Node * RecurseLinkToRightSibling(Node * currHead, int(*compare)(void*, void*));

int Union(BinomialHeap* heap1, BinomialHeap* heap2);

void* findMin(BinomialHeap * heap);

void* deleteMin(BinomialHeap * heap);


void InOrderTraversalTree_INT(Node * root);

int compareInt(void* v1, void* v2);




#endif