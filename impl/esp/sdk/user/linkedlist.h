#include "osapi.h"
#include "os_type.h"
#include "mem.h"


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
linked_list* ICACHE_FLASH_ATTR
new_linked_list();

void ICACHE_FLASH_ATTR
add(linked_list* list, void* data);
