#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <string>
using namespace std;

WSADATA wsadata;
SOCKET udpSocket;
sockaddr_in addr;

int SetUp()
{
    int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (res != NO_ERROR)
    {
        cout << "WSAStartup failked with error " << res << endl;
        return 1;
    }
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        cout << "socket failed with error " << WSAGetLastError() << endl;
        return 2;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(23000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    if (bind(udpSocket, (SOCKADDR*)&addr, sizeof(addr)) != NO_ERROR)
    {
        cout << "bind failed with error " << WSAGetLastError() << endl;
        return 3;
    }
    return 0;
}

int main()
{
    if (SetUp() != 0)
        return 1;

    //Get message
    const size_t receiveBufSize = 1024;
    char receiveBuf[receiveBufSize];

    //Address sender
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    string aMessage[3];

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    cout << "Waiting for connecting client" << endl;
    cout << "----------------------------------" << endl;

    while (TRUE)
    {
        for (int i = 0; i < 3; i++)
        {
            int bytesReceived = recvfrom(udpSocket, receiveBuf, receiveBufSize, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
            if (bytesReceived == SOCKET_ERROR)
            {
                cout << "recvfrom failed with error " << WSAGetLastError() << endl;
                return 4;
            }
            receiveBuf[bytesReceived] = '\0';
            aMessage[i] = receiveBuf;
        }

        if (strcmp(aMessage[2].c_str(), "end") == 0)
            break;

        SetConsoleTextAttribute(hConsole, atoi(aMessage[1].c_str()));
        printf("%s: %s\n", aMessage[0].c_str(), aMessage[2].c_str());
        SetConsoleTextAttribute(hConsole, 14);
    }
    cout << "----------------------------------" << endl;
    cout << "The client left the session" << endl;

    const int sendBufSize = 1024;
    char sendBuf[sendBufSize] = "Message accept";

    int sendResult = sendto(udpSocket, sendBuf, strlen(sendBuf), 0, (SOCKADDR*)&senderAddr, senderAddrSize);
    if (sendResult == SOCKET_ERROR)
    {
        cout << "sendto failed with error " << WSAGetLastError() << endl;
        return 4;
    }

    closesocket(udpSocket);
    WSACleanup();
}
