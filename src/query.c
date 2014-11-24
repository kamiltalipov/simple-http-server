#include "query.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

const int BUFF_LENGTH = 100;
const int DEFAULT_SIZE = 20; 

void make_buff(struct Buff* buff, size_t length) {
    buff->data = (char*) calloc(length, sizeof(char));
    buff->buff_length = length;
    buff->size = 0;
    buff->position = 0;
}

void extend_buff(struct Buff* buff) {
    char* new_data = (char*) calloc(2 * buff->buff_length, sizeof(char));
    for (size_t i = 0; i < buff->size; ++i) {
        new_data[i] = buff->data[i];
    }
    free(buff->data);
    buff->data = new_data;
    buff->buff_length *= 2;
}

void free_buff(struct Buff* buff) {
    free(buff->data);
    buff->data = NULL;
    buff->buff_length = 0;
    buff->size = 0;
    buff->position = 0;
}

int get_char(int fd, struct Buff* buff) {
    if (buff->position == buff->size) {
        buff->position = 0;
        buff->size = 0;
        int r = read(fd, buff->data, buff->buff_length * sizeof(char));
        if (r > 0) {
            buff->size = r;
        }
    }
    if (buff->position == buff->size) {
        return EOF;
    }
    return buff->data[buff->position++];
}

struct Buff read_to(int fd, struct Buff* buff, char* sep) {
    const size_t sep_length = strlen(sep);
    struct Buff res;
    make_buff (&res, BUFF_LENGTH);
    while (1) {
        if (res.size == res.buff_length) {
            extend_buff (&res);
        }
        int c = get_char(fd, buff);
        if (c == EOF) {
            break;
        }
        res.data[res.size++] = (char) c;
        if (res.size < sep_length) {
            continue;
        }
        if (!strncmp(sep, &res.data[res.size - sep_length], sep_length)) {
            res.size -= sep_length;
            break;
        }
    }
    return res;
}

int get_query(int fd, struct Buff** lines) {
    if (lines == NULL) {
        return -1;
    }
    if (*lines != NULL) {
        free(*lines);
    }
    *lines = (struct Buff*) calloc(DEFAULT_SIZE, sizeof(struct Buff));
    if (*lines == NULL) {
        return -2;
    }
    int line_size = DEFAULT_SIZE;
    int line_count = 0;
    struct Buff buff;
    make_buff(&buff, BUFF_LENGTH);
    while (1) {
        struct Buff line = read_to(fd, &buff, "\r\n");
        if (line.size == 0) {
            free_buff(&line);
            break;
        }

        if (line_count == line_size) {
            struct Buff* tmp = (struct Buff*) realloc(*lines, 2 * line_size * sizeof(struct Buff));
            if (tmp == NULL) {
                free_buff (&buff);
                for (int i = 0; i < line_count; ++i) {
                    free_buff(&((*lines)[i]));
                }
                free(*lines);
                *lines = NULL;
                return -2;
            }
            *lines = tmp;
            line_size *= 2;
        }
        (*lines)[line_count] = line;
        ++line_count;
    }
    free_buff(&buff);

    return line_count;
}