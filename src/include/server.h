#ifndef SERVER_H
#define SERVER_H

#include <assert.h>
#include <stddef.h>   
#include <stdbool.h>
#include <string.h>

#define BSTRING_INIT_CAPACITY 16
#define BUFFER_SIZE 1024
#define MAX_HEADERS 128

// Expandable string declarations
typedef struct BString {
    char* data;
    size_t length;
    size_t capacity;
} BString;

extern BString *filename;
BString *bString_Init(size_t capacity, const char *const s);
bool bString_Appends(BString *self, const char *const s);
void bString_Free(BString *self);

// HTTP Server related declarations
typedef enum HttpMethodTyp {
    HTTP_UNKNOWN = 0,
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE,
} HttpMethodTyp;

typedef enum ServerStatus {
    SERVER_OFFLINE = 0,
    SERVER_RUNNING,
} ServerStatus;

typedef struct HttpMethodMap {
    const char *const str;
    HttpMethodTyp typ;
} HttpMethodMap;

typedef struct HttpHeader {
  char *key;
  char *value;
} HttpHeader;

typedef struct HttpRequest {
    HttpMethodTyp method;
    char *path;

    HttpHeader *headers;
    size_t headers_len;

    char *_buffer;
    size_t _buffer_len;

    BString *body;
} HttpRequest;

typedef struct ServerContext {
    struct sockaddr_in addr;
    int addr_len;
    SOCKET listen_socket;
    ServerStatus status;
    BString *filename;
} ServerContext;

typedef struct ClientContext {
    SOCKET socket;
    struct sockaddr_in addr;
} ClientContext;

extern const char HTTP_VERSION[];
extern const HttpMethodMap KNOWN_HTTP_METHODS[];
extern size_t KNOWN_HTTP_METHODS_LEN;

// HTTP related Function declaration
int setupSaServer(const char *host, const char *port, struct sockaddr_in *out_addr, int *out_addr_len);
int bind_and_listen(ServerContext *sctx);
int handle_request(ServerContext *sctx);

HttpRequest* HttpRequest_Parse(const char *raw_request);
void HttpRequest_Free(HttpRequest *req);
int do_GET(const struct HttpHeader *self, char *header);
int do_POST(const struct HttpHeader *self);
int do_PUT(const struct HttpHeader *self);
int do_PATCH(const struct HttpHeader *self);
int do_DELETE(const struct HttpHeader *self);
#endif

