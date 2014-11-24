#ifndef __HTTP_CONSTANT_H__
#define __HTTP_CONSTANT_H__

const char* const HTTP_OK = "\
HTTP/1.1 200 Ok\r\n\
Connection: close\r\n\
Accept-Ranges: none\r\n";
const char* const HTTP_BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
const char* const HTTP_UNSUPPORTED_VERSION = "HTTP/1.0 505 HTTP Version not supported\r\n";
const char* const HTTP_NOT_FOUND = "HTTP/1.1 404 Not Found\r\n";
const char* const HTTP_FORBIDDEN = "HTTP/1.1 403 Forbidden\r\n";
const char* const HTTP_END = "\r\n";
const char* const HTTP_HTML_CONTENT_TYPE = "Content-Type: text/html; charset=UTF-8\r\n";
const char* const HTTP_FILE_CONTENT_TYPE_BEGIN = "Content-Type: application/";
const char* const HTTP_CONTENT_ENCODING = "Content-Transfer-Encoding: binary\r\n";
const char* const HTTP_CONTENT_LENGHT_BEGIN = "Content-Length: ";
const char* const HTTP_CONTENT_LENGHT_END = "\r\n";
const char* const HTTP_FILE_CONTENT_TYPE_END = "\r\n";

#endif