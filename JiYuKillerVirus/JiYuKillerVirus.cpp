﻿#include "..\JiYuKiller\JiYuKiller.h"
// JiYuKillerVirus.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "JiYuKillerVirus.h"
#include "resource.h"
#include "StringHlp.h"
#include "StringSplit.h"
#include "mhook-lib/mhook.h"
#include <list>
#include <time.h>
#include <CommCtrl.h>
#include <windowsx.h>

#define TIMER_WATCH_DOG_SRV 10011

using namespace std;

extern HINSTANCE hInst;

WNDPROC jiYuWndProc;
list<tag_WNDIJJ>  jiYuWnds;
HWND jiYuGBWnd = NULL;
HWND jiYuGBDeskRdWnd = NULL;

HWND hWndMsgCenter = NULL;
HWND hListBoxStatus = NULL;

INT screenWidth, screenHeight;

void VUnloadAll() {
	VCloseMsgCenter();
	VUnInstallHooks();
	VCloseFuckDrivers();
	jiYuWnds.clear();
}
void VRunMain() {
	VParamInit();
	VCreateMsgCenter();
	VOpenFuckDrivers();
	VInstallHooks();
}

DWORD WINAPI VMsgCenterRunThread(LPVOID lpThreadParameter) {
	hWndMsgCenter = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MSGCT), NULL, MainWndProc);
	ShowWindow(hWndMsgCenter, SW_SHOW);
	UpdateWindow(hWndMsgCenter);

	hListBoxStatus = GetDlgItem(hWndMsgCenter, IDC_STATUS_LIST);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void VParamInit() {
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
}
void VCreateMsgCenter() {
	CreateThread(NULL, NULL, VMsgCenterRunThread, NULL, NULL, NULL);
}
void VCloseMsgCenter() {
	DestroyWindow(hWndMsgCenter);
}
void VHandleMsg(LPWSTR buff) {
	VOutPutStatus(L"Receive message : %s", buff);
	wstring act(buff);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2) {
		if (arr[0] == L"hw") 
			VHookWindow(arr[1].c_str());
		else if (arr[0] == L"hwf")
			VHookFWindow(arr[1].c_str());
		else if (arr[0] == L"ss")
			VBoom();
	}
}
void VOutPutStatus(const wchar_t* str, ...) {
	time_t time_log = time(NULL);
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);
	va_list arg;
	va_start(arg, str);
	wstring format1 = FormatString(L"[%02d:%02d:%02d] %s", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec, str);
	wstring out = FormatString(format1.c_str(), arg);
	SendMessage(hListBoxStatus, LB_ADDSTRING, 0, (LPARAM)out.c_str());
	SendMessage(hListBoxStatus, LB_SETTOPINDEX, ListBox_GetCount(hListBoxStatus) - 1, 0);
	va_end(arg);
}
void VHookFWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[16];
			GetWindowText(hWnd, text, 16);
			if (wcscmp(text, L"屏幕广播") == 0) {
				VFixGuangBoWindow(hWnd);
				jiYuGBWnd = hWnd;
			}
		}
		if (!VIsInIllegalWindows(hWnd)) {
			tag_WNDIJJ ij;
			ij.wnd = hWnd;
			ij.canSizeToFull = true;
			jiYuWnds.push_back(ij);
			/*WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);;
			if (oldWndProc != (WNDPROC)JiYuWndProc && oldWndProc != (WNDPROC)MainWndProc && jiYuWndProc != oldWndProc) {
				jiYuWndProc = oldWndProc;
				SetWindowLong(hWnd, GWL_WNDPROC, (LONG)jiYuWndProc);
			}
			*/
		}
		else {
			VIsInIllegalWindowsSetCansize(hWnd);

		}
	}
}
void VHookWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[16];
			GetWindowText(hWnd, text, 16);
			if (wcscmp(text, L"屏幕广播") == 0) {
				VFixGuangBoWindow(hWnd);
				jiYuGBWnd = hWnd;
			}
		}
		//
		if (!VIsInIllegalWindows(hWnd)) {
			tag_WNDIJJ ij;
			ij.wnd = hWnd;
			ij.canSizeToFull = true;
			jiYuWnds.push_back(ij);
			/*WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);;
			if (oldWndProc != (WNDPROC)JiYuWndProc && oldWndProc != (WNDPROC)MainWndProc && jiYuWndProc != oldWndProc) {
				jiYuWndProc = oldWndProc;
				SetWindowLong(hWnd, GWL_WNDPROC, (LONG)jiYuWndProc);
			}
			*/
		}
	}
}
void VFixGuangBoWindow(HWND hWnd) {
	WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
	if (oldWndProc != (WNDPROC)JiYuWndProc && oldWndProc != (WNDPROC)MainWndProc && jiYuWndProc != oldWndProc) {
		oldWndProc = (WNDPROC)JiYuWndProc;
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)JiYuWndProc);
		VOutPutStatus(L"Hooked hWnd %d (0x%08x) WNDPROC", hWnd, hWnd);
	}
}
bool VIsInIllegalWindowsCanSize(HWND hWnd, bool setTo) {
	list<tag_WNDIJJ>::iterator testiterator;
	for (testiterator = jiYuWnds.begin(); testiterator != jiYuWnds.end(); ++testiterator)
	{
		if ((*testiterator).wnd == hWnd && (*testiterator).canSizeToFull) {
			(*testiterator).canSizeToFull = setTo;
			return true;
		}
	}
	return false;
}
bool VIsInIllegalWindows(HWND hWnd) {
	list<tag_WNDIJJ>::iterator testiterator;
	for (testiterator = jiYuWnds.begin(); testiterator != jiYuWnds.end(); ++testiterator)
	{
		if ((*testiterator).wnd == hWnd)
			return true;
	}
	return false;
}
bool VIsInIllegalWindowsSetCansize(HWND hWnd) {
	list<tag_WNDIJJ>::iterator testiterator;
	for (testiterator = jiYuWnds.begin(); testiterator != jiYuWnds.end(); ++testiterator)
	{
		if ((*testiterator).wnd == hWnd) {
			(*testiterator).canSizeToFull = true;
			return true;
		}
	}
	return false;
}
void VBoom() {
	FreeLibrary(GetModuleHandle(L"ntdll.dll"));
	CHAR*P = 0;
	*P = 0;
}

INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		//SetTimer(hDlg, TIMER_WATCH_DOG_SRV, 10000,	NULL);
		break;	
	}
	case WM_DESTROY: {
		//KillTimer(hDlg, TIMER_WATCH_DOG_SRV);
		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE)
			DestroyWindow(hWndMsgCenter);
		break;
	}
	case WM_COMMAND: {
		if (wParam == IDC_KILL) {
			PostQuitMessage(0);
			ExitProcess(0);
		}
		break;
	}
	case WM_COPYDATA: {
		PCOPYDATASTRUCT  pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
		if (pCopyDataStruct->cbData > 0)
		{
			WCHAR recvData[256] = { 0 };
			wcsncpy_s(recvData, (WCHAR *)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
			VHandleMsg(recvData);
		}
		break;
	}
	default:
		break;
	}
	return 0;
}
INT_PTR CALLBACK JiYuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	/*
	if (message == WM_TIMER) {
		KillTimer(hWnd, wParam);
		return 0;
	}
	*/
	if (message == WM_GETMINMAXINFO) {
		PMINMAXINFO pMinMaxInfo = (PMINMAXINFO)lParam;
		pMinMaxInfo->ptMinTrackSize.x = 0;
		pMinMaxInfo->ptMinTrackSize.y = 0;
		return 0;
	}
	if (message == WM_SIZE) {
		RECT rcWindow;
		RECT rcClient;
		GetWindowRect(hWnd, &rcWindow);
		GetClientRect(hWnd, &rcClient);

		bool setToFull = false;
		if (rcWindow.right - rcWindow.left == screenWidth && rcWindow.bottom - rcWindow.top == screenHeight)
			setToFull = true;
		if (jiYuGBDeskRdWnd == NULL) 
			jiYuGBDeskRdWnd = FindWindowExW(hWnd, NULL, NULL, L"TDDesk Render Window");
		if (!IsWindow(jiYuGBDeskRdWnd) || GetParent(jiYuGBDeskRdWnd) != hWnd)
			jiYuGBDeskRdWnd = NULL;
		if (jiYuGBDeskRdWnd != NULL)
			if(setToFull)MoveWindow(jiYuGBDeskRdWnd, 0, 0, screenWidth, screenHeight, TRUE);
			else MoveWindow(jiYuGBDeskRdWnd, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, TRUE);
	}

	if (jiYuWndProc) return jiYuWndProc(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL hk1 = 0, hk2 = 0, hk3 = 0, hk4 = 0,
hk5 = 0, hk6 = 0, hk7 = 0, hk8 = 0,
hk9 = 0, hk10 = 0, hk11 = 0, hk12 = 0,
hk13 = 0, hk14 = 0, hk15 = 0, hk16 = 0;

//Real api address

fnSetWindowPos raSetWindowPos = NULL;
fnMoveWindow raMoveWindow = NULL;
fnSetForegroundWindow raSetForegroundWindow = NULL;
fnBlockInput raBlockInput = NULL;
fnDeviceIoControl raDeviceIoControl = NULL;
fnCreateFileA faCreateFileA = NULL;
fnCreateFileW faCreateFileW = NULL;
fnSetWindowsHookExA faSetWindowsHookExA = NULL;
fnmouse_event famouse_event = NULL;
fnSendInput faSendInput = NULL;
fnSetThreadDesktop faSetThreadDesktop = NULL;
fnChangeDisplaySettingsW faChangeDisplaySettingsW = NULL;
fnOpenDesktopA faOpenDesktopA = NULL;
fnOpenInputDesktop faOpenInputDesktop = NULL;
fnTDDeskCreateInstance faTDDeskCreateInstance = NULL;

void VInstallHooks() {

	//Mhook_SetHook
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");

	raSetWindowPos = (fnSetWindowPos)GetProcAddress(hUser32, "SetWindowPos");
	raMoveWindow = (fnMoveWindow)GetProcAddress(hUser32, "MoveWindow");
	raSetForegroundWindow = (fnSetForegroundWindow)GetProcAddress(hUser32, "SetForegroundWindow");
	raBlockInput = (fnBlockInput)GetProcAddress(hUser32, "BlockInput");
	faSetWindowsHookExA = (fnSetWindowsHookExA)GetProcAddress(hUser32, "SetWindowsHookExA");
	famouse_event = (fnmouse_event)GetProcAddress(hUser32, "mouse_event");
	faSendInput = (fnSendInput)GetProcAddress(hUser32, "SendInput");
	faSetThreadDesktop = (fnSetThreadDesktop)GetProcAddress(hUser32, "SetThreadDesktop");
	faChangeDisplaySettingsW = (fnChangeDisplaySettingsW)GetProcAddress(hUser32, "ChangeDisplaySettingsW");
	faOpenDesktopA = (fnOpenDesktopA)GetProcAddress(hUser32, "OpenDesktopA");
	faOpenInputDesktop = (fnOpenInputDesktop)GetProcAddress(hUser32, "OpenInputDesktop");

	raDeviceIoControl = (fnDeviceIoControl)GetProcAddress(hKernel32, "DeviceIoControl");
	faCreateFileA = (fnCreateFileA)GetProcAddress(hKernel32, "CreateFileA");
	faCreateFileW = (fnCreateFileW)GetProcAddress(hKernel32, "CreateFileW");

	//HMODULE hTDDesk2 = GetModuleHandle(L"libtddesk2.dll");
	//if (hTDDesk2) {
	//	faTDDeskCreateInstance = (fnTDDeskCreateInstance)GetProcAddress(hTDDesk2, "TDDeskCreateInstance");
	//}

	hk1 = Mhook_SetHook((PVOID*)&raSetWindowPos, hkSetWindowPos);
	hk2 = Mhook_SetHook((PVOID*)&raMoveWindow, hkMoveWindow);
	hk3 = Mhook_SetHook((PVOID*)&raSetForegroundWindow, hkSetForegroundWindow);
	hk4 = Mhook_SetHook((PVOID*)&raBlockInput, hkBlockInput);
	hk5 = Mhook_SetHook((PVOID*)&raDeviceIoControl, hkDeviceIoControl);
	hk6 = Mhook_SetHook((PVOID*)&faCreateFileA, hkCreateFileA);
	hk7 = Mhook_SetHook((PVOID*)&faCreateFileW, hkCreateFileW);
	hk8 = Mhook_SetHook((PVOID*)&faSetWindowsHookExA, hkSetWindowsHookExA);
	hk9 = Mhook_SetHook((PVOID*)&famouse_event, hkmouse_event);
	hk10 = Mhook_SetHook((PVOID*)&faSendInput, hkSendInput);
	hk11 = Mhook_SetHook((PVOID*)&faSetThreadDesktop, hkSetThreadDesktop);
	hk12 = Mhook_SetHook((PVOID*)&faChangeDisplaySettingsW, hkChangeDisplaySettingsW);
	hk13 = Mhook_SetHook((PVOID*)&faOpenDesktopA, hkOpenDesktopA);
	hk14 = Mhook_SetHook((PVOID*)&faOpenInputDesktop, hkOpenInputDesktop);

	if(faTDDeskCreateInstance) 
		hk15 = Mhook_SetHook((PVOID*)&faTDDeskCreateInstance, hkTDDeskCreateInstance);

}
void VUnInstallHooks() {

	if (hk1) Mhook_Unhook((PVOID*)&raSetWindowPos);
	if (hk2) Mhook_Unhook((PVOID*)&raMoveWindow);
	if (hk3) Mhook_Unhook((PVOID*)&raSetForegroundWindow);
	if (hk4) Mhook_Unhook((PVOID*)&raBlockInput);
	if (hk5) Mhook_Unhook((PVOID*)&raDeviceIoControl);
	if (hk6) Mhook_Unhook((PVOID*)&faCreateFileA);
	if (hk7) Mhook_Unhook((PVOID*)&faCreateFileW);
	if (hk8) Mhook_Unhook((PVOID*)&faSetWindowsHookExA);
	if (hk9) Mhook_Unhook((PVOID*)&famouse_event);
	if (hk10) Mhook_Unhook((PVOID*)&faSendInput);
	if (hk11) Mhook_Unhook((PVOID*)&faSetThreadDesktop);
	if (hk12) Mhook_Unhook((PVOID*)&faChangeDisplaySettingsW);
	if (hk13) Mhook_Unhook((PVOID*)&faOpenDesktopA);
	if (hk14) Mhook_Unhook((PVOID*)&faOpenInputDesktop);
	if (hk15) Mhook_Unhook((PVOID*)&faTDDeskCreateInstance);
}

//Fuck driver devices
HANDLE hDeviceTDKeybd = NULL;

void VOpenFuckDrivers() {
	hDeviceTDKeybd = CreateFile(L"\\\\.\\TDKeybd", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}
void VCloseFuckDrivers()
{
	CloseHandle(hDeviceTDKeybd);
}

//Hook stubs

BOOL WINAPI hkSetForegroundWindow(HWND hWnd)
{
	if (VIsInIllegalWindows(hWnd)) 
		return TRUE;
	return raSetForegroundWindow(hWnd);
}
BOOL WINAPI hkSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight)
	{
		if (VIsInIllegalWindowsCanSize(hWnd, false)) 
		{
			WCHAR text[32];
			GetWindowText(hWnd, text, 32);
			if (wcscmp(text, L"屏幕广播") == 0 || wcscmp(text, L"BlackScreen Window") == 0)
			{
				ShowWindow(hWnd, SW_MINIMIZE);
				Sleep(100);
				ShowWindow(hWnd, SW_NORMAL);
			}
			return raSetWindowPos(hWnd, HWND_BOTTOM, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		}
	}
	if(VIsInIllegalWindows(hWnd)) {
		return TRUE;
	}
	return raSetWindowPos(hWnd, HWND_BOTTOM, x, y, cx, cy, uFlags^SWP_NOACTIVATE);
}
BOOL WINAPI hkMoveWindow(HWND hWnd, int x, int y, int cx, int cy, BOOL bRepaint)
{
	if (x == 0 && y == 0 && cx == screenWidth && cx == screenHeight) {
		WCHAR text[32];
		GetWindowText(hWnd, text, 32);
		if (wcscmp(text, L"屏幕广播") == 0 || wcscmp(text, L"BlackScreen Window") == 0)
		{
			ShowWindow(hWnd, SW_MINIMIZE);
			Sleep(100);
			ShowWindow(hWnd, SW_NORMAL);
		}
		return TRUE;
	}
	return raMoveWindow(hWnd, x, y, cx, cy, bRepaint);
}
BOOL WINAPI hkBlockInput(BOOL fBlockIt) {
	if (fBlockIt) return TRUE;
	return raBlockInput(fBlockIt);
}
BOOL WINAPI hkDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned,  LPOVERLAPPED lpOverlapped) {
	if (hDeviceTDKeybd) {
		if (hDevice == hDeviceTDKeybd) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	return raDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
}
HANDLE WINAPI hkCreateFileA(LPCSTR lpFileName,  DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile ) {
	if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDKeybd")) {
		SetLastError(ERROR_ACCESS_DENIED);
		return INVALID_HANDLE_VALUE;
	}	
	if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDVideo")) {
		SetLastError(ERROR_ACCESS_DENIED);
		return INVALID_HANDLE_VALUE;
	}
	
	return faCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HANDLE WINAPI hkCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,  DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDKeybd")) {
		SetLastError(ERROR_ACCESS_DENIED);
		return INVALID_HANDLE_VALUE;
	}
	if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDVideo")) {
		SetLastError(ERROR_ACCESS_DENIED);
		return INVALID_HANDLE_VALUE;
	}
	return faCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HHOOK WINAPI hkSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
	if (idHook == WH_MOUSE_LL || idHook == WH_MOUSE) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}
	return faSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
}
VOID WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData,  ULONG_PTR dwExtraInfo)
{
	
}
UINT WINAPI hkSendInput(UINT cInputs,  LPINPUT pInputs,  int cbSize)
{
	return cInputs;
}
BOOL WINAPI hkSetThreadDesktop(HDESK hDesktop) {
	return FALSE;
}
LONG WINAPI hkChangeDisplaySettingsW(DEVMODEW* lpDevMode, DWORD dwFlags)
{
	return DISP_CHANGE_SUCCESSFUL;
}
HDESK WINAPI hkOpenDesktopA(LPCSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}
HDESK WINAPI hkOpenInputDesktop(DWORD dwFlags,BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}
HRESULT __cdecl hkTDDeskCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv)
{
	return E_FAIL;
}