#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <string>
using namespace std;

int main()
{
    WSADATA wsadata;

    int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (res != NO_ERROR)
    {
        cout << "WSAStartup failked with error " << res << endl;
        return 1;
    }

    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        cout << "socket failed with error " << WSAGetLastError() << endl;
        return 2;
    }

    sockaddr_in addrTo;
    addrTo.sin_family = AF_INET;
    addrTo.sin_port = htons(23000);
    inet_pton(AF_INET, "127.0.0.1", &addrTo.sin_addr.s_addr);

    string aMessage[3];

    //get nick
    cout << "Type your nick: ";
    getline(cin, aMessage[0]);
    //get color
    cout << "Type your color: ";
    getline(cin, aMessage[1]);
    cout << "Enter \"end\" for closing chat" << endl;

    cout << "---------------------------" << endl;
    while (TRUE)
    {
        cout << "Type your message: ";
        getline(cin, aMessage[2]);

        for (size_t i = 0; i < 3; i++)
        {
            int sendResult = sendto(udpSocket, aMessage[i].c_str(), strlen(aMessage[i].c_str()), 0, (SOCKADDR*)&addrTo, sizeof(addrTo));
            if (sendResult == SOCKET_ERROR)
            {
                cout << "sendto failed with error " << WSAGetLastError() << endl;
                return 4;
            }
        }

        if (strcmp(aMessage[2].c_str(), "end") == 0)
            break;
    }
    cout << "---------------------------" << endl;

    const size_t receiveBufSize = 1024;
    char receiveBuf[receiveBufSize];

    sockaddr_in addrFrom;
    int addrFromSize = sizeof(addrFrom);

    cout << "Receiving data..." << endl;
    int bytesReceived = recvfrom(udpSocket, receiveBuf, receiveBufSize, 0, (SOCKADDR*)&addrFrom, &addrFromSize);
    if (bytesReceived == SOCKET_ERROR)
    {
        cout << "recvfrom failed with error " << WSAGetLastError() << endl;
        return 4;
    }

    receiveBuf[bytesReceived] = '\0';
    cout << "Received from " << addrFrom.sin_addr.s_host << endl;
    cout << "Data: " << receiveBuf << endl;

    closesocket(udpSocket);
    WSACleanup();
}
