#ifndef __LIBIM_BASECLASS_H__
#define __LIBIM_BASECLASS_H__
#include "Defines.h"

namespace e
{
#pragma pack(push, 1)
	typedef struct Pixel {
		channel_t data[4];
	};
#pragma pack(pop)

}

#endif