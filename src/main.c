#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "server.h"

int main(void) {
	WSADATA wsaData;
	int _wsaStart = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(_wsaStart != 0) {
		printf("error: Initialization failed.\n", WSAGetLastError());
		return 1;
	}
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	ServerContext sctx = {0};
	sctx.addr_len = sizeof(struct sockaddr_in);

	if (setupSaServer("127.0.0.1", "50000", &sctx.addr, &sctx.addr_len) != 0) {
 	   	WSACleanup();
		return 1;
	}

	if (bind_and_listen(&sctx) != 0) {
		WSACleanup();
		return 1;
	}

	WSACleanup();
	return 0;
}