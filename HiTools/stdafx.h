// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <assert.h>
#include <atlstr.h>
#include <time.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "libutils.h"
#include "libim.h"
#include "libcamera.h"
#include "streams.h"
using namespace e;

#include "UIlib.h"
using namespace DuiLib;

#ifdef _DEBUG
#	pragma comment(lib, "libduid.lib")
#	pragma comment(lib, "libutilsd.lib")
#	pragma comment(lib, "libimd.lib")
//#	pragma comment(lib, "zlibd.lib")
#	pragma comment(lib, "libstrmbased.lib")
#	pragma comment(lib, "libcamerad.lib")
#else
#	pragma comment(lib, "libdui.lib")
#	pragma comment(lib, "libutils.lib")
#	pragma comment(lib, "libim.lib")
//#	pragma comment(lib, "zlib.lib")
#	pragma comment(lib, "libstrmbase.lib")
#	pragma comment(lib, "libcamera.lib")
#endif