#include "linkedlist.h"


static linked_list_node* new_linked_list_node() {
    linked_list_node* node = (
        (linked_list_node*)os_malloc(sizeof(linked_list_node)));
    memset(node, 0, sizeof(linked_list_node));
    return node;
}

void add(linked_list* list, void* data) {
    if(list == 0) {
        return;
    }

    linked_list_node* node = new_linked_list_node();
    node->data = (void*)data;

    if(list->head == 0) {
        list->head = list->tail = node;
        list->length++;
        return;
    }

    list->tail->next = node;
    list->tail = node;
    list->length++;
}

linked_list* new_linked_list() {
    linked_list* list = (linked_list*)os_malloc(sizeof(linked_list));
    memset(list, 0, sizeof(linked_list));
    list->add = add;
    return list;
}
