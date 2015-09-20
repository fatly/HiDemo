#ifndef __LIBIM_BEAUTIFY_H__
#define __LIBIM_BEAUTIFY_H__
#include "Defines.h"
#include "Object.h"

namespace e
{
	class Beautify : public RefCountObj
	{
	public:
		Beautify(void);
		virtual ~Beautify(void);
	public:
		void Beautify::HighContrast(uint8* dst
			, uint8* src
			, int width
			, int height
			, int bitCount);
	};
}

#endif