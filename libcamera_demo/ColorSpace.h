#pragma once
#include "Defines.h"

namespace e
{
	class ColorSpace
	{
	public:
		static void BGRA2YCC(uint8* dst, uint8* src, int width, int height, int bitCount);
		static void YCC2BGRA(uint8* dst, uint8* src, int width, int height, int bitCount);
	private:

	};
}

