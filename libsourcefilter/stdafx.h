// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <uuids.h>
#include <dshow.h>

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "streams.h"
#include "libutils.h"
using namespace e;

#ifdef _DEBUG
#pragma comment(lib,"libutilsd.lib")
#pragma comment(lib, "libstrmbased.lib")
#else
#pragma comment(lib,"libutils.lib")
#pragma comment(lib, "libstrmbase.lib")
#endif

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")
#pragma comment(lib, "winmm.lib")