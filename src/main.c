#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "server.h"

int main(int argc, char **argv) {
	WSADATA wsaData;
	ServerContext sctx = {0};
	if (argc > 2) {
    	sctx.filename = bString_Init(0, argv[2]);
    	if(argv[2][strlen(argv[2]) - 1] != '/') {
       		bString_Appends(sctx.filename, "/");
    	}
	} else {
    	sctx.filename = bString_Init(0, "./public/");
	}

	int _wsaStart = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(_wsaStart != 0) {
		printf("error: Initialization failed.%d\n", WSAGetLastError());
		return 1;
	}

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	sctx.addr_len = sizeof(struct sockaddr_in);

	if (setupSaServer("127.0.0.1", "50000", &sctx.addr, &sctx.addr_len) != 0) {
 	   	WSACleanup();
		return 1;
	}

	if (bind_and_listen(&sctx) != 0) {
		WSACleanup();
		return 1;
	}

	if (sctx.filename) bString_Free(sctx.filename);
	WSACleanup();
	return 0;
}