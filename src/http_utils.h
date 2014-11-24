#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

#include "query.h"

bool is_correct_query(const struct Buff* query, const int query_size);

bool is_version_1_1(const struct Buff* query, const int query_size);

char* convert_encoding(const char* str, int* res_len);

void get_path(const struct Buff* query, const int query_size, char** path);


#endif