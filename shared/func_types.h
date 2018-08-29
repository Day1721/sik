#ifndef SHARE_FUNC_TYPES_H
#define SHARE_FUNC_TYPES_H

#include <stdbool.h>

typedef bool (*filter_func)(void *);

typedef void* (*map_func)(void *);

typedef void (*check_func)(void *);

void* id(void *);
void no_act(void*);

#endif //SHARE_FUNC_TYPES_H
