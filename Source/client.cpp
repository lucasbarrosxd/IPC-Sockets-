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
SOCKET client_socket;
sockaddr_in server_address;
char msg[MAX_MSG_SIZE];

// Functions.
unsigned short itous(unsigned int);
std::string parse_net_ip(unsigned long);

int main() {
	/* Initialize winsock ------------------------------------------------------------------------------------------ */
	std::cout << "Initializing winsock...";

	// Request winsock library.
	if(WSAStartup(WSA_MAXVER, &wsa_data) != 0) {
		std::cerr << " Winsock initialization failed with error code " << WSAGetLastError() << "." << std::endl;
		ExitProcess(EXIT_FAILURE);
	}

	// Check if library version is sufficient.
	if(wsa_data.wVersion < WSA_MINVER) {
		std::cerr << " Winsock version " << HIBYTE(wsa_data.wVersion) << "." << LOBYTE(wsa_data.wVersion)
			<< " not supported." << std::endl;
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Successfully initialized winsock with version "
		<< (int) HIBYTE(wsa_data.wVersion) << "." << (int) LOBYTE(wsa_data.wVersion) << "." << std::endl;
	/* Resolver server address ----------------------------------------------------------------------------------------- */
	// Define address type as IPV4.
	server_address.sin_family = AF_INET;

	// Receive server's IP address and process it until a valid one is received.
	std::string ip_input;
	while(true) {
		std::cout << "Server IPv4 address: ";
		std::cin >> ip_input;

		// Check for cancellation.
		if(ip_input == "exit") {
			std::cout << std::endl << "Exiting..." << std::endl;
			WSACleanup();
			ExitProcess(EXIT_SUCCESS);
		}

		if(inet_pton(AF_INET, ip_input.c_str(), &server_address.sin_addr.S_un.S_addr) == 1) {
			std::cout << "Valid IP address received." << std::endl;
			break;
		} else {
			std::cout << "Not a valid IPv4 address or command. ";
		}
	}

	// Receive server's port number and process it until a valid one is received.
	std::string port_input;
	while(true) {
		std::cout << "Server port number: ";
		std::cin >> port_input;

		// Check for cancellation.
		if(port_input == "exit") {
			std::cout << std::endl << "Exiting..." << std::endl;
			WSACleanup();
			ExitProcess(EXIT_SUCCESS);
		}

		try {
			// Convert port number to big-endian.
			server_address.sin_port = htons(itous(std::stoi(port_input)));
			std::cout << "Valid port number received." << std::endl;
			break;
		} catch(std::invalid_argument) {
			std::cerr << "A port must have numeric digits only. ";
		} catch(std::range_error) {
			std::cerr << "A port must be an integer between 0 and 65535. ";
		}
		
	}

	/* Create socket --------------------------------------------------------------------------------------------------- */
	std::cout << "Creating socket...";

	// Set socket object to default value and create TCP/IPv4 socket.
	client_socket = INVALID_SOCKET;
	if((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		std::cerr << "Failed to create socket with error code " << WSAGetLastError() << "." << std::endl;
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Socket created successfully." << std::endl;
	/* Connect to the server ------------------------------------------------------------------------------------------- */
	std::cout << "Connecting to server...";

	if(connect(client_socket, (sockaddr*) &server_address, sizeof(server_address) == SOCKET_ERROR)) {
		std::cerr << " Failed to connect to server at " << parse_net_ip(server_address.sin_addr.S_un.S_addr)
			<< ":" << ntohs(server_address.sin_port) << " with error code " << WSAGetLastError() << "." << std::endl;
		closesocket(client_socket);
		WSACleanup();
		ExitProcess(EXIT_FAILURE);
	}

	std::cout << " Connection established successfully with server at address " <<
		parse_net_ip(server_address.sin_addr.S_un.S_addr) << ":" << ntohs(server_address.sin_port) << "." << std::endl;

	/* Message loop ---------------------------------------------------------------------------------------------------- */
	while(true) {
		// Read user input.
		std::cout << ">> ";
		std::cin >> msg;

		// Process user input.
		if(msg == "exit") {
			break;
		}

		// Send message.
		if(send(client_socket, msg, strlen(msg), 0) == SOCKET_ERROR) {
			std::cerr << "Failed to send message." << std::endl;
		}
	}

	/* Disconnect & Cleanup -------------------------------------------------------------------------------------------- */
	// Shutdown socket with SD_SEND notifying the server that the client is disconnecting.
	if(shutdown(client_socket, SD_SEND) == SOCKET_ERROR) {
		std::cerr << "Failed to notify server about disconnection." << std::endl;
	} else {

	}

	closesocket(client_socket);
	WSACleanup();

	ExitProcess(EXIT_SUCCESS);
}

unsigned short itous(unsigned int number) {
	if(number > 65535) {
		// Outside ushort boundaries.
		throw std::range_error("uint over ushort boundaries");
	}

	return (unsigned short) number;
}

std::string parse_net_ip(unsigned long ip) {
	return std::to_string(LOBYTE(LOWORD(ip)))
		+ "." + std::to_string(HIBYTE(LOWORD(ip)))
		+ "." + std::to_string(LOBYTE(HIWORD(ip)))
		+ "." + std::to_string(HIBYTE(HIWORD(ip)));
}
