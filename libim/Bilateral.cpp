#include "Bilateral.h"
#include <assert.h>
#include <math.h>

namespace e
{
	Bilateral::Bilateral(void)
	{
		spaceKernal = 0;
		colorKernal = 0;

		Init(5);
	}

	Bilateral::~Bilateral(void)
	{
		Clear();
	}

	void Bilateral::Init(int d)
	{
		Clear();
		radius = d / 2;
		spaceSigma = d * 2;
		colorSigma = d / 2;
		spaceSize = 2 * radius + 1;
		colorSize = 4096 * 3;
		scaleFactor = colorSize / 255 * 3;
		spaceKernal = new float*[spaceSize];
		assert(spaceKernal);
		for (int i = 0; i < spaceSize; i++)
		{
			spaceKernal[i] = new float[spaceSize];
			assert(spaceKernal[i]);
		}

		colorKernal = new float[colorSize];
		assert(colorKernal);
		CalcKernals();
	}

	void Bilateral::CalcKernals(void)
	{
		for (int y = -radius; y <= radius; y++)
		{
			for (int x = -radius; x <= radius; x++)
			{
				double delta = sqrt((float)(y*y + x*x));
				spaceKernal[y + radius][x + radius] = exp(-(delta / (2 * spaceSigma * spaceSigma)));
			}
		}

		for (int i = 0; i < colorSize; i++)
		{
			double delta = sqrt((float)(i * i));
			colorKernal[i] = exp(-(delta / 2 * colorSigma * colorSigma));
		}
	}

	void Bilateral::Clear(void)
	{
		if (spaceKernal)
		{
			for (int i = 0; i < spaceSize; i++)
			{
				if (spaceKernal[i]) 
					delete[] spaceKernal[i];
				spaceKernal[i] = 0;
			}

			delete[] spaceKernal;
		}

		if (colorKernal)
			delete[] colorKernal;
		colorKernal = 0;

		radius = 0;
		spaceSigma = 0;
		colorSigma = 0;
		spaceSize = 0;
		colorSize = 0;
		scaleFactor = 0;
	}

	void Bilateral::SetSigma(float _spaceSigma, float _colorSigma)
	{
		spaceSigma = _spaceSigma;
		colorSigma = _colorSigma;
		CalcKernals();
	}

	void Bilateral::SetRadius(int _radius)
	{
		assert(radius > 0);
		radius = _radius;
		spaceSize = 2 * radius + 1;
		colorSize = 4096 * 3;
		scaleFactor = colorSize / 255 * 3;
		spaceKernal = new float*[spaceSize];
		assert(spaceKernal);
		for (int i = 0; i < spaceSize; i++)
		{
			spaceKernal[i] = new float[spaceSize];
			assert(spaceKernal[i]);
		}
		colorKernal = new float[colorSize];
		assert(colorKernal);

		CalcKernals();
	}

	void Bilateral::SetSetting(int id, void* value)
	{
		switch (id)
		{
		case ID_SET_RADIUS:
			SetRadius(*((int*)value));
			break;
		case ID_SET_SPACESIGMA:
			SetSigma(*((float*)value), colorSigma);
			break;
		case ID_SET_COLORSIGMA:
			SetSigma(spaceSigma, *((float*)value));
			break;
		default:
			assert(0);
			break;
		}
	}

#ifdef INTEGER_CHANNELS
	void Bilateral::Process(void* _dst, void* _src, int width, int height, int channels)
	{
		assert(_src && _dst);
		assert(width > 0 && height > 0 && channels>0);
		int bpp = channels;
		int lineBytes = WIDTHBYTES(width*channels*8);
		uint8* src = (uint8*)_src, *dst = (uint8*)_dst;

		for (int y = 0; y < height; y++)
		{
			uint8* s = src + y * lineBytes;
			uint8* d = dst + y * lineBytes;
			for (int x = 0; x < width; x++)
			{
				int b0 = s[0], g0 = s[1], r0 =s[2];
				float csSum[3] = {0}, pxSum[3] ={0};
				for (int j = -radius; j <= radius; j++)
				{
					int y1 = MAX(0, MIN(y + j, height - 1));
					for (int i = -radius; i <= radius; i++)
					{
						int x1 = MAX(0, MIN(x + i, width - 1));
						uint8* p = src + y1 * lineBytes + x1 * bpp;
						int b1 = p[0], g1 = p[1], r1 = p[2];

						float alpha = (abs(b1 - b0) + abs(g1 - g0) + abs(r1 - r0)) * scaleFactor;
						int idx = (int)alpha;
						alpha -= idx;
						float w = spaceKernal[j + radius][i + radius] * (colorKernal[idx] + alpha * (colorKernal[idx + 1] - colorKernal[idx]));

						csSum[0] += w;
						csSum[1] += w;
						csSum[2] += w;

						pxSum[0] += (b1 * w);
						pxSum[1] += (g1 * w);
						pxSum[2] += (r1 * w);
					}
				}

				d[0] = clamp0255(pxSum[0] / csSum[0]);
				d[1] = clamp0255(pxSum[1] / csSum[1]);
				d[2] = clamp0255(pxSum[2] / csSum[2]);

				s += bpp;
				d += bpp;
			}
		}
	}
#else
	void Bilateral::Process(void* dst, void* src, int width, int height, int channels)
	{
		assert(0);
	}
#endif
}