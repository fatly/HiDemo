#include "BlurFilter.h"
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
		assert(filter);
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
}