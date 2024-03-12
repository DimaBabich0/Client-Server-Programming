#pragma once
#include "header.h"

class cSourceDlg
{
public:
	static cSourceDlg* ptr;

	cSourceDlg(void);
	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);

	void MessageError(const WCHAR* text);
	void MessageInfo(const WCHAR* text);

	void StartServer();
	void DisableServer();
	
	void SendText();

	HWND hDialog, hButStartServer, hButStopServer, hEditSendMessage, hButSendMessage, hEditReadMessage;
	HANDLE hThread;

	WSADATA wsaData;
	SOCKET _socket;
	SOCKET acceptSocket;
	sockaddr_in addr;
};
