#include <stdlib.h>
#include <string.h>


typedef struct linked_list_node_struct {
    void* data;
    void* next;
} linked_list_node;

typedef struct linked_list_struct {
    linked_list_node* head;
    linked_list_node* tail;
    int length;

    void (*add)(struct linked_list_struct* list, void*data);
} linked_list;

// linked_list_node* new_linked_list_node();
linked_list* new_linked_list();
void add(linked_list* list, void* data);
