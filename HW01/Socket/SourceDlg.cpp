#include "SourceDlg.h"

cSourceDlg* cSourceDlg::ptr = NULL;
cSourceDlg::cSourceDlg(void)
{
	_socket = NULL;
	acceptSocket = NULL;
	ptr = this;
}
void cSourceDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL cSourceDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hDialog = hwnd;
	hButStartClient = GetDlgItem(hDialog, IDC_BUT_START_CLIENT);
	hButStopClient = GetDlgItem(hDialog, IDC_BUT_STOP_CLIENT);
	hEditSendMessage = GetDlgItem(hDialog, IDC_EDIT_WRITE_MESSAGE);
	hEditReadMessage = GetDlgItem(hDialog, IDC_EDIT_READ_MESSAGE);
	hButSendMessage = GetDlgItem(hDialog, IDC_BUT_SEND_MESSAGE);
	return TRUE;
}

void cSourceDlg::MessageError(const WCHAR* text)
{
	MessageBox(hDialog, text, TEXT("Error"), MB_OK | MB_ICONERROR);
}
void cSourceDlg::MessageInfo(const WCHAR* text)
{
	MessageBox(hDialog, text, TEXT("Info"), MB_OK | MB_ICONINFORMATION);
}

DWORD WINAPI Thread(LPVOID lp)
{
	cSourceDlg* dlg = (cSourceDlg*)lp;

	while (true)
	{
		char buf[STR_SIZE];
		int i = recv(dlg->_socket, buf, STR_SIZE, 0);
		if (i == INVALID_SOCKET)
		{
			dlg->MessageError(TEXT("Connection with server is lost"));
			dlg->DisableClient();
			break;
		}

		buf[i] = '\0';
		Beep(4000, 100);
		WCHAR str[STR_SIZE];
		mbstowcs(str, buf, strlen(buf) + 1);
		SetWindowText(dlg->hEditReadMessage, str);
	}
	return 0;
}

void cSourceDlg::StartClient()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		MessageError(TEXT("WSAStartup error"));
		WSACleanup();
		return;
	}
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET)
	{
		MessageError(TEXT("Socket create error"));
		WSACleanup();
		return;
	}
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(20000);
	if (connect(_socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		MessageError(TEXT("Failed to connect to server"));
		WSACleanup();
		return;
	}

	hThread = CreateThread(NULL, 0, Thread, this, 0, NULL);
	EnableWindow(hButStartClient, FALSE);
	EnableWindow(hButStopClient, TRUE);
	EnableWindow(hButSendMessage, TRUE);
}

void cSourceDlg::SendText()
{
	WCHAR str[STR_SIZE];
	GetWindowText(hEditSendMessage, str, STR_SIZE);
	if (lstrlen(str))
	{
		char buf[STR_SIZE];
		wcstombs(buf, str, STR_SIZE);
		send(_socket, buf, strlen(buf), 0);
	}
	else
		MessageError(TEXT("Empty message field"));
}

void cSourceDlg::DisableClient()
{
	WSACleanup();
	closesocket(acceptSocket);
	closesocket(_socket);

	EnableWindow(hButStartClient, TRUE);
	EnableWindow(hButStopClient, FALSE);
	EnableWindow(hButSendMessage, FALSE);

	SetWindowText(hEditReadMessage, NULL);
	SetWindowText(hEditSendMessage, NULL);
}

void cSourceDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BUT_START_CLIENT)
	{
		StartClient();
	}
	else if (id == IDC_BUT_SEND_MESSAGE)
	{
		SendText();
	}
	else if (id == IDC_BUT_STOP_CLIENT)
	{
		TerminateThread(hThread, 0);
		DisableClient();
	}
}

INT_PTR CALLBACK cSourceDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}