
#define SERVER "127.0.0.1" // ip address of udp server
#define BUFLEN 512         // Max length of buffer
#define PORT 8005          // The port on which to listen for incoming data

class UdpClient {
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	WSADATA wsa;
	char buf[BUFLEN];
	char message[BUFLEN];

  public:
	UdpClient();
	~UdpClient();

	void send(char *inputBuffer, unsigned int bufferBytes);
};