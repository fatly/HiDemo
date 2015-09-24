#ifndef __E_PRIVATE_H__
#define __E_PRIVATE_H__

#include <map>
#include <math.h>

#if defined(_WIN32) || defined(_WIN64)
#	ifndef WINVER
#		define WINVER 0x500
#	endif
#	include <Winsock2.h>
#	include <mswsock.h>
#	include <Windows.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <memory.h>

#include "config.h"

#endif
