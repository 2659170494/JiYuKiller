#pragma once
#include "stdafx.h"

//��������
//    lpszDriverName�������ķ�����
//    driverPath������������·��
//    lpszDisplayName��nullptr
BOOL LoadKernelDriver(const wchar_t* lpszDriverName, const wchar_t* driverPath, const wchar_t* lpszDisplayName);
//ж������
//    szSvrName��������
BOOL UnLoadKernelDriver(const wchar_t* szSvrName);
//������
BOOL OpenDriver();
//���������Ƿ����
BOOL DriverLoaded();

BOOL XLoadDriver();

BOOL XUnLoadDriver();
