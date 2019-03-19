// JiYuKiller.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuKiller.h"
#include "DriverLoader.h"
#include "Executor.h"
#include "NtHlp.h"
#include "MsgCenter.h"
#include "StringHlp.h"
#include "StringSplit.h"
#include <time.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include <commoncontrols.h>
#include <windowsx.h>
#include <ShellAPI.h>
#include <shlwapi.h>
// 开启可视化效果  
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name = 'Microsoft.Windows.Common-Controls' version = '6.0.0.0' \
processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

#define TIMER_AOP 10001
#define TIMER_RESET_PID 10002
#define TIMER_TOP_CHECK 10003
#define TIMER_AOP_FULL 10004
#define TIMER_AUTO_HIDE 10005
#define TIMER_CK_LIGHT_BK 10006
#define TIMER_AUTO_HIDE_GB2 10007

using namespace std;

int screenWidth;
int screenHeight;

HINSTANCE hInst;  
WCHAR currentDir[MAX_PATH];
WCHAR virusPath[MAX_PATH];
WCHAR driverPath[MAX_PATH];

HWND hWndMain, hWndTool;
LONG OldEditProc;

HMENU hMenuTray;
NOTIFYICONDATA nid;
UINT WM_TASKBARCREATED;

HBITMAP hBitmapIcoRed, hBitmapIcoGreen, hBitmapIcoOK, hBitmapIcoFail, hBitmapIcoGrey;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR lpCmdLine,_In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	GetModuleFileName(0, currentDir, MAX_PATH);
	PathRemoveFileSpec(currentDir);

	wcscpy_s(virusPath, currentDir);
	wcscpy_s(driverPath, currentDir);
	wcscat_s(driverPath, L"\\JiYuKillerDriver.sys");
	wcscat_s(virusPath, L"\\JiYuKillerVirus.dll");

	LPWSTR *szArgList;
	int argCount;

	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
	if (szArgList == NULL)
	{
		MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
		return -1;
	}

	if(OnReadCommandLine(szArgList, argCount))
		return 0;

	hInst = hInstance;

	InitCommonControls();

	hWndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainWndProc);
	hWndTool = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TOOLBOX), NULL, ToolWndProc);
	ShowWindow(hWndTool, SW_SHOW);
	ShowWindow(hWndMain, SW_SHOW);
	UpdateWindow(hWndMain);
	UpdateWindow(hWndTool);

    MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	LocalFree(szArgList);

    return (int) msg.wParam;
}

INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE) {
			if(OnBeforeExit(hDlg))			
				DestroyWindow(hDlg);
			return (INT_PTR)TRUE;
		}
		if (wParam == SC_MINIMIZE) {
			ShowWindow(hDlg, SW_HIDE);
			return (INT_PTR)TRUE;
		}
		break;
	}
	case WM_INITDIALOG: {
		OnInit(hDlg);
		return (INT_PTR)TRUE;
	}
	case WM_DESTROY: {
		OnDestroy(hDlg);
		PostQuitMessage(0);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		    case IDCANCEL: {
				SendMessage(hDlg, WM_SYSCOMMAND, SC_CLOSE, NULL);
		    }
			default: {
				return OnWnCommand(hDlg, LOWORD(wParam));
			}
		}
		break;
	}
	case WM_TIMER: {
		switch (wParam)
		{
		case TIMER_AOP: {
			OnAop(hDlg);
			break;
		}
		case TIMER_AOP_FULL: {
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
		}
		case TIMER_RESET_PID: {
			OnResetPID(hDlg);
			break;
		}
		case TIMER_TOP_CHECK: {
			SetStatIcon(hDlg, IDC_LIGHT_CK, hBitmapIcoGreen);
			SetStatIcon(hWndTool, IDC_LIGHT_CK, hBitmapIcoGreen);
			RunTopWindowCheckWk(); 
			SetTimer(hDlg, TIMER_CK_LIGHT_BK, 800, NULL);
			break;
		}
		case TIMER_CK_LIGHT_BK: {
			KillTimer(hDlg, TIMER_CK_LIGHT_BK);
			SetStatIcon(hDlg, IDC_LIGHT_CK, hBitmapIcoGrey);
			SetStatIcon(hWndTool, IDC_LIGHT_CK, hBitmapIcoGrey);
			break;
		}
		case TIMER_AUTO_HIDE_GB2: {
			KillTimer(hDlg, TIMER_AUTO_HIDE_GB2);
			EmptyGBStatus();
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_COPYDATA: {
		PCOPYDATASTRUCT  pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
		if (pCopyDataStruct->cbData > 0)
		{
			WCHAR recvData[256] = { 0 };
			wcsncpy_s(recvData, (WCHAR *)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
			HandleMsg(recvData);
		}
		break;
	}
	case WM_USER: {
		OnWmUser(hDlg, wParam, lParam);
		break;
	}
	default: {
		if (message == WM_TASKBARCREATED) CreateTrayIcon(hDlg);
		break;
	}
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ToolWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG: {
		SetWindowPos(hDlg, HWND_TOPMOST, screenWidth - 250, 0, 0, 0, SWP_NOSIZE);
		SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
		SetTimer(hDlg, TIMER_AOP_FULL, 200, NULL);
		SetTimer(hDlg, TIMER_AUTO_HIDE, 6000, NULL);
		break;
	}
	case WM_DESTROY: {
		KillTimer(hDlg, TIMER_AOP_FULL);
		break;
	}
	case WM_LBUTTONDOWN: {
		RECT rc; GetWindowRect(hDlg, &rc);
		if (rc.top == -36) {
			SetWindowPos(hDlg, 0, rc.left, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
		else {
			ReleaseCapture();
			SendMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		break;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_CK: {
			SendMessage(hWndMain, WM_COMMAND, IDC_RUN_CK, NULL);
			break;
		}
		case IDC_BK: {
			SendMessage(hWndMain, WM_COMMAND, IDC_START_KILL, NULL);
			break;
		}
		case IDC_SHOWMAIN: {
			if (IsWindowVisible(hWndMain)) {
				ShowWindow(hWndMain, SW_SHOW);
				SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				SetForegroundWindow(hWndMain);
			}
			else ShowWindow(hWndMain, SW_HIDE);
			break;
		}
		case IDC_MINSIZE: {
			RECT rc; GetWindowRect(hDlg,  &rc);
			SetWindowPos(hDlg, 0, rc.left, -36, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			break;
		}
		default: break;
		}
		break;
	}
	case WM_TIMER: {
		if (wParam == TIMER_AOP_FULL) SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		if (wParam == TIMER_AUTO_HIDE) {
			KillTimer(hDlg, TIMER_AUTO_HIDE);
			SendMessage(hDlg, WM_COMMAND, IDC_MINSIZE, NULL);
		}
		break;
	}
	default: break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK AboutWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CLICK:
		case NM_RETURN:
		{
			PNMLINK pNMLink = (PNMLINK)lParam;
			LITEM item = pNMLink->item;
			if ((((LPNMHDR)lParam)->hwndFrom == GetDlgItem(hDlg, IDC_GITHUB)) && (item.iLink == 0))
				ShellExecute(hDlg, L"open", L"https://github.com/717021/JiYuKiller", NULL, NULL, SW_NORMAL);
			else if ((((LPNMHDR)lParam)->hwndFrom == GetDlgItem(hDlg, IDC_HELPDOC)) && (item.iLink == 0))
				ShellExecute(hDlg, L"open", L"https://github.com/717021/JiYuKiller/blob/master/README.md", NULL, NULL, SW_NORMAL);
			break;
		}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

HDESK hDesktop;
HWND hListBoxStatus;
bool top = false,
aop = false,
noMercyMode = false,
driverLoaded = false,
forceKill = false,
mouseHook = false,
autoCheckTop = true,
autoVirus = true,
autoFck = false,
fullMode = true,
oldCtled = false;

DWORD jiyuPid = 0;
WCHAR jiyuPath[MAX_PATH];

void OutPutStatus(const wchar_t* str, ...)
{
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

void OnRunCmd(vector<wstring> * cmds, int len);

INT_PTR OnWnCommand(HWND hDlg, UINT id)
{
	switch (id) {
	case IDC_CHECK_TOP: {
		if (top) {
			top = false;
			SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else {
			top = true;
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		CheckDlgButton(hDlg, IDC_CHECK_TOP, top);
		break;
	}
	case IDC_MOUSE_HOOK: {
		if (mouseHook) {
			mouseHook = false;
			SetDlgItemText(hDlg, IDC_MOUSE_HOOK, L"MK OFF");
			OutPutStatus(L"MK 已停止");
		}
		else {
			mouseHook = true;
			SetDlgItemText(hDlg, IDC_MOUSE_HOOK, L"MK ON");
			OutPutStatus(L"MK 已启动");
		}
		break;
	}
	case IDC_STATE:
	case IDC_CHECK_TIMER_TOP: {
		if (aop) {
			aop = false;
			SetDlgItemText(hDlg, IDC_STATE, L"AOP OFF");
			KillTimer(hDlg, TIMER_AOP);
			KillTimer(hDlg, TIMER_AOP_FULL);
			OutPutStatus(L"AOP 已停止");
			if(!top) CheckDlgButton(hDlg, IDC_CHECK_TOP, false);
		}
		else {
			aop = true;
			SetDlgItemText(hDlg, IDC_STATE, L"AOP ON");
			OutPutStatus(L"AOP 已启动");
			SetTimer(hDlg, TIMER_AOP, 250, NULL);
			SetTimer(hDlg, TIMER_AOP_FULL, 15, NULL);
			CheckDlgButton(hDlg, IDC_CHECK_TOP, true);
		}
		CheckDlgButton(hDlg, IDC_CHECK_TIMER_TOP, aop);
		break;
	}
	case IDC_CHECK_AUTOVIRUS: {
		autoVirus = !autoVirus;
		CheckDlgButton(hDlg, IDC_CHECK_AUTOVIRUS, autoVirus);
		break;
	}
	case IDC_CHECK_AUTOFCK: {
		autoFck = !autoFck;
		CheckDlgButton(hDlg, IDC_CHECK_AUTOFCK, autoFck);
		break;
	}
	case IDC_TOP_CHECK: {
		if (autoCheckTop) {
			KillTimer(hDlg, TIMER_TOP_CHECK);
			SetDlgItemText(hDlg, IDC_TOP_CHECK, L"CK OFF");
			SetStatIcon(hDlg, IDC_LIGHT_CK, hBitmapIcoRed);
			SetStatIcon(hWndTool, IDC_LIGHT_CK, hBitmapIcoRed);
			autoCheckTop = false;
			OutPutStatus(L"CK 已停止");
		}
		else {
			SetTimer(hDlg, TIMER_TOP_CHECK, 6000, NULL);
			SetDlgItemText(hDlg, IDC_TOP_CHECK, L"CK ON");
			SetStatIcon(hDlg, IDC_LIGHT_CK, hBitmapIcoGreen);
			SetStatIcon(hWndTool, IDC_LIGHT_CK, hBitmapIcoGreen);
			OutPutStatus(L"CK 已启动");
			autoCheckTop = true;
		}
		break;
	}
	case IDC_CHECK_NO_MERCY: {
		noMercyMode = !noMercyMode;
		CheckDlgButton(hDlg, IDC_CHECK_NO_MERCY, noMercyMode);
		break;
	}
	case IDC_CHECK_FORCEKILL: {
		forceKill = !forceKill;
		CheckDlgButton(hDlg, IDC_CHECK_FORCEKILL, forceKill);
		break;
	}
	case IDC_DRIVER_STATUS: {
		if (!driverLoaded) {
			WCHAR currentDrv[MAX_PATH];
			wcscpy_s(currentDrv, currentDir);
			wcscat_s(currentDrv, L"\\JiYuKillerDriver.sys");
			if (LoadKernelDriver(L"JiYuKillerDriver", currentDrv, NULL))
			{
				if (OpenDriver())
				{
					SetDlgItemText(hDlg, IDC_DRIVER_STATUS, L"驱动已加载");
					OutPutStatus(L"驱动加载成功");
					driverLoaded = DriverLoaded();
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FORCEKILL), TRUE);
				}
				else {
					OutPutStatus(L"驱动加载完成，但无法获取驱动程序句柄。");
					SetDlgItemText(hDlg, IDC_DRIVER_STATUS, L"重新加载驱动");
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FORCEKILL), FALSE);
				}
			}
			else {
				SetDlgItemText(hDlg, IDC_DRIVER_STATUS, L"驱动未能加载成功");
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FORCEKILL), FALSE);
			}
		}
		else {
			if (MessageBox(hDlg, L"您希望卸载驱动吗？卸载以后攻击力将大打折扣。", L"提示", MB_ICONASTERISK | MB_YESNO) == IDYES)
			{
				if (UnLoadKernelDriver(L"JiYuKillerDriver")) {
					SetDlgItemText(hDlg, IDC_DRIVER_STATUS, L"驱动未加载");
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FORCEKILL), FALSE);
					driverLoaded = false;
				}
			}
		}
		break;
	}
	case IDC_START_KILL: KillSt(); ReloadJiYuPid(hDlg); break;
	case IDC_EXEC_CMD: {
		WCHAR maxbuf[128];
		GetDlgItemText(hDlg, IDC_EDIT_CMD, maxbuf, 128);
		if (StrEmepty(maxbuf)) MessageBox(hDlg, L"请输入命令以后再执行！", L"提示", MB_ICONEXCLAMATION);
		else {
			wstring cmd(maxbuf);
			vector<wstring> cmds;
			SplitString(cmd, cmds, L" ");
			OnRunCmd(&cmds, cmds.size() - 1);
		}
		break;
	}
	case IDC_CMDS: {
		MessageBox(0, L"killst\nfindst\nfind\nkill\nkillks\nkillkf\nsu\nre\nss\nsss\nssss\n", L"所有命令", 0);
		break;
	}
	case IDC_RERUN_JIYU: if (wcscmp(jiyuPath, L"") != 0) ShellExecute(hDlg, L"open", jiyuPath, NULL, NULL, SW_NORMAL); else { MessageBox(hDlg, L"没有找到 StudentMain.exe ", L"错误", MB_ICONEXCLAMATION); OutPutStatus(L"没有找到 StudentMain.exe "); }break;
	case IDC_HUNH_JIYU: HungSt(); break;
	case IDC_RESUSE_JIYU: ResuseSt(); break;
	case IDC_RUN_CK: RunTopWindowCheckWk(); break;
	case IDC_INSERT_VIRUS: InstallVirus(); break;
	case IDC_TRYU_DRV: TryForceUnloadJiYuDriver(); break;
	case IDC_FCK: autoFck = true;  RunTopWindowCheckWk(); autoFck = false; break;
	case IDC_MORESW: {
		if (fullMode) {
			fullMode = false;
			SetDlgItemText(hDlg, IDC_MORESW, L"▼ 高级模式");
			RECT rc;
			GetWindowRect(hDlg, &rc);
			MoveWindow(hDlg, rc.left, rc.top, rc.right - rc.left, 138, TRUE);
			//132
		}
		else {
			fullMode = true;
			SetDlgItemText(hDlg, IDC_MORESW, L"▲ 隐藏面板");
			RECT rc; 
			GetWindowRect(hDlg, &rc);
			MoveWindow(hDlg, rc.left, rc.top, rc.right - rc.left, 462, TRUE);
			//466
		}
		break;
	}
	case IDC_DLL_VIRUS: InstallDllHook(); break;
	case IDM_EXIT: DestroyWindow(hDlg); break; 
	case IDM_SHOWMAIN: {
		if (IsWindowVisible(hDlg))
			ShowWindow(hDlg, SW_HIDE);
		else ShowWindow(hDlg, SW_SHOW);
		break;
	}
	case IDM_HELP:
	case IDC_ABOUT: DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutWndProc); break;
	}
	return (INT_PTR)FALSE;
}
bool OnBeforeExit(HWND hDlg)
{

	return true;
}
void OnInit(HWND hDlg)
{
	EnableDebugPriv(SE_DEBUG_NAME);
	hDesktop = OpenDesktop(L"Default", 0, FALSE, DESKTOP_ENUMERATE);
	hListBoxStatus = GetDlgItem(hDlg, IDC_LIST_STATUS);
	SendMessage(hListBoxStatus, LB_SETHORIZONTALEXTENT, 1000, 0);

	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_JIYUKILLER)));
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_JIYUKILLER)));

	//ICO Bitmaps
	hBitmapIcoRed = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RED));
	hBitmapIcoGreen = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GREEN));
	hBitmapIcoOK = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OK));
	hBitmapIcoFail = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FAIL));
	hBitmapIcoGrey = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_GREY));

	HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_CMD);
	OldEditProc = GetWindowLong(hEdit, GWL_WNDPROC);
	SetWindowLong(hEdit, GWL_WNDPROC, (LONG)EditProc);

	//Tray icon

	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));

	CreateTrayIcon(hDlg);

	hMenuTray = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAINMENU));
	hMenuTray = GetSubMenu(hMenuTray, 0);

	ExtractParts();

	//Timers
	LoadNt();
	SetTimer(hDlg, TIMER_TOP_CHECK, 6000, NULL);
	SetTimer(hDlg, TIMER_RESET_PID, 6000, NULL);
	OutPutStatus(L"CK 已启动");

	//Find pid
	ReloadJiYuPid(hDlg);

	CheckDlgButton(hDlg, IDC_CHECK_AUTOVIRUS, autoVirus);

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	OutPutStatus(L"屏幕大小：%dx%d", screenWidth, screenHeight);

	SendMessage(hDlg, WM_COMMAND, IDC_MORESW, NULL);

	MsgCenterSendToVirus((LPWSTR)L"hk:ckstat", hDlg);
}
void OnDestroy(HWND hDlg)
{
	DeleteBitmap(hBitmapIcoGrey);
	DeleteBitmap(hBitmapIcoRed);
	DeleteBitmap(hBitmapIcoGreen);
	DeleteBitmap(hBitmapIcoOK);
	DeleteBitmap(hBitmapIcoFail);
	CloseDesktop(hDesktop);
}
void OnRunCmd(vector<wstring> * cmds, int len)
{
	bool succ = true;
	wstring cmd = (*cmds)[0];
	if (cmd == L"killst") KillSt();
	else if (cmd == L"findst") {
		DWORD pid = 0;
		if (TryFindStudent(&pid)) {
			OutPutStatus(L"已经找到 StudentMain.exe PID 为：%d", pid);
		}
		else OutPutStatus(L"没有找到 StudentMain.exe");
	}
	else if (cmd == L"find") {
		if (len > 0) {
			DWORD pid = 0;
			LPWSTR name= (LPWSTR)(*cmds)[1].c_str();
			if (TryFindStudent(&pid)) {
				OutPutStatus(L"已经找到 %s PID 为：%d", name, pid);
			}
			else OutPutStatus(L"没有找到 %s", name);
		}
		else OutPutStatus(L"请输入要查找的进程 find xx.");
	}
	else if (cmd == L"kill") {
		if (len > 0) {
			DWORD pid = _wtol((LPWSTR)(*cmds)[1].c_str());
			if (KillNormal(pid)) 
				OutPutStatus(L"已经结束 PID 为：%d", pid);
			else OutPutStatus(L"无法正常结束：%d", pid);
		}
		else OutPutStatus(L"请输入要结束的进程pid find pid.");
	}
	else if (cmd == L"killks") {
		if (len > 0) {
			DWORD pid = _wtol((LPWSTR)(*cmds)[1].c_str());
			NTSTATUS status = 0;
			if (KillKernelSafe(pid,&status))
				OutPutStatus(L"已经结束 PID 为：%d", pid);
			else OutPutStatus(L"无法强制结束：%d NTSTATUS:0x%08X", pid, status);
		}
		else OutPutStatus(L"请输入要强制结束的进程pid killks pid.");
	}
	else if (cmd == L"killkf") {
		if (len > 0) {
			DWORD pid = _wtol((LPWSTR)(*cmds)[1].c_str());
			NTSTATUS status = 0;
			if (KillKernelUnsafe(pid, &status))
				OutPutStatus(L"已经结束 PID 为：%d", pid);
			else OutPutStatus(L"无法Unsafe结束：%d NTSTATUS:0x%08X", pid, status);
		}
		else OutPutStatus(L"请输入要Unsafe结束的进程pid killkf pid.");
	}
	else if (cmd == L"su") {
		if (len > 0) {
			DWORD pid = _wtol((LPWSTR)(*cmds)[1].c_str());
			NTSTATUS status = 0;
			if (KillKernelUnsafe(pid, &status))
				OutPutStatus(L"已经挂起 PID 为：%d", pid);
			else OutPutStatus(L"无法正常挂起：%d NTSTATUS:0x%08X", pid, status);
		}
		else OutPutStatus(L"请输入要挂起的进程pid killkf pid.");
	}
	else if (cmd == L"re") {
		if (len > 0) {
			DWORD pid = _wtol((LPWSTR)(*cmds)[1].c_str());
			NTSTATUS status = 0;
			if (KillKernelUnsafe(pid, &status))
				OutPutStatus(L"已经取消挂起 PID 为：%d", pid);
			else OutPutStatus(L"无法正常取消挂起：%d NTSTATUS:0x%08X", pid, status);
		}
		else OutPutStatus(L"请输入要取消挂起的进程pid killkf pid.");
	}
	else if (cmd == L"ss") {
		SendVBoom();
	}
	else if (cmd == L"sss") {
		FShutdownn();
	}
	else if (cmd == L"ssss") {
		SendVActive();
	}
	else {
		succ = false;
		MessageBox(0, L"未知命令", L"JY Killer", 0);
	}
	if (succ) SetDlgItemText(hWndMain, IDC_EDIT_CMD, L"");
}
void OnAop(HWND hDlg) {
	//SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	HWND hWnd = TryGetJIYuFullscreenWindow();
	if (hWnd) {
		WCHAR text[32];
		GetWindowText(hWnd, text, 32);
		FuckWindow(hWnd, text);
	}
}
void OnResetPID(HWND hDlg) {
	ReloadJiYuPid(hDlg);
}
bool OnReadCommandLine(LPWSTR *szArgList, int argCount) {
	if (argCount >= 2) {
		if (wcscmp(szArgList[1], L"sss") == 0) {
			WCHAR currentDrv[MAX_PATH];
			wcscpy_s(currentDrv, currentDir);
			wcscat_s(currentDrv, L"\\JiYuKillerDriver.sys");
			if (LoadKernelDriver(L"JiYuKillerDriver", currentDrv, NULL))
			{
				if (OpenDriver())
					FShutdownn();
			}
		}
	}
	return false;
}
void OnWmUser(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDBLCLK)
		SendMessage(hDlg, WM_COMMAND, IDM_SHOWMAIN, lParam);
	if (lParam == WM_RBUTTONDOWN)
	{
		POINT pt;
		GetCursorPos(&pt);//取鼠标坐标  
		SetForegroundWindow(hDlg);//解决在菜单外单击左键菜单不消失的问题  
		TrackPopupMenu(hMenuTray, TPM_RIGHTBUTTON, pt.x - 177, pt.y, NULL, hDlg, NULL);//显示菜单并获取选项ID  
	}
}

LRESULT CALLBACK EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			OnWnCommand(hWndMain, IDC_EXEC_CMD);
			return TRUE;
		}
		break;
	}
	//一定要这么加，只处理需要的消息，不需要的返回给父窗口
	return CallWindowProc((WNDPROC)OldEditProc, hWnd, message, wParam, lParam);
}

void CreateTrayIcon(HWND hDlg) {
	nid.cbSize = sizeof(nid);
	nid.hWnd = hDlg;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_USER;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_JIYUKILLER));
	lstrcpy(nid.szTip, L"JiYuKiller");
	Shell_NotifyIcon(NIM_ADD, &nid);
}
void SetStatIcon(HWND hDlg, int id, HBITMAP hBitmap)
{
	HWND hPic = GetDlgItem(hDlg, id);
	SendMessage(hPic, STM_SETIMAGE, IMAGE_BITMAP, LPARAM(hBitmap));
}
void ReloadJiYuPid(HWND hDlg) {
	DWORD pid = 0, oldPid = jiyuPid;
	if (TryFindStudent(&pid)) {
		if (pid != oldPid && pid != 0) {
			jiyuPid = pid;
			OutPutStatus(L"已找到 StudentMain.exe %d (0x%08x)", jiyuPid, jiyuPid);

			WCHAR text[36]; swprintf_s(text, L"已锁定极域：%d (0x%08x)", jiyuPid, jiyuPid);
			SetDlgItemText(hDlg, IDC_STATIC_JIYU_STAT, text);
			SetStatIcon(hDlg, IDC_STATIC_LIGHT_JIYU_STAT, hBitmapIcoGreen);

			TryFIndStPath();
		}
	}
	if (jiyuPid == 0) {
		SetDlgItemText(hDlg, IDC_STATIC_JIYU_STAT, L"没有找到极域");
		SetStatIcon(hDlg, IDC_STATIC_LIGHT_JIYU_STAT, hBitmapIcoRed);
		ResetCtlStatus(hDlg, false);
	}
}
void ResetCtlStatus(HWND hDlg, bool ctled) {
	if (oldCtled != ctled) {
		oldCtled = ctled;
		if (ctled) {
			SetDlgItemText(hDlg, IDC_STATIC_CTL_STAT, L"已经控制极域软件主进程");
			SetStatIcon(hDlg, IDC_LIGHT_CTL_STAT, hBitmapIcoOK);
		}
		else {
			SetDlgItemText(hDlg, IDC_STATIC_CTL_STAT, L"当前未控制极域软件主进程");
			SetStatIcon(hDlg, IDC_LIGHT_CTL_STAT, hBitmapIcoFail);
			ResetGBStatus(0, false, false);
		}
	}
}
void SetCtlFailStatus(LPCWSTR fail) {
	SetDlgItemText(hWndMain, IDC_STATIC_CTL_STAT, fail);
	SetStatIcon(hWndMain, IDC_LIGHT_CTL_STAT, hBitmapIcoFail);
}
void HandleMsg(LPWSTR buff) {
	OutPutStatus(L"Receive message : %s", buff);
	wstring act(buff);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2) {
		if (arr[0] == L"hkb" && arr[1] == L"succ")
			ResetCtlStatus(hWndMain, true);
		else if (arr[0] == L"wcd") {
			//wwcd
		}
	}
}
void ResetGBStatus(int count, bool hasGb, bool hasHp)
{
	WCHAR timeText[128];
	WCHAR text[32];
	time_t time_log = time(NULL);
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);
	swprintf_s(timeText, L"[%02d:%02d:%02d] ", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec);
	if (count > 0) {
		if(hasGb &&hasHp) swprintf_s(text, L"已发现极域广播和黑屏窗口。已处理：%d ", count);
		else if (hasHp) swprintf_s(text, L"已发现极域黑屏窗口。已处理：%d ", count);
		else if (hasGb) swprintf_s(text, L"已发现极域广播窗口。已处理：%d ", count);
		else swprintf_s(text, L"已发现极域非法窗口。已处理：%d ", count);
		wcscat_s(timeText, text);

		SetDlgItemText(hWndMain, IDC_STATIC_GBWND_STAT, timeText);
		SetStatIcon(hWndMain, IDC_LIGHT_GB_WND_STAT, hBitmapIcoGreen);
	}
	else {
		wcscat_s(timeText, L"未发现极域非法窗口");
		SetDlgItemText(hWndMain, IDC_STATIC_GBWND_STAT, timeText);
		SetStatIcon(hWndMain, IDC_LIGHT_GB_WND_STAT, hBitmapIcoRed);
	}

	SetTimer(hWndMain, TIMER_AUTO_HIDE_GB2, 3000, NULL);
}
void EmptyGBStatus() {
	SetDlgItemText(hWndMain, IDC_STATIC_GBWND_STAT, L"");
	SetStatIcon(hWndMain, IDC_LIGHT_GB_WND_STAT, hBitmapIcoGrey);
}

