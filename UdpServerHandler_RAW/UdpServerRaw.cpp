/*
 * Creator: Ronnie Wong
 * Date: January 16, 2018
 * Board IP = 192.168.1.10
 * CPU IP   = 192.168.1.11
 * Listen Port = 11
 * Send   Port = 10
 * Terminology:
 * Server (aka CPU) - Listens to incoming data being sent from Client [CPU -> Picozed]
 * Client (aka Picozed) - Sends out data to Server [Picozed -> CPU]
 */

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN for WINSOCK2

#define UNICODE 1
#define _UNICODE 1

#include <stdlib.h>
#include <stdio.h> 
#include <WinSock2.h>
#include <WS2tcpip.h>				// not used here since not TCP/IP <can delete>
#include <Windows.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <winioctl.h>
#include <iphlpapi.h>
#include <fstream>
#include <tchar.h>
#include <strsafe.h>
#include <winioctl.h>
#include <iphlpapi.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")	// Winsock Library

#define BUFLEN 512					// Max length of buffer
#define PORT 12						// The port on which to listen to for incoming data
#define SEND_PORT 11				// Send to Picozed
#define LISTEN_PORT 12				// Receive from Picozed
#define INITIAL_SEND_BYTE 17

// namespace
using namespace std;

// Function Protoypes
void swap(char *firstElem, char *secondElem);


DWORD WINAPI CheckEscape(LPVOID lpParam) {
	while (GetAsyncKeyState(VK_ESCAPE) == 0) {
		//sleep 
		Sleep(10);
	}
	exit(0);

}


int main()
{

	CreateThread(NULL, 0, CheckEscape, NULL, 0, NULL);

	WSADATA WSAData;
	SOCKET server, client;
	SOCKADDR_IN serverAddr, clientAddr;
	BOOL UDPSocketOption = TRUE;
	int UDPSocketOptionLen = sizeof(BOOL);

	int iResult;
	char buffer[BUFLEN];							// set to 512 Bytes for SD CARD Access
	int clientAddrSize = sizeof(clientAddr);

	int NumWordPairs = BUFLEN / 4;
	DWORD data[108] = { 0 };		// 432/4 = 108 (4 Bytes x 8 bits/byte = 32 bits)
	int track;
	int wrapSize = 4;

	TCHAR szBuffer[256];
	char DataBuffer[BUFLEN];
	DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;

	DWORD dwBytesToWriteETH;
	DWORD dwBytesWrittenETH;

	// Create/Open Data File to be Written
	HANDLE hFile;
	hFile = CreateFile(L"\\\\.\\I:\\WriteBinaryData_02142018.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// hFile = CreateFile(...,...,...,FILE_ATTRIBUTE_NORMAL,...);
	// or use: FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Terminal failure: Unable to open file for writing.\n\r");
		printf("Press enter to exit()\n\r");
		getchar();
		return 1;
	}

	// Initialize Winsock
	printf("Initializing Winsock...\n\r");
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}

	// Send handshake activation
	/* Create Send [to Picozed] SOCKET */
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == INVALID_SOCKET) {
		printf("Client socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}

	// Define client sock structure
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr("192.168.1.10");
	clientAddr.sin_port = htons(SEND_PORT);

	char startByte[1] = { 0x40 };
	int startByteSent;
	printf("Sending 0x%X to begin transmission\n\r", startByte[0]);
	printf("Press key when ready...\n\r");
	getchar();
	//startByteSent = send(client, startByte, (int)sizeof(startByte), 0);
	startByteSent = sendto(client, startByte, 1, 0, (SOCKADDR *) & clientAddr, sizeof(clientAddr));
	if (startByteSent == SOCKET_ERROR){
		printf("Send failed with error: %d\n", WSAGetLastError());
		closesocket(client);
		WSACleanup();
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}
	else {
		printf("Send successful.\n\r");
		closesocket(client);
	}



	// Open socket for server side:
	printf("Opening server socket.......");
	//won't work if using SOCK_RAW
	//if ((server = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == INVALID_SOCKET) {
	if ((server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("Could not create RAW socket: %d\r\n", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		WSACleanup();
		return 1;
	}
	printf("opened.\n\r");



	// Bind server socket to address
	printf("Binding socket to port: %d\r\n", PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);			// htonl(INADDR_ANY) or inet_addr("192.168.1.10") "172.16.3.181"
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(LISTEN_PORT);
	if (bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("Bind failed: %d\r\n", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		return 1;
	}
	printf("Bind successful.\n\r");

	// Begin recording...
	printf("Press any key to begin recording incoming connections...\r\n");
	getchar();
	printf("Recording...\n\r");

	// Enter listening FOR-LOOP state...
	//while (true) {
	// Write to SD CARD
	dwBytesToWriteETH = (DWORD)(BUFLEN);
	clock_t begin = clock();
	for(int km = 0; km < 600000; km++){
		
		dwBytesWrittenETH = 0;
		if (recv(server, buffer, sizeof(buffer), 0) > 0) {

			// 'just to be friendly instead of waiting forever....
			if (km == 0) printf("Record 0 saved.\n\r");
			if (km == 300000) printf("Record %d saved.\n\r",km);
			if (km == 550000) printf("Almost done...at record %d.\n\r", km);

			bErrorFlag = WriteFile(hFile, buffer, dwBytesToWriteETH, &dwBytesWrittenETH, NULL);
			if (bErrorFlag == FALSE) {
				printf("Terminal failure: Unable to write to file.\n\r");
			}

			// clear data buffer
			for (int i = 0; i < BUFLEN; i++) {
				buffer[i] = 0;
			}
		}
		else {
			printf("SOCKET ERROR: %d\r\n", WSAGetLastError());
			printf("Press any key to exit()\r\n");
			getchar();
			WSACleanup();
			return 1;
		}
	}
	clock_t end = clock();
	closesocket(server);
	WSACleanup();

	unsigned long time_spent = (unsigned long)(end - begin) / CLOCKS_PER_SEC;

	printf("Time elapsed is: %lu\n\r", time_spent);

	if (!CloseHandle(hFile)){
		printf("CloseHandle failed.\n\r");
	}

	printf("Press any key to exit()\n\r");
	getchar();

	return 0;
}


// Functions
void swap(char *firstElem, char *secondElem)
{
	char temp = *firstElem;
	*firstElem = *secondElem;
	*secondElem = temp;
}