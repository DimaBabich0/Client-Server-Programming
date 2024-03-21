#pragma once
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include "cOrder.h";
class MyThread
{
public:
	vector <cOrder> cOrder;
	HANDLE hThread;

	static DWORD WINAPI CheckVector(void* Param)
	{
		MyThread* obj = (MyThread*)Param;
		while (TRUE)
		{
			if (obj->cOrder.size() != 0)
			{
				obj->hThread = obj->cOrder[0].StartThread();
				WaitForSingleObject(obj->hThread, INFINITE);

				char szMessage[] = "Your order is ready. Enjoy your meal.";
				send(obj->cOrder[0].client, szMessage, strlen(szMessage), 0);
				sprintf_s(szMessage, "begin");
				send(obj->cOrder[0].client, szMessage, strlen(szMessage), 0);

				CloseHandle(obj->hThread);
				obj->cOrder.erase(obj->cOrder.begin());
			}
		}
	}

	void StartThread()
	{
		CreateThread(NULL, 0, CheckVector, (void*)this, 0, 0);
	}
};
