#include <stdlib.h>

#include "list.h"

list_t* new_list(void* head, list_t* tail) {
    list_t* res = (list_t*)malloc(sizeof(list_t));
    res->head = head;
    res->tail = tail;
    return res;
}

list_t* cut_head(list_t* list, check_func freeer) {
    if (list == NULL) return NULL;
    list_t* res = list->tail;
    freeer(list->head);
    free(list);
    return res;
}

bool list_constains(list_t* list, void* elem) {
    for (;list != NULL; list = list->tail) {
        if (list->head == elem) return true;
    }
    return false;
}




list2_t* insert_before(list2_t* l, void* new_val) {
    list2_t* ll = NULL;
    ll = (list2_t*)malloc(sizeof(list2_t));
    ll->val = new_val;

    if (l == NULL) {
        ll->prev = ll->next = ll;
        return ll;
    }
    
    ll->next = l;
    ll->prev = l->prev;
    l->prev = ll;
    return ll;
}

list2_t* insert_after(list2_t* l, void* new_val) {
    list2_t* ll = NULL;
    ll = (list2_t*)malloc(sizeof(list2_t));
    ll->val = new_val;

    if (l == NULL) {
        ll->prev = ll->next = ll;
        return ll;
    }
    
    ll->prev = l;
    ll->next = l->next;
    l->next = ll;
    return ll;
}

list2_t* remove_current(list2_t* curr, check_func freeer) {
    if (curr == NULL) return NULL;

    list2_t* res = NULL;
    if (curr->next != curr) {
        curr->next->prev = curr->prev;
        curr->prev->next = curr->next;
        res = curr->next;
    }

    freeer(curr->val);
    free(curr);

    return res;
}

size_t length2(list2_t* l) {
    if (l == NULL) return 0;
    
    list2_t* start = l;
    size_t cnt = 1;
    while (start != l->next) {
        cnt++;
        l = l->next;
    }
    return cnt;
}