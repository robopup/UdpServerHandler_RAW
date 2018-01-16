/*
 * Creator: Ronnie Wong
 * Date: January 16, 2018
 * Summary: UDP Raw Socket Server Code
 * 
 */

#include <stdlib.h>
#include <stdio.h> 
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")	// Winsock Library

#define BUFLEN 512	// Max length of buffer
#define PORT 8888	// The port on which to listen to for incoming data

int main()
{
	WSADATA WSAData;
	SOCKET server;
	SOCKADDR_IN servAddr, clientAddr;

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


	printf("Press any key to exit()\n\r");
	getchar();

	return 0;
}