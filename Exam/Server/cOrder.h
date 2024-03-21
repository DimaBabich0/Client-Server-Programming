#pragma once
#include <iostream>
#include <string>
using namespace std;

class cOrder
{
public:
    SOCKET client;
    float fPrice;
    int dTime;
    string szMessage;

    cOrder(SOCKET client, float fPrice, int dTime, string szMessage)
    {
        this->client = client;
        this->fPrice = fPrice;
        this->dTime = dTime;
        this->szMessage = szMessage;
    }

    static DWORD WINAPI Cook(void* Param)
    {
        cOrder* obj = (cOrder*)Param;
        for (int TimeSleep = obj->dTime; TimeSleep >= 0; TimeSleep--)
            Sleep(1000);
        return 0;
    }

    HANDLE StartThread()
    {
        send(client, szMessage.c_str(), szMessage.size(), 0);
        return CreateThread(NULL, 0, Cook, (void*)this, 0, 0);
    }
};

bool operator==(const cOrder& l, const cOrder& r)
{
    return (l.dTime == r.dTime && l.fPrice == r.fPrice) ? TRUE : FALSE;
}