#ifndef SHARED_LIST_H
#define SHARED_LIST_H

#include <stddef.h>
#include "func_types.h"

typedef struct list {
    void* head;
    struct list* tail;
} list_t;

typedef struct list2 {
    void* val;
    struct list2* next;
    struct list2* prev;
} list2_t;

list_t* new_list(void* head, list_t* tail);
list_t* cut_head(list_t* list, check_func freeer);
bool list_constains(list_t* list, void* elem);

list2_t* insert_before(list2_t* curr, void* new_val);
list2_t* insert_after(list2_t* curr, void* new_val);

//returns next or null if absent
list2_t* remove_current(list2_t* curr, check_func freeer);
size_t length2(list2_t* list);

#endif //SHARED_LIST_H
