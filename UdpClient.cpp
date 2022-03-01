/*
    Simple udp client
*/
#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include "UdpClient.h"

using namespace std;

typedef signed short MY_TYPE;

UdpClient::UdpClient() {
	cout << "Constructing UdpClient" << endl;

	// Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	// create socket
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == SOCKET_ERROR) {
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
}

UdpClient::~UdpClient() {
	cout << "Deconstructing UdpClient" << endl;
	closesocket(s);
	WSACleanup();
}

void UdpClient::send(void *inputBuffer, unsigned int bufferBytes) {
	// MY_TYPE *bufferData = (MY_TYPE *)inputBuffer;
    // printf("Enter message : ");
    // gets(message);
    // send the message
	char *msg2 = (char *)inputBuffer;
	cout << "Sending data of length: "<< bufferBytes << endl;
    if (sendto(s, msg2, bufferBytes, 0, (struct sockaddr *)&si_other, slen) == SOCKET_ERROR) {
        printf("sendto() failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

// UdpClient::receive() {
//     memset(buf, '\0', BUFLEN);
//     // try to receive some data, this is a blocking call
//     if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen) == SOCKET_ERROR) {
//         printf("recvfrom() failed with error code : %d", WSAGetLastError());
//         exit(EXIT_FAILURE);
//     }

//     puts(buf);
// }
