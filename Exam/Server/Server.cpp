#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include "cDish.h"
#include "cOrder.h"
#include "cMyThread.h"
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

#define MAX_CLIENTS 30
#define DEFAULT_BUFLEN 4096

MyThread objThread;
vector <cDish> aDishes;
SOCKET server_socket;

void StringLowercase(string& str)
{
	for (int i = 0; i < str.size(); i++)
	{
		str[i] = tolower(str[i]);
	}
}

int main()
{
	LoadDishes(aDishes);
	objThread.StartThread();
	system("title Server");

	puts("Start server... DONE.");
	WSADATA wsa;
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

	fd_set readfds;
	SOCKET client_socket[MAX_CLIENTS] = {};
	while (true)
	{
		FD_ZERO(&readfds);

		FD_SET(server_socket, &readfds);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET s = client_socket[i];
			if (s > 0)
				FD_SET(s, &readfds);
		}

		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR)
		{
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		SOCKET new_socket;
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds))
		{
			if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0)
			{
				perror("accept function error");
				return 5;
			}

			printf("New connection, socket fd is %d, ip is: %s, port: %d\n", 
				new_socket, 
				inet_ntoa(address.sin_addr), 
				ntohs(address.sin_port));

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

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET socket = client_socket[i];
			if (FD_ISSET(socket, &readfds))
			{
				getpeername(socket, (sockaddr*)&address, (int*)&addrlen);

				char aMessage[DEFAULT_BUFLEN];
				int i = recv(socket, aMessage, DEFAULT_BUFLEN, 0);
				aMessage[i] = '\0';

				string szMessage = aMessage;
				if (szMessage == "off")
				{
					cout << "Client #" << i << " is off\n";
					client_socket[i] = 0;
				}

				//lowercase message
				StringLowercase(szMessage);

				float fPrice = 0.f;
				int dTime = 0;
				for (int i = 0; i < aDishes.size(); i++)
				{
					//lowercase dish name
					string szDishName = aDishes[i].szDishName;
					StringLowercase(szDishName);

					//check for dish
					size_t pos = szMessage.find(szDishName);
					if (pos != string::npos)
					{
						fPrice += aDishes[i].fPrice;
						dTime += aDishes[i].dTime;
					}
				}

				if (dTime == 0)
				{
					char szAnswer[DEFAULT_BUFLEN];
					sprintf(szAnswer, "Sorry, but you write wrong order. Try again");
					send(socket, szAnswer, strlen(szAnswer), 0);
					sprintf(szAnswer, "begin");
					send(socket, szAnswer, strlen(szAnswer), 0);
					continue;
				}

				char szAnswer[DEFAULT_BUFLEN];
				sprintf(szAnswer,
					"Your order is accepted\nOrder cost: %.2f UAH\nCooking time: %d seconds\nPlease wait",
					fPrice, dTime);
				cOrder obj(socket, fPrice, dTime, szAnswer);
				objThread.cOrder.push_back(obj);

				if (objThread.cOrder.size() > 1)
				{
					sprintf(szAnswer,
						"Sorry, but another order is currently being processed. Please wait.");
					send(socket, szAnswer, strlen(szAnswer), 0);
				}
			}
		}
	}

	WSACleanup();
}