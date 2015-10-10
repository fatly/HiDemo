#ifndef __CORE_LIBCAMERA_H__
#define __CORE_LIBCAMERA_H__

#include "ColorSpace.h"
#include "DShowHelp.h"
#include "FFScale.h"
#include "FilterManager.h"
#include "RenderFilter.h"
#include "libcamdefs.h"
#include "FrameCtrl.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")
#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#pragma comment(lib, "libstrmbased.lib")
#else 
#pragma comment(lib, "libstrmbase.lib")
#endif

#endif