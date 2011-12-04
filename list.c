/* Ken Sheedlo
 * Simple circular linked list implementation */

#include "list.h"

void list_init(list_t *list){
    //Initialize the list to empty (one sentinel node).
    node_t *new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        CriticalError("Failed to allocate memory");
    }
    new_node->data = NULL;
    new_node->next = new_node;
    new_node->prev = new_node;

    list->head = new_node;
    list->length = 0;
}

void list_addfirst(list_t *list, void *data){
    //Add data to a new node at the head of the list.
    node_t *new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        CriticalError("Failed to allocate memory");
    }

    new_node->data = data;
    new_node->next = list->head->next;
    new_node->prev = list->head;
    new_node->list = list;

    new_node->next->prev = new_node;
    list->head->next = new_node;

    list->length = list->length + 1;
}

void list_addlast(list_t *list, void *data){
    //Add data to a new node at the tail of the list.
    node_t *new_node = malloc(sizeof(node_t));
    if(new_node == NULL){
        CriticalError("Failed to allocate memory");
    }

    new_node->data = data;
    new_node->list = list;
    new_node->next = list->head;
    new_node->prev = list->head->prev;

    new_node->prev->next = new_node;
    list->head->prev = new_node;

    list->length = list->length + 1;
}

void *list_remove(node_t *node){
    /*Remove the specified node from it's list and return a ptr to its data
     * entry. */
    void *ret = node->data;

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->list->length = node->list->length - 1;

    free(node);
    return ret;
}

void list_clear(list_t *list, int32_t free_data){
    //Clear the list, freeing data if specified.
    while(list->head->next != list->head){
        void *data = list_remove(list->head->next);
        if(free_data){
            free(data);
        }
    }
    free(list->head);
}

int32_t list_match(list_t *lst0, node_t *s0, list_t *lst1, node_t *s1, 
    int32_t (*eq)(const void *, const void*), int32_t count){
    
    node_t *lhs = s0, *rhs = s1;
    for(int i = 0; i<count; i++){
        if(lhs == lst0->head || rhs == lst1->head){
            return 0;  /*We hit the end of the list */
        }
        if(!eq(lhs->data, rhs->data)){
            return 0; /*Values not equal w.r.t. user-defined function */
        }
        lhs = lhs->next;
        rhs = rhs->next;
    }
    return 1;
}


