#include "http_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>

#include "query.h"

const size_t ARR_STANDART_SIZE = 10;

bool is_correct_query(const struct Buff* const query, const int query_size) {
    if (query_size <= 1) {
        return false;
    }
    return strstr(query[0].data, "GET") != NULL;
}

bool is_version_1_1(const struct Buff* const query, const int query_size)
{
    if (strstr(query[0].data, "HTTP/1.1") == NULL) {
        return false;
    }
    for (int i = 1; i < query_size; ++i) {
        if (strstr(query[i].data, "Host:") != NULL) {
            return true;
        }
    }
    return false;
}

char* convert_encoding(const char* const str, int* res_len) {
    const int str_len = strlen(str);
    char* res = (char*) calloc(ARR_STANDART_SIZE, sizeof(char));
    if (res == NULL) {
        *res_len = 0;
        return NULL;
    }
    int res_i = 0, res_size = ARR_STANDART_SIZE;

    int i = 0;
    while (i < str_len) {
        if (res_i == res_size) {
            char* tmp = (char*) realloc(res, 2 * res_size * sizeof(char));
            if (tmp == NULL) {
                free(res);
                *res_len = 0;
                return NULL;
            }
            res = tmp;
            res_size *= 2;
        }

        if (str[i] == '%') {
            int d = 0;
            sscanf(&str[i + 1], "%2x", &d);
            res[res_i] = (char) d;
            ++res_i;

            i += 3;
        } else {
            res[res_i] = str[i];
            ++res_i;

            ++i;
        }
    }

    if (res_i == res_size) {
        char* tmp = (char*) realloc(res, (res_size + 1) * sizeof(char));
        if (tmp == NULL) {
            free(res);
            *res_len = 0;
            return NULL;
        }
        res = tmp;
        ++res_size;
    }
    res[res_i] = '\0';
    *res_len = res_i;
    return res;
}

void get_path(const struct Buff* query, const int query_size, char** path) {
    if (path == NULL) {
        return;
    }
    if (*path != NULL) {
        free(*path);
    }
    *path = (char*) calloc(query[0].size, sizeof(char));
    if (*path == NULL) {
        return;
    }
    sscanf(query[0].data, "%s %s", *path, *path);
    int len_res = strlen(*path);
    char* res = convert_encoding(*path, &len_res);
    strcpy(*path, res);
    free(res);
}
