#pragma once
#include "Defines.h"
#include "AutoPtr.h"
#include "Object.h"

namespace e
{
	class Beautify : public RefCountObj
	{
	public:
		Beautify(void);
		virtual ~Beautify(void);
	public:
		void Process(uint8* dst, uint8* src, int width, int height, int bitCount);
		void HighContrast(uint8* dst, uint8* src, int width, int height, int bitCount);
	protected:
		
	};
}
