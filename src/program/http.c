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

DWORD WINAPI _handle_connection(LPVOID param) {
    assert(param != NULL);

    SOCKET conn_fd = *(SOCKET *)param;

    printf("Processing connection with fd %d\n", conn_fd);

    HttpRequest req = {0};
    req._buffer = malloc(BUFFER_SIZE);

    FILE *fp = NULL;

    if(req._buffer == NULL) {
        printf("error: Failed to allocate memmory\n");
        goto cleanup;
    }

    memset(req._buffer, 0, BUFFER_SIZE);

    int bytes_read = recv(conn_fd, req._buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read == SOCKET_ERROR) {
        printf("error: Recv failed: %d\n", WSAGetLastError());
        goto cleanup;
    } else if (bytes_read == 0) {
        printf("error: Recv: connection closed by client\n");
        goto cleanup;
    }

    req._buffer[bytes_read] = '\0';
    req._buffer_len = (size_t)bytes_read;

    char *line_start = req._buffer;
    char *line_end = strstr(line_start, "\r\n");

    if(line_end == NULL) {
        printf("error: No CLRF found\n");
        goto cleanup;
    }

    *line_end = '\0';

    char *method = strtok(line_start, " "); 
    char *path = strtok(NULL, " ");
    char *http_version = strtok(NULL, " ");

    if (method == NULL || path == NULL) {
        printf("error: Failed to parse request line\n");
        goto cleanup;
    }

    for (size_t i = 0; i < KNOWN_HTTP_METHODS_LEN; i++) {
        if(strcmp(method, KNOWN_HTTP_METHODS[i].str) == 0) {
            req.method = KNOWN_HTTP_METHODS[i].typ;
            break;
        }
    }

    if(req.method == HTTP_UNKNOWN) {
        printf("error(parse): Unknown method\n");
        goto cleanup;
    }

    req.path = path;
    if(req.path == NULL) {
        printf("error: Failed to parse path\n");
        goto cleanup;
    }

    char *headers_start = line_end + 2;
   
    req.headers = malloc(sizeof(HttpHeader) * MAX_HEADERS);
    if(req.headers == NULL) {
        printf("error: Failed to allocate header\n");
        goto cleanup;
    }

    req.headers_len = 0;

    char *header_line = headers_start;
    while (req.headers_len < MAX_HEADERS) {
        char *next_line = strstr(header_line, "\r\n");
        if (next_line == NULL) break;
        
        *next_line = '\0';
        
        if (header_line[0] == '\0') {
            headers_start = next_line + 2;
            break;
        }
        
        // Parse "Key: Value"
        char *colon = strchr(header_line, ':');
        if (colon != NULL) {
            *colon = '\0';
            char *key = header_line;
            char *value = colon + 1;
            
            // Skip leading space in value
            while (*value == ' ') value++;
            
            req.headers[req.headers_len].key = key;
            req.headers[req.headers_len].value = value;
            req.headers_len++;
        }
        
        header_line = next_line + 2;
    }

    size_t content_length = 0;
    char *content_length_header = http_get_header(&req, "content-length");
    if(content_length_header != NULL) {
        content_length = strtoul(content_length_header, NULL, 10);
    }

    if (content_length > 0 && headers_start != NULL) {
        req.body = bString_Init(content_length + 1, NULL);
        bString_Appends(req.body, headers_start);
    }

    int bytes_sent = SOCKET_ERROR;

    printf("Parsed %zu headers:\n", req.headers_len);
    for (size_t i = 0; i < req.headers_len; i++) {
        printf("  [%zu] '%s': '%s'\n", i, req.headers[i].key, req.headers[i].value);
    }

    if(strcmp(req.path, "/") == 0) {
        const char res[] = "HTTP/1.1 200 OK\r\n\r\n";
        bytes_sent = send(conn_fd, res, sizeof(res) -1, 0);
    } else if (strcmp(req.path, "/user-agent") == 0) {
        char *s = http_get_header(&req, "user-agent");
        if(s == NULL) s = "NULL";

        size_t slen = strlen(s);
        char *res = malloc(BUFFER_SIZE);
        if (res) {
            sprintf(res, 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: %zu\r\n\r\n"
                    "%s", 
                    slen, s);
            bytes_sent = send(conn_fd, res, (int)strlen(res), 0);
            free(res);
        }
    } else if(strncmp(req.path, "/echo/", 6) == 0) {
        char *s = req.path + 6;
        size_t slen = strlen(s);

        char *res = malloc(BUFFER_SIZE);
        if(res) {
            sprintf(res,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: %zu\r\n\r\n"
                    "%s",
                    slen, s);
            bytes_sent = send(conn_fd, res, (int)strlen(res), 0);
            free(res);
        }
    } else {
        const char res[] = "HTTP/1.1 404 Not Found\r\n\r\n";
        bytes_sent = send(conn_fd, res, sizeof(res) - 1, 0);
    }

    if(bytes_sent == SOCKET_ERROR) {
        printf("error: send() failed %d\n", WSAGetLastError());
    }

    cleanup:
        free(param);
        free(req._buffer);
        free(req.headers);

        if(req.body != NULL) {
            bString_Free(req.body);
        }

        closesocket(conn_fd);
        
        if(fp != NULL) {
            fclose(fp);
        }

        return 0;
}

void handle_connection(SOCKET conn_fd) {
    SOCKET *conn_fd_ptr = malloc(sizeof(SOCKET));
    if(conn_fd_ptr == NULL) {
        printf("error: Failed to allocate memmory for conn_fd_ptr");
        return;
    }

    *conn_fd_ptr = conn_fd;

    HANDLE thread = CreateThread(NULL, 0, _handle_connection, conn_fd_ptr, 0, NULL);

    if(thread == NULL) {
        printf("error: Failed to create thread, %lu\n", GetLastError());
        free(conn_fd_ptr);
        closesocket(conn_fd);
        return;
    }

    CloseHandle(thread);
}

char *http_get_header(HttpRequest *self, char *header) {
    assert(self != NULL);
    assert(header != NULL);

    for (size_t i = 0; i < self->headers_len; i++) {
        if(strcasecmp(header, self->headers[i].key) == 0) {
            return self->headers[i].value;
        } 
    }
    
    return NULL;
};

