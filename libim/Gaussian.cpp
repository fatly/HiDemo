#include "Gaussian.h"
#include <math.h>
#include <assert.h>

#pragma warning(disable:4244)

namespace e
{
	inline float CalcParam(float sigma)
	{
		return exp(-(1.695f / sigma));
	}

	Gaussian::Gaussian(void)
	{
		mode = GM_HIGH;
		sigma = 0.1f;
		a0 = a1 = a2 = a3 = 0.0f;
		b1 = b2 = 0.0f;
		coefp = coefn = 0.0f;

		CalcParameters(sigma); 
	}

	Gaussian::~Gaussian(void)
	{

	}

	void Gaussian::SetMode(int mode)
	{
		this->mode = mode;
	}

	void Gaussian::SetSigma(float sigma)
	{
		this->sigma = sigma;
		CalcParameters(sigma);
	}

	void Gaussian::CalcParameters(float sigma)
	{
		assert(sigma >= 0.1f);
		float alpha = 1.695f / sigma;
		float ema = exp(-alpha);
		float ema2 = exp(-2.0f * alpha);
		// pre-compute filter coefficients
		this->sigma = sigma; // note: fSigma is range-checked and clamped >= 0.1f upstream
		this->b1 = -2.0f * ema;
		this->b2 = ema2;

		float k = (1.0f - ema)*(1.0f - ema) / (1.0f + (2.0f * alpha * ema) - ema2);
		this->a0 = k;
		this->a1 = k * (alpha - 1.0f) * ema;
		this->a2 = k * (alpha + 1.0f) * ema;
		this->a3 = -k * ema2;

		this->coefp = (this->a0 + this->a1) / (1.0f + this->b1 + this->b2);
		this->coefn = (this->a2 + this->a3) / (1.0f + this->b1 + this->b2);
	}

	void Gaussian::Process(uint8* dst, const uint8* src, int width, int height, int channels)
	{
		if (mode == GM_SIMPLE)
		{
			if (channels == 1)
				SimpleGaussian8(dst, src, width, height, channels);
			else
				SimpleGaussian24(dst, src, width, height, channels);
		}
		else
		{
			if (channels == 1)
				RecursiveGaussian8(dst, src, width, height, channels);
			else
				RecursiveGaussian24(dst, src, width, height, channels);
		}
	}

	void Gaussian::Process(float* dst, const float* src, int width, int height, int channels)
	{
		if (mode == GM_SIMPLE)
		{
			if (channels == 1)
				SimpleGaussian8(dst, src, width, height, channels);
			else
				SimpleGaussian24(dst, src, width, height, channels);
		}
		else
		{
			if (channels == 1)
				RecursiveGaussian8(dst, src, width, height, channels);
			else
				RecursiveGaussian24(dst, src, width, height, channels);
		}
	}
}


