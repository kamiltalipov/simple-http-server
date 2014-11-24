#ifndef __QUERY_H__
#define __QUERY_H__

#include <stddef.h>
#include <stdbool.h>

struct Buff 
{
    char* data;
    size_t buff_length;
    size_t size;
    size_t position;
};

void make_buff(struct Buff* buff, size_t length);

void extend_buff(struct Buff* buff);

void free_buff(struct Buff* buff);

int get_char(int fd, struct Buff* buff);

struct Buff read_to(int fd, struct Buff* buff, char* sep);

int get_query(int fd, struct Buff** lines);

#endif