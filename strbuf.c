#include <stdio.h>
#include <stdlib.h>

#include "strbuf.h"

StringBuffer *create_str_buffer(void) {
    StringBuffer *buf = (StringBuffer *) malloc(sizeof(StringBuffer));

    buf->str = (char *) malloc(DEF_STRBUF_SIZE_CHAR * sizeof(char));
    buf->size = DEF_STRBUF_SIZE_CHAR;
    buf->last_char_indx = 0;

    zero_str_buffer(buf);

    return buf;
}

void zero_str_buffer(StringBuffer *buf) {
    int i;

    for (i = 0; i < buf->size; i++)
        buf->str[i] = 0;
}

int push_char_to_buffer(StringBuffer *buf, char c) {
    if (buf->size <= buf->last_char_indx - 1)
        extend_str_buffer(buf);

    buf->str[buf->last_char_indx++] = c;

    return buf->last_char_indx;
}

int push_str_to_buffer(StringBuffer *buf, char *str, int ssize) {
    int i;

    for (i = 0; i < ssize; i++)
        push_char_to_buffer(buf, str[i]);

    return buf->last_char_indx;
}

int extend_str_buffer(StringBuffer *buf) {
    buf->size += STRBUF_EXTEND_SIZE;
    buf->str = (char *) realloc(buf->str, buf->size);

    return buf->size;
}