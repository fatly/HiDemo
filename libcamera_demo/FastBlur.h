#pragma once
#include "Defines.h"

namespace e
{
	void FastBlur(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma, int mode);
}
