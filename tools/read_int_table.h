/* Read an exact row of integers while ignoring provenance comments and solver chatter. */
#ifndef RADIO_READ_INT_TABLE_H
#define RADIO_READ_INT_TABLE_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int radio_read_int_row(FILE *fp, int *values, size_t count) {
    char line[4096];
    int discard_continuation = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        int has_newline = strchr(line, '\n') != NULL;
        if (discard_continuation) {
            discard_continuation = !has_newline;
            continue;
        }
        if (line[0] == '#') {
            discard_continuation = !has_newline;
            continue;
        }
        char *p = line;
        size_t i;
        for (i = 0; i < count; i++) {
            while (*p == ' ' || *p == '\t') p++;
            errno = 0;
            char *end;
            long value = strtol(p, &end, 10);
            if (end == p || errno == ERANGE || value < INT_MIN || value > INT_MAX) break;
            values[i] = (int)value;
            p = end;
        }
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (i == count && *p == 0 && (has_newline || feof(fp))) return 1;
        /* Never reconsider the tail of an overlong comment/chatter line as a numeric row. */
        discard_continuation = !has_newline;
    }
    return 0;
}

#endif
