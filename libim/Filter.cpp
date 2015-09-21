#include "Filter.h"
#include <stdlib.h>
#include <assert.h>
#include "Gaussian.h"

namespace e
{
	BlurFilter::BlurFilter(void)
	{
		tmp = 0;
		width = 0;
		height = 0;

		filter = new Gaussian();
		filter->SetMode(GM_HIGH);
		filter->SetSigma(1.5f);
	}

	BlurFilter::BlurFilter(float sigma)
	{
		tmp = 0;
		width = 0;
		height = 0;

		filter = new Gaussian();
		filter->SetMode(GM_HIGH);
		filter->SetSigma(sigma);
	}

	BlurFilter::~BlurFilter(void)
	{
		if (tmp) free(tmp);
	}

	void BlurFilter::SetSize(int width, int height)
	{
		assert(width > 0 && height > 0);
		this->width = width;
		this->height = height;

		int size = width * height * 4;
		tmp = (uint8*)realloc(tmp, size);
		assert(tmp);
	}

	void BlurFilter::SetSigma(float sigma)
	{
		assert(filter);
		filter->SetSigma(sigma);
	}

	void BlurFilter::Process(uint8* dst
		, const uint8* src
		, const int width
		, const int height
		, const int bitCount)
	{
		if (this->width != width || this->height != height)
		{
			SetSize(width, height);
		}

		assert(filter);
		filter->Process(tmp, src, width, height, bitCount);
		filter->Process(dst, tmp, height, width, bitCount);
	}
}