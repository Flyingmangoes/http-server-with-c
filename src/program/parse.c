#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "server.h"

HttpRequest* HttpRequest_Parse(const char *raw_request) {

};

void HttpRequest_Free(HttpRequest *req) {
    
};