#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "cDish.h"
using namespace std;
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096
#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

vector <cDish> aDishes;
SOCKET client_socket;
bool bCanSendOrder = TRUE;

DWORD WINAPI Sender(void* param)
{
    LoadDishes(aDishes);
    PrintDishes(aDishes);
    while (true)
    {
        if (bCanSendOrder)
        {
            cout << "Enter your order:\n";

            string query;
            getline(cin, query);

            send(client_socket, query.c_str(), query.size(), 0);
            bCanSendOrder = FALSE;
        }
    }
}

DWORD WINAPI Receiver(void* param)
{
    char szMessageStop[] = "stop";
    char szMessageBegin[] = "begin";

    while (true)
    {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';
        
        if (strcmp(response, szMessageStop) == 0)
            bCanSendOrder = FALSE;
        else if (strcmp(response, szMessageBegin) == 0)
            bCanSendOrder = TRUE;
        else
            cout << "\n" << response << "\n";
    }
}

int main()
{
    system("title Client");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (client_socket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }

        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }

    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
}