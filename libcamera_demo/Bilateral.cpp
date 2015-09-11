#include "Bilateral.h"
#include <math.h>

namespace e
{
#define abs(a) ((a)<(0)?(-a):(a))
#define max(a, b)((a)<(b)?(b):(a))
#define min(a, b)((a)<(b)?(a):(b))

	Bilateral::Bilateral()
	{
		size[0] = 0;
		size[1] = 0;
		radius = 2;
		c_sigma = (radius * 2 + 1) / 2;
		s_sigma = (radius * 2 + 1) * 2;
		c_table = 0;
		s_table = 0;

		Init(5);
	}

	Bilateral::~Bilateral(void)
	{
		int size = 2 * radius + 1;
		if (s_table) delete[] s_table;
		for (int i = 0; i < size; i++)
		{
			if (c_table[i]) delete[] c_table[i];
		}
		if (c_table) delete[] c_table;
	}

	void Bilateral::Init(int d)
	{
		radius = d / 2;
		c_sigma = d / 2;
		s_sigma = d * 2;
		size[0] = 4096 * 3;
		size[1] = 2 * radius + 1;
		s_table = new float[size[0]];
		c_table = new float*[size[1]];
		for (int i = 0; i < size[1]; i++)
		{
			c_table[i] = new float[size[1]];
		}

		CalcParam();
	}

	void Bilateral::SetRadius(int radius)
	{
		this->radius = radius;
	}

	void Bilateral::KeyDown(int key)
	{
		if (key == 37) // 'left'
			c_sigma -= 0.1f;
		else if (key == 39) // 'right'
			c_sigma += 0.1f;
		else if (key == 38) // 'up'
			s_sigma += 0.1f;
		else if (key == 40) // 'down'
			s_sigma -= 0.1f;

		CalcParam();
	}

	void Bilateral::CalcParam(void)
	{
		//init distance weight table
		for (int y = -radius; y <= radius; y++)
		{
			for (int x = -radius; x <= radius; x++)
			{
				double delta = sqrt((float)(y*y + x*x));
				c_table[y + radius][x + radius] = exp(-(delta/(2 * c_sigma * c_sigma)));
			}
		}
		//init similarity weight table
		for (int i = 0; i < size[0]; i++)
		{
			double delta = sqrt((float)(i * i));
			s_table[i] = exp(-(delta/2 * s_sigma * s_sigma));
		}

		scaleIndex = size[0] / (255 * 3);
	}

	void Bilateral::FindMaxMin(uint8* src, int width, int height, int bitCount, uint8 &maxValue, uint8 &minValue)
	{
		int lineBytes = WIDTHBYTES(width * bitCount);
		int bpp = bitCount / 8;

		uint8 _min[3] = { 0 }, _max[3] = { 0 };
		for (int y = 0; y < height; y++)
		{
			uint8* p = src + y * lineBytes;
			for (int x = 0; x < width; x++)
			{
				if (p[0] > _max[0]) _max[0] = p[0];
				if (p[0] < _min[0]) _min[0] = p[0];
				if (p[1] > _max[1]) _max[1] = p[1];
				if (p[1] < _min[1]) _min[1] = p[1];
				if (p[2] > _max[2]) _max[2] = p[2];
				if (p[2] < _min[2]) _min[2] = p[2];
			}
		}

		minValue = min(_min[0], min(_min[1], _min[2]));
		maxValue = max(_min[0], max(_max[1], _max[2]));
	}

	void Bilateral::Process(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma, int mode)
	{
		int lineBytes = WIDTHBYTES(width * bitCount);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* s = src + y * lineBytes;
			uint8* d = dst + y * lineBytes;
			for (int x = 0; x < width; x++)
			{
				int b0 = *(s + 0);
				int g0 = *(s + 1);
				int r0 = *(s + 2);

				float csSum[3] = { 0.0f };
				float pxSum[3] = { 0.0f };
				int roffset = 0, coffset = 0;
				for (int j = -radius; j <= radius; j++)
				{
					roffset = max(0, min(y + j, height - 1));
					for (int i = -radius; i <= radius; i++)
					{
						coffset = max(0, min(x + i, width - 1));
						uint8* p = src + roffset * lineBytes + coffset * bpp;

						int b1 = *(p + 0);
						int g1 = *(p + 1);
						int r1 = *(p + 2);

						float alpha = (abs(b1 - b0) + abs(g1 - g0) + abs(r1 - r0)) * scaleIndex;
						int idx = (int)alpha;
						alpha -= idx;
						float w = c_table[j + radius][i + radius] * (s_table[idx] + alpha * (s_table[idx+1] - s_table[idx]));

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
}