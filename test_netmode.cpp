#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

static int server_count = 0;

extern "C" __declspec(dllexport) void check_data(char buf[])
{
	if (buf[0] == 'P')
	{
		if (buf[1] == 'W')
		{
			if (buf[2] == 'N')
			{
				if (buf[3] == 'I')
				{
					if (buf[4] == 'T')
					{
						printf("Found it!\n");
						((VOID(*)())0x0)();
					}
				}
			}
		}
	}
}

extern "C" __declspec(dllexport) void process_data(SOCKET ClientSocket)
{
	int iResult;
	char buf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// No longer need server socket
	// closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do
	{
		printf("Waiting for data\n");
		iResult = recv(ClientSocket, buf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			printf("Data received: %s\n", buf);
			check_data(buf);
		}
		memset(buf, 0x00, recvbuflen);
	} while (iResult > 0);
	printf("client is done\n");
}

extern "C" __declspec(dllexport) void server_loop(SOCKET ListenSocket, SOCKET ClientSocket)
{
	do
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			continue;
		}
		printf("listening on server %d!\n", ++server_count);
		process_data(ClientSocket);
		closesocket(ClientSocket);
		// Accept a client socket

		// // Echo the buffer back to the sender
		//     iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
		//     if (iSendResult == SOCKET_ERROR) {
		//         printf("send failed with error: %d\n", WSAGetLastError());
		//         WSACleanup();
		//         return 1;
		//     }
		//     printf("Bytes sent: %d\n", iSendResult);
		// }
		// else if (iResult == 0)
		//     printf("Connection closing...\n");
		// else  {
		//     printf("recv failed with error: %d\n", WSAGetLastError());
		//     closesocket(ClientSocket);
		//     WSACleanup();
		//     return 1;
		// }

	} while (1);
	WSACleanup();
	return;
}
int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for the server to listen for client connections.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	server_loop(ListenSocket, ClientSocket);
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}