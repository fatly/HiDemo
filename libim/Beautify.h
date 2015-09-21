#ifndef __LIBIM_BEAUTIFY_H__
#define __LIBIM_BEAUTIFY_H__
#include "Defines.h"
#include "Object.h"

namespace e
{
	class BlurFilter;
	class Blender;
	class Beautify : public RefCountObj
	{
	public:
		Beautify(void);
		virtual ~Beautify(void);
	public:
		void Beautify::HighContrast(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);
	protected:
		BlurFilter* filter;
		Blender* blender;
	};
}

#endif