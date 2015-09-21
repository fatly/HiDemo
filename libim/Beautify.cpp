#include "Beautify.h"
#include "Bitmap.h"
#include "AutoPtr.h"
#include "Filter.h"
#include "Blender.h"
#include <assert.h>

namespace e
{
	Beautify::Beautify(void)
	{
		filter = new BlurFilter();
		assert(filter);
		filter->SetSigma(5.0f);

		blender = new Blender();
		assert(blender);
	}


	Beautify::~Beautify(void)
	{
		if (filter) delete filter;
		if (blender) delete blender;
	}

	void Beautify::HighContrast(uint8* dst
		, const uint8* src
		, const int width
		, const int height
		, const int bitCount)
	{
		//blur filter
		filter->Process(dst, src, width, height, bitCount);
		//blend op
		blender->SetMode(BM_HIGHPASS);
		blender->Process(dst, src, width, height, bitCount);
	}
}