#ifndef D_LINKEDLIST
#define D_LINKEDLIST

typedef struct D_ListNode{

    struct D_ListNode * next;
    struct D_ListNode * prev;

    void * data;

} D_ListNode;

typedef struct{

    D_ListNode * head;
    D_ListNode * tail;

    int size;

} D_LinkedList;

void initialize_empty_DList(D_LinkedList * list);

D_LinkedList* create_empty_Dlist();

int is_empty(D_LinkedList* list);

void addFirst(D_LinkedList* list, void* data);

void* removeFirst(D_LinkedList* list);

void* removeLast(D_LinkedList* list);

void* removeNode(D_LinkedList* list, D_ListNode* node);

int disconnectNode(D_LinkedList* list, D_ListNode* node);

void* peek(D_LinkedList* list);

void addLast(D_LinkedList* list, void* data);

void free_D_LinkedList(D_LinkedList* list);

D_ListNode* get_first(D_LinkedList* list);

D_ListNode* get_next(D_ListNode* node);

D_ListNode* get_prev(D_ListNode* node);

void* get_data(D_ListNode* node);




#endif