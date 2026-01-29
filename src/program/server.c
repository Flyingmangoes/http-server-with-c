#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

/*
----------------------------------------------------
1. Address Resolution (setupSaServer)
----------------------------------------------------
    - Resolves a host and port into an IPv4 socket address.
    - Uses getaddrinfo() to perform DNS / address lookup.
    - The resulting addrinfo is copied into a sockaddr_in
    structure owned by the caller.
    - Also outputs the size of the sockaddr structure.
    - This step ONLY prepares the address; no socket is
    created or bound here.

----------------------------------------------------
2. Socket Creation and Binding (bind_and_listen)
----------------------------------------------------
    - Creates a TCP listening socket (AF_INET, SOCK_STREAM).
    - Enables SO_REUSEADDR to allow quick server restarts.
    - Binds the socket to the resolved server address.
    - Transitions the socket into listening mode via listen().
    - Once listen() succeeds, the server is ready to accept
    incoming connections.

----------------------------------------------------
3. Connection Acceptance Loop (handle_request)
----------------------------------------------------
    - Enters an infinite loop waiting for client connections.
    - accept() blocks until a client connects.
    - Each successful accept() returns a new socket that
    represents a single client connection.
    - The current implementation immediately closes the
    client socket after logging the connection.
    - No HTTP parsing or request handling is performed yet.
*/

int setupSaServer(const char* host, const char *port, struct sockaddr_in *out_addr, int *out_addr_len) {
    struct addrinfo hints;
    struct addrinfo *result = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    int get_addrinfo = getaddrinfo(host, port, &hints, &result);
    if(get_addrinfo != 0) {
        printf("Failed to get addrinfo: %d\n", get_addrinfo);
        return -1;
    }

    memcpy(out_addr, result->ai_addr, sizeof(struct sockaddr_in));
    *out_addr_len = sizeof(struct sockaddr_in);

    freeaddrinfo(result);
    return 0;
};

int bind_and_listen(ServerContext *sctx) {
    sctx->listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (sctx->listen_socket == INVALID_SOCKET) {
            printf("error: Invalid Socket\n");
            return -1;
    }

    int reuse = 1;
    if (setsockopt(sctx->listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) < 0) {
        printf("error: SO_REUSEADDR failed, %d\n", WSAGetLastError());
        return -1;
    }

    if (bind(sctx->listen_socket, (struct sockaddr *) &sctx->addr, sizeof(struct sockaddr_in)) ==  SOCKET_ERROR) {
        printf("error: Failed to bind Socket, %d\n", WSAGetLastError());
        closesocket(sctx->listen_socket);
        sctx->listen_socket = INVALID_SOCKET;
        return -1;
    }

    if (listen(sctx->listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("error: Listen failed, \n", WSAGetLastError());
        closesocket(sctx->listen_socket);
        sctx->listen_socket = INVALID_SOCKET;
        return -1;
    }
    

    printf("Server Listening . . .\n");
    
    handle_request(sctx);

    return 0;
};

int handle_request(ServerContext *sctx) {
    while(1){
        SOCKET conn_fd = accept(sctx->listen_socket, (struct sockaddr *) &sctx->addr, &sctx->addr_len);
        if(conn_fd == INVALID_SOCKET) {
            printf("error: Accept failed, %d\n", WSAGetLastError());
            continue;
        }
        printf("Client connected on socket %llu\n", (unsigned long long)conn_fd);
        
        closesocket(conn_fd);
    }

    return 0;
};

