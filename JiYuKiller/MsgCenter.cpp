#include "stdafx.h"
#include "MsgCenter.h"
#include "Tracker.h"
#include <list>

extern HWND hWndMain;
extern bool controlled;

std::list<HWND> jiyuWnds;

void MsgCenterSendToVirus(LPWSTR buff, HWND form) 
{
	HWND receiveWindow = FindWindow(NULL, L"JY Killer Virus");
	if (receiveWindow) {
		COPYDATASTRUCT copyData = { 0 };
		copyData.lpData = buff;
		copyData.cbData = sizeof(WCHAR) * (wcslen(buff) + 1);
		SendMessageTimeout(receiveWindow, WM_COPYDATA, (WPARAM)form, (LPARAM)&copyData, SMTO_ABORTIFHUNG | SMTO_NORMAL, 500, 0);
	}
	else {
		if (controlled &&  TForceKill())
			MessageBox(NULL, L"���򲡶�������������ƣ�Ϊ�˰�ȫ������Ѿ�������������Ҫ�ֶ�������������", L"��ʾ", MB_ICONINFORMATION);
	}
}

bool IsInIllegalWindows(HWND hWnd) {
	std::list<HWND>::iterator testiterator;
	for (testiterator = jiyuWnds.begin(); testiterator != jiyuWnds.end(); ++testiterator)
	{
		if ((*testiterator) == hWnd)
			return true;
	}
	return false;
}
void MsgCenteAppendHWND(HWND hWnd)
{
	if (!IsInIllegalWindows(hWnd)) jiyuWnds.push_back(hWnd);
}
void MsgCenterSendHWNDS()
{
	int iwndCount = jiyuWnds.size();
	std::list<HWND>::iterator testiterator;
	for (testiterator = jiyuWnds.begin(); testiterator != jiyuWnds.end(); ++testiterator)
	{
		HWND hWnd = (*testiterator);
		WCHAR str[65];
		swprintf_s(str, L"hw:%d", (LONG)hWnd);
		MsgCenterSendToVirus(str, hWndMain);
	}
	jiyuWnds.clear();
	//ResetGBStatus(iwndCount, lastHasGb, lastHasHp);
}