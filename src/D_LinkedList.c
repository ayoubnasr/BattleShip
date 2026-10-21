#include <stdio.h>
#include <stdlib.h>
#include "D_LinkedList.h"

void initialize_empty_DList(D_LinkedList * list){
    if (list == NULL) return;

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

D_LinkedList* create_empty_Dlist() {
    D_LinkedList * list = (D_LinkedList*)malloc(sizeof(D_LinkedList));

    if (list == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

int is_empty(D_LinkedList* list) {
    return list->head == NULL;
}

void addFirst(D_LinkedList* list, void* data) {
    D_ListNode* new_node = (D_ListNode*)malloc(sizeof(D_ListNode));

    if (new_node == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->next = list->head;

    if (list->head == NULL){
        list->tail = new_node;
    }
    else{
        list->head->prev = new_node;
    }

    list->head = new_node;

    list->size++;
}

void* removeFirst(D_LinkedList* list) {
    if (is_empty(list)) {
        return NULL; 
    }
    D_ListNode* temp = list->head;
    void* data = temp->data;
    list->head = list->head->next;
    if (list->head != NULL){
        list->head->prev = NULL;
    }
    else {
        list->tail = NULL;
    }
    free(temp);

    list->size--;

    return data;
}

void* removeLast(D_LinkedList* list) {
    if (is_empty(list)) {
        return NULL; 
    }
    D_ListNode* temp = list->tail;
    void* data = temp->data;
    list->tail = list->tail->prev;
    if (list->tail != NULL){
        list->tail->next = NULL;
    }
    else {
        list->head = NULL;
    }
    free(temp);

    list->size--;

    return data;
}

void* removeNode(D_LinkedList* list, D_ListNode* node) {
    if (list == NULL || node == NULL || is_empty(list)) {
        return NULL; // List is empty or node is NULL
    }

    void* data = node->data;

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        // Node is the head
        list->head = node->next;
    }

    // Adjust links of the next node
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        // Node is the tail
        list->tail = node->prev;
    }

    free(node);
    list->size--;

    return data;
}

int disconnectNode(D_LinkedList* list, D_ListNode* node) {
    if (list == NULL || node == NULL || is_empty(list)) {
        return -1; // List is empty or node is NULL
    }

    void* data = node->data;

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        // Node is the head
        list->head = node->next;
    }

    // Adjust links of the next node
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        // Node is the tail
        list->tail = node->prev;
    }

    list->size--;
}


void* peek(D_LinkedList* list) {
    if (is_empty(list)) {
        return NULL; 
    }
    return list->head->data;
}

void addLast(D_LinkedList* list, void* data) {
    D_ListNode* new_node = (D_ListNode*)malloc(sizeof(D_ListNode));

    if (new_node == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;

    //printf("DASFA\n");

    if (is_empty(list)) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->size++;
}

void free_D_LinkedList(D_LinkedList* list) {
    while (!is_empty(list)) {
        removeFirst(list);
    }
    free(list);
}

//Below are iterator functions:

D_ListNode* get_first(D_LinkedList* list) {
    return list->head;
}

D_ListNode* get_next(D_ListNode* node) {
    if (node == NULL) return NULL;
    return node->next;
}

D_ListNode* get_prev(D_ListNode* node) {
    if (node == NULL) return NULL;
    return node->prev;
}

void* get_data(D_ListNode* node) {
    if (node == NULL) return NULL;
    return node->data;
}

