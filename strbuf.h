#if !defined(STRBUF_H)
#define STRBUF_H

#define DEF_STRBUF_SIZE_CHAR    128
#define STRBUF_EXTEND_SIZE  16

typedef struct _StringBuffer {
    char *str;
    int size;
    int last_char_indx;
} StringBuffer;

StringBuffer *create_str_buffer(void);
void zero_str_buffer(StringBuffer *buf);
int push_char_to_buffer(StringBuffer *buf, char c);
int push_str_to_buffer(StringBuffer *buf, char *str, int ssize);
int extend_str_buffer(StringBuffer *buf);

#endif