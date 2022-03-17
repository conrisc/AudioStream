/*
    Simple udp client
*/
#include <stdio.h>
#include <winsock2.h>
#include <iostream>

#include "UdpClient.h"

using namespace std;


UdpClient::UdpClient(string ip, unsigned int port) {
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
	si_other.sin_port = htons(port);
	char *ipC = &ip[0];
	si_other.sin_addr.S_un.S_addr = inet_addr(ipC);
}

UdpClient::~UdpClient() {
	cout << "Deconstructing UdpClient" << endl;
	closesocket(s);
	WSACleanup();
}

void UdpClient::send(std::span<const char> data) {
	// cout << "Sending data of length: "<< bufferBytes << endl;
    if (sendto(s, data.data(), data.size_bytes(), 0, (struct sockaddr *)&si_other, slen) == SOCKET_ERROR) {
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
