#include "server.h"

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <stdbool.h>

#include "query.h"
#include "http_utils.h"
#include "http_constant.h"
#include "html_constant.h"

const int MAX_NUM_STR_LENGTH = 35;

void check_errno(const char* where) {
    if (errno) {
        perror(where);
        exit(1);
    }
}

char* get_file_extension(const char* path) {
    int i = 0;
    const int path_len = strlen(path);
    for (i = path_len - 1; i > 0; --i) {
        if (path[i] == '.') {
            break;
        }
    }
    if (path[i] == '.') {
        char* res = (char*) calloc(path_len - i + 2, sizeof(char));
        for (int j = i + 1; j < path_len; ++j) {
            res[j - i - 1] = path[j];
        }
        res[path_len - i - 1] = '\0';
        return res;
    }
    return NULL;
}

int send_file(int fd, const char* path) {
    if (path == NULL) {
        return -1;
    }

    struct stat st;
    if (lstat(path, &st) == -1) {
        if (errno == EACCES) {
            return -2;
        }
        return -1;
    }

    send(fd, HTTP_OK, strlen(HTTP_OK), 0);
    send(fd, HTTP_FILE_CONTENT_TYPE_BEGIN, strlen(HTTP_FILE_CONTENT_TYPE_BEGIN), 0);
    char* file_extension = get_file_extension(path);
    send(fd, file_extension, strlen(file_extension), 0);
    free(file_extension);
    send(fd, HTTP_FILE_CONTENT_TYPE_END, strlen(HTTP_FILE_CONTENT_TYPE_END), 0);
    send(fd, HTTP_CONTENT_ENCODING, strlen(HTTP_CONTENT_ENCODING), 0);
    send(fd, HTTP_CONTENT_LENGHT_BEGIN, strlen(HTTP_CONTENT_LENGHT_BEGIN), 0);
    char file_size[MAX_NUM_STR_LENGTH];
    snprintf(file_size, MAX_NUM_STR_LENGTH, "%d", (int)st.st_size);
    send(fd, file_size, strlen(file_size), 0);
    send(fd, HTTP_CONTENT_LENGHT_END, strlen(HTTP_CONTENT_LENGHT_END), 0);
    send(fd, HTTP_END, strlen(HTTP_END), 0);

    int file_d = open(path, O_RDONLY);
    if (file_d == -1) {
        if (errno == EACCES) {
            return -2;
        }
        return -1;
    }
    sendfile(fd, file_d, NULL, st.st_size);
    close(file_d);

    return 0;
}

void transfer_data(int fd) {
    struct Buff* query = NULL;
    int query_size = get_query(fd, &query);
    if (!is_correct_query(query, query_size)) {
        //Bad_reques;
        send(fd, HTTP_BAD_REQUEST, strlen(HTTP_BAD_REQUEST), 0);
        send(fd, HTTP_END, strlen(HTTP_END), 0);
        return;
    }
    if (!is_version_1_1(query, query_size)) {
        //Unsupport version
        send(fd, HTTP_UNSUPPORTED_VERSION, strlen(HTTP_UNSUPPORTED_VERSION), 0);
        send(fd, HTTP_END, strlen(HTTP_END), 0);
        return;
    }

    char* path = NULL;
    get_path(query, query_size, &path);
    if (path == NULL) {
        //Error at get path
        send(fd, HTTP_NOT_FOUND, strlen(HTTP_NOT_FOUND), 0);
        send(fd, HTTP_END, strlen(HTTP_END), 0);
        return;
    }
    const int path_len = strlen(path);
    //fprintf(stderr, "Path = %s\n", path);

    for (int i = 0; i < query_size; ++i) {
        free_buff(&query[i]);
    }
    free(query);

    DIR* dir = opendir(path);
    if (dir == NULL) {
        if (errno == EACCES) {
            send(fd, HTTP_FORBIDDEN, strlen(HTTP_FORBIDDEN), 0);
            send(fd, HTTP_END, strlen(HTTP_END), 0);
        } else {
            int ret = 0;
            if ((ret = send_file(fd, path)) == -1) {
                fprintf(stderr, "Can't open dir or file(path - %s, error - %d)\n", path, errno);
                send(fd, HTTP_NOT_FOUND, strlen(HTTP_NOT_FOUND), 0);
                send(fd, HTTP_END, strlen(HTTP_END), 0);
            } else if (ret == -2) {
                send(fd, HTTP_FORBIDDEN, strlen(HTTP_FORBIDDEN), 0);
                send(fd, HTTP_END, strlen(HTTP_END), 0);
            }
        }
        free(path);
        return;
    }

    send(fd, HTTP_OK, strlen(HTTP_OK), 0);
    send(fd, HTTP_HTML_CONTENT_TYPE, strlen(HTTP_HTML_CONTENT_TYPE), 0);
    send(fd, HTTP_END, strlen(HTTP_END), 0);
    send(fd, HTML_BEGIN, strlen(HTML_BEGIN), 0);
    struct dirent* ent = NULL;
    int index = 0;
    char i_file[MAX_NUM_STR_LENGTH];
    while ((ent = readdir(dir)) != NULL) {
        snprintf(i_file, MAX_NUM_STR_LENGTH, "%d", index);
        ++index;
        int link_path_len = path_len + 1 + strlen(ent->d_name) + 1;
        char* link_path = (char*) calloc(link_path_len, sizeof(char));
        if (link_path == NULL) {
            fprintf(stderr, "Can't allocate memory!\n");
            continue;
        }
        if (path[path_len - 1] == '/') {
            snprintf(link_path, link_path_len, "%s%s", path, ent->d_name);
        } else {
            snprintf(link_path, link_path_len, "%s/%s", path, ent->d_name);
        }

        send(fd, HTML_TABLE_BEGIN, strlen(HTML_TABLE_BEGIN), 0);
        send(fd, i_file, strlen(i_file), 0);

        send(fd, " ", 1, 0);

        send(fd, HTML_LINK_BEGIN, strlen(HTML_LINK_BEGIN), 0);
        send(fd, link_path, strlen(link_path), 0);
        send(fd, HTML_LINK_MIDDLE, strlen(HTML_LINK_MIDDLE), 0);
        send(fd, ent->d_name, strlen(ent->d_name), 0);
        send(fd, HTML_LINK_END, strlen(HTML_LINK_END), 0);

        send(fd, HTML_TABLE_END, strlen(HTML_TABLE_END), 0);

        free(link_path);
    }
    closedir(dir);
    free(path);

    send(fd, HTML_END, strlen(HTML_END), 0);
}

void finalaze_socket(int fd) {
    shutdown(fd, SHUT_WR);
    char buff[10];
    recv(fd, buff, 10 * sizeof(char), 0);
    close(fd);
}


void run_server(const char* IP_ADDR, int PORT_NUMBER, int MAX_BACKLOG) {
	int socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_d == -1) {
        check_errno("socket");
    }
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT_NUMBER);
    local_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    if (!bind(socket_d, (const struct sockaddr*) &local_addr, sizeof(struct sockaddr_in))) {
        check_errno("bind");
    }

    while (!listen(socket_d, MAX_BACKLOG)) {
        struct sockaddr_in remote_addr;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        int rs = accept(socket_d, (struct sockaddr*) &remote_addr, &addr_len);
        if (rs == -1) {
            check_errno("accept");
        }
        if (fork() == 0) {
            close(socket_d);
            fprintf(stderr, "Connected: %s:%d\n", inet_ntoa(remote_addr.sin_addr),
                     (int) ntohs(remote_addr.sin_port));
            transfer_data(rs);

            finalaze_socket(rs);

            fprintf(stderr, "Closed: %s:%d\n", inet_ntoa(remote_addr.sin_addr),
                     (int) ntohs(remote_addr.sin_port));
            exit(0);
        }
        close(rs);
    }
    close(socket_d);
    check_errno("listen");
}