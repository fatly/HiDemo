#include "Beautify.h"
#include "Bitmap.h"
#include "AutoPtr.h"
#include "BlurFilter.h"
#include "Blender.h"
#include "Curve.h"
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

		curves = new CurvesConfig(18, 256);
		assert(curves);
		InitCurves();
	}


	Beautify::~Beautify(void)
	{
		if (filter) delete filter;
		if (blender) delete blender;
		if (curves) delete curves;
	}

	void Beautify::InitCurves(void)
	{
		double pts[] = {
			0.000000, 0.000000,
			0.125000, 0.150334,
			0.250000, 0.297188,
			0.375000, 0.437080,
			0.500000, 0.566550,
			0.625000, 0.684769,
			0.750000, 0.794540,
			0.875000, 0.898678,
			1.000000, 1.000000,
		};

		assert(curves);
		curves->CreateSpline(CURVE_CHANNEL_C, 9, pts);
		curves->CreateSpline(CURVE_CHANNEL_B, 9, pts);
		curves->CreateSpline(CURVE_CHANNEL_G, 9, pts);
		curves->CreateSpline(CURVE_CHANNEL_R, 9, pts);
		curves->Calculate();
		curves->GetCurve(CURVE_CHANNEL_C)->GetSamples(samples);
	}

	void Beautify::HighPass(uint8* dst, const uint8* src, int width, int height, int channels)
	{
		//blur filter
		filter->Process(dst, src, width, height, channels);
		//blend op
		blender->SetMode(BM_HIGHPASS);
		blender->Process(dst, src, width, height, channels);
	}

	void Beautify::CalcMatte(uint8* dst, const uint8* src, int width, int height, int channels)
	{
		blender->SetMode(BM_HARDLIGHT);
		blender->Process(dst, src, width, height, channels);
	}

	void Beautify::Smooth(uint8* dst, const uint8* src, const uint8* mte, int width, int height, int channels)
	{
		int bpp = channels;
		int bitCount = channels * 8;
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(8 * width);
		channels = channels > 1 ? 3 : channels;

		for (int y = 0; y < height; y++)
		{
			uint8* d = dst + y * lineBytes0;
			const uint8* s = src + y *lineBytes0;
			const uint8* m = mte + y *lineBytes1;

			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					d[c] = *m > 128 ? s[c] : samples[s[c]];
				}

				m += 1;
				d += bpp;
				s += bpp;
			}
		}
	}

	void Beautify::AdjustSample(uint8* dst, const uint8* src, int width, int height, int channels)
	{
		int bpp = channels;
		int bitCount = channels * 8;
		int lineBytes = WIDTHBYTES(bitCount * width);
		channels = bpp > 1 ? 3 : 1;

		for (int y = 0; y < height; y++)
		{
			uint8* d = dst + y * lineBytes;
			const uint8* s = src + y *lineBytes;

			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					d[c] = samples[s[c]];
				}

				d += bpp;
				s += bpp;
			}
		}
	}
}