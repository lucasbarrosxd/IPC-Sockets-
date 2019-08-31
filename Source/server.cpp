#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

// Constants.
const WORD WSA_MAXVER = MAKEWORD(2, 2);
const WORD WSA_MINVER = MAKEWORD(2, 0);
const std::size_t MAX_MSG_SIZE = 100;

// Global variables.
WSADATA wsa_data;
SOCKET server_socket;
sockaddr_in server_address;
SOCKET client_socket;
sockaddr_in client_address;
char msg[MAX_MSG_SIZE];

// Functions
std::string parse_net_ip(unsigned long);

int main() {
	/* Initialize winsock ------------------------------------------------------------------------------------------ */
	std::cout << "Initializing winsock...";

	// Request winsock library.
	if (WSAStartup(WSA_MAXVER, &wsa_data) != 0) {
		std::cerr << " Winsock initialization failed with error code " << WSAGetLastError() << "." << std::endl;
		ExitProcess(EXIT_FAILURE);
	}

	// Check if library version is sufficient.
	if (wsa_data.wVersion < WSA_MINVER) {
		std::cerr << " Winsock version " << HIBYTE(wsa_data.wVersion) << "." << LOBYTE(wsa_data.wVersion)
			<< " not supported." << std::endl;
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Successfully initialized winsock with version "
		<< (int) HIBYTE(wsa_data.wVersion) << "." << (int) LOBYTE(wsa_data.wVersion) << "." << std::endl;
	/* Create socket --------------------------------------------------------------------------------------------------- */
	std::cout << "Creating socket...";

	// Set socket object to default value and create TCP/IPv4 socket.
	server_socket = INVALID_SOCKET;
	if((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		std::cerr << " Failed to create socket with error code " << WSAGetLastError() << "." << std::endl;
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Socket created successfully." << std::endl;
	/* Bind socket ----------------------------------------------------------------------------------------------------- */
	std::cout << "Binding server to address...";

	// Define address type as IPv4.
	server_address.sin_family = AF_INET;

	// Allow connections only through any IP address.
	server_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// Let the computer choose an available port.
	server_address.sin_port = htons(0);

	if(bind(server_socket, (sockaddr*) &server_address, sizeof(server_address)) == SOCKET_ERROR) {
		std::cerr << " Failed to bind server to an address with error code " << WSAGetLastError() << "." << std::endl;
		WSACleanup();
		closesocket(server_socket);
		ExitProcess(EXIT_FAILURE);
	}

	// Retrieve server port.
	int server_addrlen = sizeof(server_address);
	if(getsockname(server_socket, (sockaddr*)&server_address, &server_addrlen) == SOCKET_ERROR) {
		std::cerr << " Failed to retrieve server address with error code " << WSAGetLastError() << "." << std::endl;
		WSACleanup();
		closesocket(server_socket);
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Successfully bound server to local address on port "
		<< ntohs(server_address.sin_port) << "." << std::endl;
	/* Set socket to listening ------------------------------------------------------------------------------------- */
	std::cout << "Starting socket...";

	// Set socket in listening mode.
	if(listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Failed to start socket with error code " << WSAGetLastError() << "." << std::endl;
		closesocket(server_socket);
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Socket started." << std::endl;
	/* Wait for clients -------------------------------------------------------------------------------------------- */
	std::cout << "Waiting for clients...";

	// Create a new connection when a client connects.
	client_socket = INVALID_SOCKET;
	int client_addrlen = sizeof(client_address);
	if((client_socket = accept(server_socket, (sockaddr*) &client_address, &client_addrlen)) == INVALID_SOCKET) {
		std::cerr << " Failed to connect to client with error code " << WSAGetLastError() << "." << std::endl;
		closesocket(server_socket);
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Successfully connected to client with local address " <<
		parse_net_ip(client_address.sin_addr.S_un.S_addr) << ":" << ntohs(client_address.sin_port) << "." << std::endl;
	/* Message loop ---------------------------------------------------------------------------------------------------- */
	std::cout << "Listening for client messages." << std::endl;
	
	while(true) {
		int recv_return;
		if((recv_return = recv(client_socket, msg, MAX_MSG_SIZE, 0)) == SOCKET_ERROR) {
			std::cerr << "Failed to received message with error code " << WSAGetLastError() << "." << std::endl;
			closesocket(client_socket);
			closesocket(server_socket);
			WSACleanup();
			ExitProcess(EXIT_FAILURE);
		} else if(recv_return == 0) {
			// Client indicated connection closing.
			break;
		} else {
			// Message received.
			std::cout << "> " << msg << std::endl;
		}
	}

	/* Disconnect & Cleanup -------------------------------------------------------------------------------------------- */
	std::cout << "Received disconnect from client. Disconnecting...";
	
	if(shutdown(client_socket, SD_SEND) == SOCKET_ERROR) {
		std::cerr << " Failed to disconnect with error code " << WSAGetLastError() << "." << std::endl;
	} else {
		std::cout << " Disconnected successfully." << std::endl;
	}

	std::cout << "Closing sockets...";

	closesocket(client_socket);
	closesocket(server_socket);
	WSACleanup();

	std::cout << " Finished. Exiting application..." << std::endl;

	ExitProcess(EXIT_SUCCESS);
}

std::string parse_net_ip(unsigned long ip) {
	return std::to_string(LOBYTE(LOWORD(ip)))
		+ "." + std::to_string(HIBYTE(LOWORD(ip)))
		+ "." + std::to_string(LOBYTE(HIWORD(ip)))
		+ "." + std::to_string(HIBYTE(HIWORD(ip)));
}
