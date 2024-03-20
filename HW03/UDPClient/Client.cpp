#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#pragma comment (lib, "Ws2_32.lib")
using namespace std;

#define DEFAULT_BUFLEN 4096
#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

class Client
{
public:
    SOCKET client_socket;
    WSADATA wsaData;
    addrinfo hints;
    addrinfo* result;
    addrinfo* ptr;

    string color, nickname;

    int CreateClient();
    int InitClient();
    void EraseLine();
    void EnterInfo();
};

DWORD WINAPI Sender(LPVOID lp)
{
	Client* dlg = (Client*)lp;
    
    while (true) 
    {
        string str;
        getline(cin, str);
        dlg->EraseLine();
        if (str.size() != 0)
        {
            string message = dlg->color + dlg->nickname + ": " + str + "\033[0m";
            send(dlg->client_socket, message.c_str(), message.size(), 0);
        }
    }
}

DWORD WINAPI Receiver(LPVOID lp)
{
    Client* dlg = (Client*)lp;
    while (true)
    {
        char response[DEFAULT_BUFLEN];
        int result = recv(dlg->client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';

        cout << response;
    }
}

int main()
{
    Client dlg;
    dlg.EnterInfo();
    if (int i = (dlg.CreateClient() != 0))
        return i;
    Sleep(INFINITE);
}

int Client::CreateClient()
{
    system("title Client");

    if (int i = (InitClient() != 0))
        return i;

    CreateThread(0, 0, Sender, this, 0, 0);
    CreateThread(0, 0, Receiver, this, 0, 0);

    return 0;
}

int Client::InitClient()
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    ptr = nullptr;
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

    return 0;
}

void Client::EraseLine()
{
    cout << "\x1b[A";
    cout << "\x1b[2K";
}

void Client::EnterInfo()
{
    cout << "Enter your nickname: ";
    getline(cin, nickname);
    
    int dColor;
    while (TRUE)
    {
        cout << "Enter your color (from 0 to 5): ";
        cin >> dColor;

        if (dColor >= 0 && dColor <= 5)
            break;
        else
            cout << "Error: wrong color number";
    }
    dColor = 31 + dColor;
    color = "\033[" + to_string(dColor) + "m";
    EraseLine();
}

BOOL ExitHandler(DWORD whatHappening)
{
    switch (whatHappening)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        return(TRUE);
        break;
    default:
        return FALSE;
    }
}