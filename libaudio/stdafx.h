// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "libutils.h"
#include "streams.h"

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#pragma comment(lib, "libstrmbased.lib")
#else
#pragma comment(lib, "libutilis.lib")
#pragma comment(lib, "libstrmbase.lib")
#endif