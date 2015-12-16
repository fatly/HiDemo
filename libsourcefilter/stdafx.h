// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <uuids.h>
#include <dshow.h>

// TODO:  在此处引用程序需要的其他头文件
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