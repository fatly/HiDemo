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

#ifdef INTEGER_CHANNELS
	void Bilateral::Process(uint8* dst, uint8* src, int width, int height, int channels)
	{
		int bitCount = channels * 8;
		int lineBytes = WIDTHBYTES(width*bitCount);
	}
#else
	void Bilateral::Process(float* dst, float* src, int width, int height, int channels)
	{

	}
#endif
}