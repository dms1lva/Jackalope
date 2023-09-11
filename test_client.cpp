#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
int __cdecl main(void)
{
    static struct sockaddr_in si_other;
    static int slen = sizeof(si_other);
    static WSADATA wsa;
    SOCKET s;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        printf("WSAStartup failed. Error Code : %d", WSAGetLastError());

    // setup address structure
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(27015);
    si_other.sin_addr.S_un.S_addr = inet_addr((char *)"127.0.0.1");

    /* In case of TCP we need to open a socket each time we want to establish
     * connection. In theory we can keep connections always open but it might
     * cause our target behave differently (probably there are a bunch of
     * applications where we should apply such scheme to trigger interesting
     * behavior).
     */
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == SOCKET_ERROR)
        printf("socket() failed with error code : %d", WSAGetLastError());

    // Connect to server.
    if (connect(s, (SOCKADDR *)&si_other, slen) == SOCKET_ERROR)
        printf("connect() failed with error code : %d", WSAGetLastError());

    // Send our buffer
    if (send(s, "PWNIT", 6, 0) == SOCKET_ERROR)
        printf("send() failed with error code : %d", WSAGetLastError());
}