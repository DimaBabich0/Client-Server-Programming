#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
using namespace std;

string FindKey(string message, string key)
{
    string value;
    size_t pos = message.find(key);
    if (pos != wstring::npos)
    {
        pos += key.size();
        pos += 1;
        if (message[pos] == '"' ||
            message[pos] == ' ')
            pos++;
        while (TRUE)
        {
            if (message[pos] == '"' ||
                (message[pos] == ',' && key != "Date") ||
                message[pos] == '}' ||
                message[pos] == '\n')
                break;
            value += message[pos];
            pos++;
        }
    }
    return value;
}

int main()
{
    setlocale(0, "ru");

    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться

    char hostname[255] = "api.openweathermap.org";

    addrinfo* result = NULL;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int dResult = getaddrinfo(hostname, "http", &hints, &result);
    if (dResult != 0)
    {
        cout << "getaddrinfo failed with error: " << dResult << endl;
        WSACleanup();
        return 3;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;

    //Пробуем присоединиться к полученному адресу
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        //2. создание клиентского сокета
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET)
        {
            printf("Socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        //3. Соединяемся с сервером
        dResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (dResult == SOCKET_ERROR)
        {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    //4. HTTP Request
    const string API = "3a5b8c57ba9796882ea63fc5704c505a";
    string city;
    cout << "Enter city name: ";
    cin >> city;
    cout << endl;
    string uri = "/data/2.5/weather?q=" + city + "&appid=" + API + "&units=metric" + "&mode=JSON";

    string request = "GET " + uri + " HTTP/1.1\n";
    request += "Host: " + string(hostname) + "\n";
    request += "Accept: */*\n";
    request += "Accept-Encoding: gzip, deflate, br\n";
    request += "Connection: close\n";
    request += "\n";

    //отправка сообщения
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR)
    {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    //cout << "send data" << endl;

    //5. HTTP Response

    string response;

    const size_t BUFFERSIZE = 1024;
    char resBuf[BUFFERSIZE];

    int respLength;

    do
    {
        respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
        if (respLength > 0)
        {
            response += string(resBuf).substr(0, respLength);
        }
        else
        {
            cout << "recv failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 6;
        }
    } while (respLength == BUFFERSIZE);

    //cout << response << endl;

    //отключает отправку и получение сообщений сокетом
    dResult = shutdown(connectSocket, SD_BOTH);
    if (dResult == SOCKET_ERROR)
    {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }

    closesocket(connectSocket);
    WSACleanup();

    time_t time;
    char* dt;
    tm* gmtm;

    if (FindKey(response, "\"cod\"") == "404")
    {
        cout << "City not found" << endl;
        return 8;
    }

    cout << "Date: " << FindKey(response, "Date") << endl;
    cout << "Country: " << FindKey(response, "\"country\"") << endl;
    cout << "City: " << FindKey(response, "\"name\"") << endl;
    cout << "Coordinates: " << FindKey(response, "\"lon\"") << ", " << FindKey(response, "\"lat\"") << endl;
    cout << "Temperature: " << FindKey(response, "\"temp\"") << "°C" << endl;
    
    time = stoi(FindKey(response, "\"sunrise\""));
    dt = ctime(&time);
    gmtm = gmtime(&time);
    dt = asctime(gmtm);
    cout << "Sunrise time: " << dt;

    time = stoi(FindKey(response, "\"sunset\""));
    dt = ctime(&time);
    gmtm = gmtime(&time);
    dt = asctime(gmtm);
    cout << "Sunset time: " << dt;
}