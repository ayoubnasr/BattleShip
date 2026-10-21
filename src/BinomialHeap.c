#include <stdio.h>
#include <stdlib.h>
#include "../include/BinomialHeap.h"


/**
 * This function constructs a binomial heap out of an array of elements.
 * 
 * Input:
 *      - Array of void pointers. This data structure works for any type of object, but requires the client to input a pointer to a compare function
 *        that compares between 2 objects of the type being used.
 * 
 * Output:
 *      - A binomial heap built from the elements inputted via the array.
 *      - returns NULL if pointer to compare function is NULL
 * 
 * Time complexity: O(nlogn)
 */
BinomialHeap * ConstructBinomialHeap(void ** array, int arrayLen, int (*compare)(void*, void*)){

    if (compare == NULL) return NULL;

    BinomialHeap * heap = (BinomialHeap *)(malloc(sizeof(BinomialHeap)));

    heap->head = NULL;

    heap->compare = compare;
    
    if (array == NULL) return heap;

    for (int i = 0; i < arrayLen; i++)
    {
        //printf("inserting %d\n", *(int*)(array[i]));
        insert(heap, array[i]);
    }

    return heap;

}

/**
 * This function inserts a new element into the specified binomial heap.
 * 
 * Input:
 *      - binomHeap: The binomial heap you want to insert into
 *      - data: the data value you want to insert
 * 
 * 
 * Execution Time:
 *      - This operation runs in O(logn) because in the worst case it would have logn binomial trees and must unite them all.
 * 
 */
int insert(BinomialHeap * binomHeap, void * data){

    if (binomHeap->head == NULL){
        binomHeap->head = (Node *)(malloc(sizeof(Node)));
        binomHeap->head->degree = 0;
        binomHeap->head->leftChild = NULL;
        binomHeap->head->parent = NULL;
        binomHeap->head->rightSibling = NULL;
        binomHeap->head->value = data;
    }
    else {

        Node * newNode = (Node *)(malloc(sizeof(Node)));
        newNode->parent = NULL;
        newNode->leftChild = NULL;
        newNode->rightSibling = binomHeap->head;
        newNode->value = data;
        newNode->degree = 0;

        Node * currHead = newNode;
        binomHeap->head = currHead;

        binomHeap->head = RecurseLinkToRightSibling(currHead, binomHeap->compare);


    }

    return 1;
}


/**
 * This function links two binomial trees represented as nodes node1 and node2. It links them in a way such that the resultant tree satisfies
 * the heap property. It compares the values of the two nodes and links accordingly.
 * This is an independent function that cannot know to what larger trees the two nodes might belong, it assumes that no matter what, you want the resultant
 * tree to be a sibling in between the leftSibling and rightSibling arguments inputted.
 * 
 * THIS FUNCTION ASSUMES THAT THE TWO NODES ARE ROOTS OF BINOMIAL TREES OF SAME HEIGHT.
 * 
 * Input:
 *      - node1: the root of the first binomial heap
 *      - node2: the root of the second binomial heap
 *      - leftSibling: this represents the root of the binomial tree of whom the resultant linked tree will be the right sibling
 *      - rightSibling: this represents the tree that will be the rightSibling of the resultant tree
 * 
 * So if the two nodes node1 and node2 come from different binomial heaps, we can specify which heap we want to link them into using the leftSibling and
 * rightSibling nodes.
 * 
 * Note: At first I thought that I needed to pass pointers to pointers when it came to nodes so that when changing the addresses they point to we actually change the addresses of the original
 * pointers and not the argument copies. However, unless I had to make the argument node itself point to something else, I don't need to do that. Node pointers
 * point to structure blocks in memory, and when you use -> to access the inner pointers you are accessing the original ones. The arguments passed are
 * copies of the Node pointers, but they point to the same structure in memory.
 * In fact, using Node** rather than Node* created a ton of problems. When passing a pointer to a pointer, and using derferencing, you are essentially
 * accessing the original pointer. And when the function changes that pointer, merely because it has to sometimes if that original pointer was accessed and
 * modified via some other argument, then you'd be expecting the initial value of the pointer, but it changed already. So what you need in this case
 * is the argument being a copy pointer to that original address in memory.
 * 
 */
Node * LinkNodes(Node* node1, Node* node2, Node* leftSibling, Node* rightSibling, int (*compare)(void*, void*)){

    if (compare((node1)->value, (node2)->value) > 0){
        //then node 2 must be the root:
        (node1)->parent = node2;
        (node1)->rightSibling = (node2)->leftChild;
        (node2)->leftChild = node1;
        if (leftSibling != NULL){
            (leftSibling)->rightSibling = node2;
        }
        if (rightSibling != NULL){
            (node2)->rightSibling = rightSibling;
        }
        else node2->rightSibling = NULL;
        (node2)->degree++;
        return node2;
    }
    else {
        (node2)->parent = node1;
        (node2)->rightSibling = (node1)->leftChild;
        (node1)->leftChild = node2;
        if (leftSibling != NULL){
            (leftSibling)->rightSibling = node1;
        }
        if (rightSibling != NULL){
            (node1)->rightSibling = rightSibling;
        }
        else node1->rightSibling = NULL;
        (node1)->degree++;
        return node1;
    }
}


Node * RecurseLinkToRightSibling(Node * currHead, int(*compare)(void*, void*)){
    while (currHead->rightSibling != NULL && currHead->rightSibling->degree == currHead->degree)
    {
        //printf("currheadRS = %d\n", currHead->rightSibling);
        //printf("currDegree = %d, rightDegree = %d\n", currHead->degree, currHead->rightSibling->degree);
        //printf("Two heads %d and %d with same degree %d\n", *(int*)currHead->value, *(int*)currHead->rightSibling->value, currHead->degree);
        
        //printf("currHead: %d\ncurrHead.right: %d\ncurrHead.r.r: %d\n", *(int*)currHead->value, *(int*)currHead->rightSibling->value, (currHead->rightSibling->rightSibling != NULL)?(*(int*)currHead->rightSibling->rightSibling->value):0);


        currHead= LinkNodes(currHead, (currHead->rightSibling), NULL, (currHead->rightSibling->rightSibling), compare);
        //printf("New linked head: %d\n", *(int*)currHead->value);

    }

    return currHead;
}

/**
 * This function merges two heaps heap1 and heap2 into one heap. The merged heap then becomes heap1 and heap2 becomes empty.
 * 
 * I'll have to keep track of 2 pointers, one on heap1, call it currHead_1, and one on heap2, call it currHead_2, as well as a prevHead_1 pointer to track
 * the left sibling of the currHead_1 pointer.
 * Now, this function shall unite the two heaps into heap1. Therefore heap2 will empty itself out into heap1.
 * I will compare the degrees of the two binomial trees rooted at currHead_1 and currHead_2 respectively.
 * This function works with a collection of binomial trees regardless the order. That is because when deleting the minimum, we will have
 * to unite with a heap in reverse order due to the nature of how children of a root are ordered in a binomial tree. They are ordered from
 * highest degree to lowest degree from left to right, whereas the collection of trees in a heap is ordered from smallest degree to largest
 * from left to right.
 * 
 * I then have three cases:
 *      - deg(currHead_1) > deg(currHead_2):
 *          Then currHead_2 is a binomial heap of smaller size than currHead_1 of heap1.
 *          Because we are assuming that heap2 could come in any order, then we can't make any assumptions about whether currHead_2 is has a higher
 *          degree than what was before currHead_1.
 *          So, what we do in this case is check for two conditions:
 *              - currHead_1 == heap1.head: then nothing is before it so it must be the smallest degree of heap1 --> thus place currHead_2 as the new head
 *              - currHead_1 != heap1.head: then smth is before it and we must go back and compare currHead_2 with what came before.
 *          Since we can't be sure that currHead_2 was compared to what is before currHead_1 we must start from the head, but if nothing was equal to it,
 *          it will end up reaching back to this same condition, so in case there was no equivalent degree in heap1, we must handle its placement in the
 *          second condition: deg(currHead_1) < deg(currHead_2); this way if no equivalent degree was found it will be placed right after the last
 *          tree with a smaller degree than it.
 *          
 *      - deg(currHead_1) < deg(currHead_2):
 *          If currHead_1 is the last root node, then just place currHead_2 as its right sibling.
 *          Else if there is a right sibling to currHead_1:
 *              - if the degree of the right sibling is greater than degree of currHead_2: place currHead_2 in between currHead_1 and its right sibling
 *              - if the degree of the right sibling is less than or equal: move currHead_1 to the right
 *      - deg(currHead_1) == deg(currHead_2):
 *          Then we combine the two nodes using LinkNodes() function. After that, we call the RecursiveLinkNode() function to make sure
 *          we recursively combine the resultant tree rooted at currHead_1 until its right child is either NULL or has a larger degree than it.
 * 
 * 
 */
int Union(BinomialHeap* heap1, BinomialHeap* heap2){

    //First check if any input is NULL:
    if (heap1 == NULL){
        return 0;
    }

    if (heap2 == NULL || heap2->head == NULL) return 0;

    //make sure heaps have same datatypes (same comparison functions):
    if (*(heap1->compare) != *(heap2->compare)) return 0;
    
    if (heap1->head == NULL){
        heap1->head = heap2->head;
    }
    else {

        Node * currHead_1 = heap1->head;
        Node * currHead_2 = heap2->head;
        Node * prevHead_1 = NULL;

        while (currHead_2 != NULL)
        {
            char c;
            //scanf("%c", &c);
            if (currHead_1 == NULL){\
                if (heap1->head == NULL) return 0;

                currHead_1 = heap1->head;
            }

            Node * nextHead_2 = currHead_2->rightSibling;
            Node * prevTemp = currHead_1;

            if (currHead_1->degree == currHead_2->degree){
                //Link the two nodes into currHead_1, then call the RecursiveLinkNodes() function on currHead_1 and its right sibling:
                currHead_1 = LinkNodes(currHead_1, currHead_2, prevHead_1, currHead_1->rightSibling, heap1->compare);

                currHead_1 = RecurseLinkToRightSibling(currHead_1, heap1->compare);

                //In case we are working with the head of the heap, we must update it:
                if (heap1->head == prevTemp){
                    heap1->head = currHead_1;
                }

                //In this case we want to leave the prev as it is but we must set preHead_1.right to the new currHead_1:
                if (prevHead_1 != NULL){
                    prevHead_1->rightSibling = currHead_1;
                }

                currHead_2 = nextHead_2;

            }
            else if (currHead_1->degree > currHead_2->degree){

                if (currHead_1 != heap1->head){//If there are trees before the current one, then go back and start from head because smaller trees exist
                    //This will make sure that currHead_2 is compared to every smaller tree before reaching this one. If no tree of its size is 
                    //found it will be placed between a tree smaller than it and one larger than it in the else below.
                    currHead_1 = heap1->head;
                    prevHead_1 = NULL;
                }
                else {
                    
                    heap1->head = currHead_2;

                    currHead_2->rightSibling = currHead_1;
                    
                    currHead_2 = nextHead_2;

                }

            }
            else{
                //For the case where currHead_2 is a tree whom all trees in heap1 are either smaller or larger than, we must check
                //if the right sibling is larger than it so that we place it in between this one and its sibling:
                if (currHead_1->rightSibling != NULL){

                    if (currHead_1->rightSibling->degree > currHead_2->degree){                        
                        //Then place curr2 as the right sibling of currHead_1:
                        currHead_2->rightSibling = currHead_1->rightSibling;
                        currHead_1->rightSibling = currHead_2;

                        //keep currHead_1 where it is, no need to move it

                        currHead_2 = nextHead_2;
                    }
                    else {
                        prevHead_1 = currHead_1;
                        currHead_1 = currHead_1->rightSibling;
                    }

                }
                else {

                    currHead_1->rightSibling = currHead_2;
                    currHead_2->rightSibling = NULL;
                    currHead_2 = nextHead_2;

                }

            }
        }

    }

    return 1;
}


void* findMin(BinomialHeap * heap){
    if (heap == NULL) return NULL;
    if (heap->compare == NULL) return NULL;

    Node * curr = heap->head;
    Node * minNode = curr;

    if (curr == NULL) return NULL;

    while (curr != NULL)
    {
        if (heap->compare(curr->value, minNode->value) < 0){
            minNode = curr;
        }
        curr = curr->rightSibling;
    }

    return minNode->value;
}

void* deleteMin(BinomialHeap * heap){

    

    /**
     * Okay, here I must first search for the minimum root (O(logn)), then cut off that tree from the heap, delete and free the root node, then
     * perform a union on the original heap and the one formed of the children of the deleted root.
     */

    Node * curr = heap->head;
    Node * prev = NULL;
    Node * minNode = curr;
    Node * leftSibOfMinNode = NULL;

    if (curr == NULL) return NULL;

    //Find the min:
    while (curr != NULL)
    {
        if (heap->compare(curr->value, minNode->value) < 0){
            minNode = curr;
            leftSibOfMinNode = prev;
        }
        prev = curr;
        curr = curr->rightSibling;
    }
    

    //Cut off the heap with the minimum node:
    if (leftSibOfMinNode != NULL){
        leftSibOfMinNode->rightSibling = minNode->rightSibling;
    }

    if (minNode == heap->head){
        heap->head = minNode->rightSibling;
    }

    //Make a heap out of the children (they are connected by right sibling bond):
    BinomialHeap childHeap;
    childHeap.compare = heap->compare;
    childHeap.head = minNode->leftChild;

    Union(heap, &childHeap);

    void* res = minNode->value;

    free(minNode);

    return res;

}

int compareInt(void* v1, void* v2){

    return *(int*)v1 - *(int*)v2;

}

int findMinInt(BinomialHeap * heap){

    return *(int*)findMin(heap);

}

int deleteMinInt(BinomialHeap * heap){

    void* res = deleteMin(heap);

    if (res == NULL) return 0;

    return *(int*)res;

}

BinomialHeap * ConstructIntBinomialHeap(int* array, int arrayLen, int (*compare)(void*, void*)){

    void** voidArr = (void**)(malloc(sizeof(void*) * arrayLen));

    for (int i = 0; i < arrayLen; i++)
    {
        int * ptr = (int*)(malloc(sizeof(int)));
        *ptr = array[i];

        voidArr[i] = (void*)ptr;
    }
    

    return ConstructBinomialHeap(voidArr, arrayLen, compare);

}


//Debugging material incoming...

void InOrderTraversalTree_INT(Node * root){
    if (root == NULL) return;

    
    InOrderTraversalTree_INT(root->leftChild);
    printf("%d, ", *((int*)(root->value)));
    
    InOrderTraversalTree_INT(root->rightSibling);
}


void InOrderTraversal_INT_Print(BinomialHeap * heap){

    if (heap == NULL) return;

    Node * curr = heap->head;


    InOrderTraversalTree_INT(curr);
    
    printf("\n");

}










/**
 * 
 *      Recall: 
 *          Properties of a binomial tree:
 *              - has 2^k nodes
 *              - has a height of k
 *              - there are kCi nodes at depth i for i = 0, 1, 2, ..., k
 *              - root has degree k (k children), which is larger than that of any other node in the tree. Its children
 *                are numbered by degree from left to right as k - 1, k - 2, ..., 0.
 * 
 *          Binomial heaps:
 *              Properties:
 *                  - A collection of binomial trees
 *                  - For every non-negative integer k, there is at most 1 tree of height k in the binomial heap
 *                  - There are at most floor(logn) + 1 binomial trees in a binomial heap because for any non-negative integer n,
 *                    you need at most floor(logn) + 1 bits to add up to it.
 * 
 * 
 * 
 * Inserting into a binomial heap is not like normal insertion, it requires heap union. That is in order to keep the binomial heap properties.
 * To keep number of trees O(logn) and height O(logn)
 * 
 * To insert into a binomial heap, we must insert the new element, and perform a union until the binomial heap properties are achieved.
 */