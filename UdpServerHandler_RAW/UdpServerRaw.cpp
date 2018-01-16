/*
 * Creator: Ronnie Wong
 * Date: January 16, 2018
 * Summary: UDP Raw Socket Server Code
 * 
 */

#include <stdlib.h>
#include <stdio.h> 
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib,"ws2_32.lib")	// Winsock Library

#define BUFLEN 512	// Max length of buffer
#define PORT 8888	// The port on which to listen to for incoming data

int main()
{
	WSADATA WSAData;
	SOCKET server;
	SOCKADDR_IN servAddr, clientAddr;
	BOOL UDPSocketOption = TRUE;
	int UDPSocketOptionLen = sizeof(BOOL);

	// Initialize Winsock
	printf("Initializing Winsock...\n\r");
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}

	// Open socket
	printf("Creating RAW socket...\n\r");
	if ((server = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("Could not create RAW socket: %d\r\n", WSAGetLastError);
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}

	// SetSockOpt function for UDP options
	// See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms740525(v=vs.85).aspx for more info
	// this option is not available in Windows 10 (only for Windows 7 and below)
	/*
	printf("Setting SetSockOpt to True...\n\r");
	if ((setsockopt(server, IPPROTO_UDP, UDP_NOCHECKSUM, (char *)&UDPSocketOption, UDPSocketOptionLen)) != 0) {
		printf("Could not SetSockOpt: %d\r\n", WSAGetLastError);
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}
	*/





	printf("Press any key to exit()\n\r");
	getchar();

	return 0;
}