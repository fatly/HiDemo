// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <assert.h>
#include <atlstr.h>
#include <time.h>


// TODO:  在此处引用程序需要的其他头文件
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