#include "BlurFilter.h"
#include <stdlib.h>
#include <assert.h>
#include "Gaussian.h"
#include "Bilateral.h"

namespace e
{
	BlurFilter::BlurFilter(void)
	{
		type = -1;
		SetType(BT_BILATERAL);

		int mode = GM_HIGH;
		float sigma = 1.5f;
		filter->SetSetting(ID_SET_MODE, &mode);
		filter->SetSetting(ID_SET_SIGMA, &sigma);
	}

	BlurFilter::BlurFilter(float sigma)
	{
		filter = new Gaussian();
		assert(filter);

		int mode = GM_HIGH;
		filter->SetSetting(ID_SET_MODE, &mode);
		filter->SetSetting(ID_SET_SIGMA, &sigma);
	}

	BlurFilter::~BlurFilter(void)
	{
		if (filter) delete filter;
	}

	void BlurFilter::SetType(int _type)
	{
		if (type == _type) return;
		if (filter) delete filter;

		if (_type == BT_GAUSSIAN){
			filter = new Gaussian();
		}else{
			filter = new Bilateral();
		}
		
		type = _type;
	}

	void BlurFilter::SetSetting(int id, void* value)
	{
		assert(filter);
		if (id == ID_SET_BLURTYPE)
		{
			SetType(*((int*)value));
		}
		else
		{
			filter->SetSetting(id, value);
		}
	}

	void BlurFilter::Process(void* dst, void* src, int width, int height, int channels)
	{
		assert(filter);
		filter->Process(dst, src, width, height, channels);
	}
}