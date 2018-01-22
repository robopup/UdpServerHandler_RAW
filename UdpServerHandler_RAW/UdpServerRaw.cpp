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

#define BUFLEN 432	// Max length of buffer
#define PORT 12	// The port on which to listen to for incoming data

// Function Protoypes
void swap(char *firstElem, char *secondElem);

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
	int NumWordPairs = BUFLEN/4;
	DWORD data[108] = { 0 };
	int track;
	int wrapSize = 4;

	TCHAR szBuffer[256];
	char DataBuffer[512];
	DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;

	DWORD dwBytesToWriteETH;
	DWORD dwBytesWrittenETH;

	// Create File to be Written
	HANDLE hFile;
	hFile = CreateFile(L"\\\\.\\I:\\WriteBinaryData.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
	serverAddr.sin_port = htons(PORT);
	if (bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("Bind failed: %d\r\n", WSAGetLastError());
		printf("Press any key to exit()\n\r");
		getchar();
		return 1; 
	}
	printf("Bind successful. Listening for incoming connections...\r\n");

	// Enter listening FOR-LOOP state...
	//while (true) {
	// Write to SD CARD
	dwBytesToWriteETH = (DWORD)(BUFLEN);
	clock_t begin = clock();
	for(int km = 0; km < 100000; km++){
		dwBytesWrittenETH = 0;
		if (recv(server, buffer, sizeof(buffer), 0) > 0) {

			// buffer has 432 bytes of data
			// need to pad these before pushing out in 512 byte blocks

			//printf("dwBytesToWriteETH: %d", dwBytesToWriteETH);
			//getchar();

			bErrorFlag = WriteFile(hFile, buffer, dwBytesToWriteETH, &dwBytesWrittenETH, NULL);
			if (bErrorFlag == FALSE) {
				printf("Terminal failure: Unable to write to file.\n\r");
			}

			/* <- comment/uncomment
			// Swap and rearrange the buffer
			// Rearranges little endian -> big endian format
			for (int i = 0; i < 432; i+=4) {
				swap(buffer[i + 0], buffer[i + 3]);
				swap(buffer[i + 1], buffer[i + 2]);
			}
			
			// Rearrange into columns
			// Count	Time	ZC#1	ZC#2	ZC#3	ZC#4
			// -----	----	----	----	----	----
			int track = 0;
			int skipline = 0;
			for (int k = 0; k < BUFLEN; k += 4) {
				for (int m = 0; m < wrapSize; m++) {
					data[track] = (data[track] << 8) | (unsigned char)buffer[m + k];
				}
				printf("%10d", data[track]);
				skipline++;
				track++;
				if (skipline == 6) {
					skipline = 0;
					printf("\r\n");
				}
			}

			int skipline = 0;
			for (int j = 0; j < BUFLEN; j++) {
				printf("%02X ", (unsigned char)(buffer[j]));
				skipline++;
				if (skipline == 4) {
					printf("\r\n");
					skipline = 0;
				}
			}

			printf("\r\n");
			*/ // comment/uncomment
		}
		else {
			printf("SOCKET ERROR: %d\r\n", WSAGetLastError());
			printf("Press any key to exit()\r\n");
			getchar();
			return 1;
		}
	}
	clock_t end = clock();

	unsigned long time_spent = (unsigned long)(end - begin) / CLOCKS_PER_SEC;

	printf("Time elapsed for 51.2MB is: %lu\n\r", time_spent);

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