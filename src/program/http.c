#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

const char HTTP_VERSION[] = "HTTP/1.1";
const HttpMethodMap KNOWN_HTTP_METHODS[] = {
    {.str = "GET",   .typ = HTTP_GET   },
    {.str = "POST",  .typ = HTTP_POST  },
    {.str = "PUT", 	 .typ = HTTP_PUT   },
    {.str = "PATCH", .typ = HTTP_PATCH },
    {.str = "DELETE",.typ = HTTP_DELETE},
};

size_t KNOWN_HTTP_METHODS_LEN = sizeof(KNOWN_HTTP_METHODS) / sizeof(KNOWN_HTTP_METHODS[0]);

int do_GET(const struct HttpHeader *self, char *header) {

};

int do_POST(const struct HttpHeader* header) {

};

int do_PUT(const struct HttpHeader* header) {

};

int do_PATCH(const struct HttpHeader* header) {

};

int do_DELETE(const struct HttpHeader* header) {

};


