/*
 * Creator: Ronnie Wong
 * Date: January 16, 2018
 * Summary: UDP Raw Socket Server Code
 * Protocol: RFC 768 - Link: https://tools.ietf.org/html/rfc768
 * Format
 *  0       7 8       15 16       23 24       31
 * +---------+----------+-----------+-----------+
 * |    Source Port     |    Destination Port   |
 * +---------+----------+-----------+-----------+
 * |      Length        |       Checksum        |
 * +---------+----------+-----------+-----------+
 * |         data octets.....
 * +-----------------.....
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <stdio.h> 
#include <WinSock2.h>
#include <WS2tcpip.h>				// not used here since not TCP/IP <can delete>
#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <winioctl.h>
#include <iphlpapi.h>



#pragma comment(lib,"ws2_32.lib")	// Winsock Library

#define BUFLEN 512	// Max length of buffer
#define PORT 11000	// The port on which to listen to for incoming data

using namespace std;

int main()
{
	WSADATA WSAData;
	SOCKET server;
	SOCKADDR_IN serverAddr, clientAddr;
	BOOL UDPSocketOption = TRUE;
	int UDPSocketOptionLen = sizeof(BOOL);
	char buffer[BUFLEN];
	int clientAddrSize = sizeof(clientAddr);

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
	//won't work if using SOCK_RAW
	//if ((server = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == INVALID_SOCKET) {
	if ((server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("Could not create RAW socket: %d\r\n", WSAGetLastError());
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
		printf("Could not SetSockOpt: %d\r\n", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}
	*/

	// Bind socket to address
	printf("Binding socket to port: %d\r\n", PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(11000);
	if (bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("Bind failed: %d\r\n", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		return 1; 
	}
	printf("Bind successful. Listening for incoming connections...\r\n");

	// Enter listening WHILE-LOOP state...
	while (1) {
		if (recv(server, buffer, sizeof(buffer), 0) > 0) {
			printf("Buffer size is: %d\r\n", sizeof(buffer));
			printf("Client connected.\n\r");
			cout << "Client says: " << buffer << endl;
		}
		else {
			printf("SOCKET ERROR: %d\r\n", WSAGetLastError());
			printf("Press any key to exit()\r\n");
			getchar();
			return 1;
		}
	}

	printf("Press any key to exit()\n\r");
	getchar();

	return 0;
}