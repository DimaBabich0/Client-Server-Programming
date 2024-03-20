#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

class Server
{
public:
	WSADATA wsa;
	SOCKET server_socket;
	vector <string> history;

	fd_set readfds;
	SOCKET client_socket[MAX_CLIENTS] = {};

	SOCKET new_socket;
	sockaddr_in address;
	
	int addrlen;

	int CreateServer();
	int WorkServer();
	
	void ClearFdset();
	int AcceptNewSocket();
	void PrintInfoAboutSocket();
	void AddSocketToVector();
	void GetMessageFromSocket();
};

int Server::CreateServer()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}
	
	listen(server_socket, MAX_CLIENTS);
	
	return 0;
}

int Server::WorkServer()
{
	while (true)
	{
		ClearFdset();

		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR)
		{
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		if (int i = (AcceptNewSocket() != 0))
		{
			puts("AcceptNewSocket error");
			return i;
		}

		GetMessageFromSocket();
	}

	WSACleanup();
	return 0;
}

void Server::ClearFdset()
{
	FD_ZERO(&readfds);

	FD_SET(server_socket, &readfds);

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		SOCKET s = client_socket[i];
		if (s > 0)
			FD_SET(s, &readfds);
	}
}

int Server::AcceptNewSocket()
{
	addrlen = sizeof(sockaddr_in);
	if (FD_ISSET(server_socket, &readfds))
	{
		if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0)
		{
			puts("ERROR: accept function error");
			return 5;
		}

		for (int i = 0; i < history.size(); i++)
		{
			send(new_socket, history[i].c_str(), history[i].size(), 0);
		}

		PrintInfoAboutSocket();
		AddSocketToVector();
	}
	return 0;
}

void Server::PrintInfoAboutSocket()
{
	printf("New connection. Socket FD is %d; IP is: %s; Port: %d;\n",
		new_socket,
		inet_ntoa(address.sin_addr),
		ntohs(address.sin_port));
}

void Server::AddSocketToVector()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_socket[i] == 0)
		{
			client_socket[i] = new_socket;
			printf("Adding to list of sockets at index %d\n", i);
			break;
		}
	}
}

void Server::GetMessageFromSocket()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		SOCKET s = client_socket[i];
		if (FD_ISSET(s, &readfds))
		{
			getpeername(s, (sockaddr*)&address, (int*)&addrlen);
			
			char response[DEFAULT_BUFLEN];
			int response_length = recv(s, response, DEFAULT_BUFLEN, 0);
			response[response_length] = '\n';
			response[response_length + 1] = '\0';

			string userStr = response;
			size_t pos = userStr.find('off');
			if (pos != string::npos)
			{
				cout << "Client #" << i << " is off\n";
				client_socket[i] = 0;
			}

			history.push_back(userStr);

			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (client_socket[i] != 0)
					send(client_socket[i], userStr.c_str(), userStr.size(), 0);
			}
		}
	}
}

int main()
{
	system("title Server");
	Server dlg;

	if (int i = dlg.CreateServer() != 0)
	{
		return i;
	}
	puts("Start server... DONE.");
	
	puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

	if (int i = dlg.WorkServer() != 0)
		return i;
}