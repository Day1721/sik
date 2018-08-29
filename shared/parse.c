#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

#include "parse.h"

long checked_strtol(char* str) {
    errno = 0;
    char* endptr = NULL;
    int base = 10;

    long number = strtol(str, &endptr, base);
    if (errno == 0 && str && !*endptr && number > 0) {
        return number;
    } else {
        return -1;
    }
}


int64_t str_to_i64(const char *s) {
  uint64_t i;
  char c;
  int scanned = sscanf(s, "%" SCNd64 "%c", &i, &c);
  if (scanned == 1) return i;
  return -1;  
}