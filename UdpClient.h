#define BUFLEN 512         // Max length of buffer


class UdpClient {
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	WSADATA wsa;
	char buf[BUFLEN];
	char message[BUFLEN];
	// string serverIp; 		// server ip to connect to
	// unsigned int serverPort;  // server port


  public:
	UdpClient(std::string ip, unsigned int port);
	~UdpClient();

	void send(char *inputBuffer, unsigned int bufferBytes);
};