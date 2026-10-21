/**
 * Tests for the data structures the bot is built on: the doubly linked list (D_LinkedList.c) and the binomial heap (BinomialHeap.c).
 */
#include <stdlib.h>
#include "test.h"
#include "../include/D_LinkedList.h"
#include "../include/BinomialHeap.h"

static int *IntPtr(int v){
    int *p = malloc(sizeof(int));
    *p = v;
    return p;
}

void test_list_add_remove(void){
    D_LinkedList *list = create_empty_Dlist();
    int a = 1, b = 2, c = 3;

    CHECK(is_empty(list));
    addLast(list, &b);
    addFirst(list, &a);
    addLast(list, &c);
    CHECK_EQ_INT(list->size, 3);
    CHECK(peek(list) == &a);

    //Walk forwards and backwards: the prev links must be consistent too
    CHECK(get_data(get_first(list)) == &a);
    CHECK(get_data(get_next(get_first(list))) == &b);
    CHECK(get_data(get_prev(list->tail)) == &b);
    CHECK(get_prev(list->head) == NULL);

    CHECK(removeLast(list) == &c);
    CHECK(removeFirst(list) == &a);
    CHECK(removeFirst(list) == &b);
    CHECK(is_empty(list));
    CHECK(list->tail == NULL);
    CHECK(removeFirst(list) == NULL);

    free_D_LinkedList(list);
}

void test_list_remove_node_anywhere(void){
    D_LinkedList list;
    initialize_empty_DList(&list);
    int v[4] = {0, 1, 2, 3};

    addFirst(&list, &v[1]);
    addFirst(&list, &v[0]); //head added with addFirst: its prev must be NULL
    addLast(&list, &v[2]);
    addLast(&list, &v[3]);

    CHECK(removeNode(&list, list.head) == &v[0]);     //head
    CHECK(removeNode(&list, list.tail) == &v[3]);     //tail
    CHECK(list.head->prev == NULL);
    CHECK(list.tail->next == NULL);

    D_ListNode *middle = list.head;
    CHECK_EQ_INT(disconnectNode(&list, middle), 1);   //unlinks without freeing
    CHECK_EQ_INT(list.size, 1);
    CHECK(list.head == list.tail);
    CHECK(list.head->data == &v[2]);
    free(middle);

    removeFirst(&list);
    CHECK(is_empty(&list));
}

void test_heap_find_and_delete_in_order(void){
    //Insert numbers in a scrambled order, then deleteMin must give them back sorted.
    int count = 200;
    BinomialHeap *heap = ConstructBinomialHeap(NULL, 0, compareInt);

    for (int i = 0; i < count; i++){
        insert(heap, IntPtr((i * 37) % count)); //37 and 200 are coprime: every number 0..199 exactly once
    }

    for (int expected = 0; expected < count; expected++){
        CHECK_EQ_INT(*(int *)findMin(heap), expected);
        int *v = deleteMin(heap);
        CHECK(v != NULL);
        if (v == NULL) break;
        CHECK_EQ_INT(*v, expected);
        free(v);
    }
    CHECK(heap->head == NULL);
    CHECK(deleteMin(heap) == NULL);
    CHECK(findMin(heap) == NULL);
    free(heap);
}

void test_heap_with_duplicates_and_every_size(void){
    //Binomial heaps change shape at every power of two, so test all sizes up to 64, with duplicates.
    for (int n = 1; n <= 64; n++){
        BinomialHeap *heap = ConstructBinomialHeap(NULL, 0, compareInt);
        for (int i = 0; i < n; i++) insert(heap, IntPtr((i * 7) % 5));

        int prev = -1, popped = 0;
        int *v;
        while ((v = deleteMin(heap)) != NULL){
            CHECK(*v >= prev);
            prev = *v;
            popped++;
            free(v);
        }
        CHECK_EQ_INT(popped, n);
        free(heap);
    }
}

void test_heap_construct_from_array_and_union(void){
    void *a[5] = {IntPtr(9), IntPtr(4), IntPtr(7), IntPtr(1), IntPtr(8)};
    void *b[3] = {IntPtr(6), IntPtr(0), IntPtr(5)};
    BinomialHeap *h1 = ConstructBinomialHeap(a, 5, compareInt);
    BinomialHeap *h2 = ConstructBinomialHeap(b, 3, compareInt);

    CHECK_EQ_INT(*(int *)findMin(h1), 1);
    CHECK_EQ_INT(Union(h1, h2), 1);
    CHECK_EQ_INT(*(int *)findMin(h1), 0);

    int expected[8] = {0, 1, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 8; i++){
        int *v = deleteMin(h1);
        CHECK(v != NULL);
        if (v == NULL) break;
        CHECK_EQ_INT(*v, expected[i]);
        free(v);
    }
    CHECK(h1->head == NULL);
    free(h1);
    free(h2);

    CHECK(ConstructBinomialHeap(NULL, 0, NULL) == NULL); //a heap needs a compare function
}
