#include "Resample.h"
#include <assert.h>

namespace e
{

#define div4(x) ((x) >> 2)
#define div16(x) ((x) >> 4)


	Resample::Resample()
	{
		fnResample = 0;
	}

	Resample::~Resample()
	{

	}

	void Resample::BindHandle(int index)
	{
		switch (index)
		{
		case 0:
			fnResample = &Resample::ResampleRow1;
			break;
		case 1:
			fnResample = &Resample::ResampleRowV2;
			break;
		case 2:
			fnResample = &Resample::ResampleRowH2;
			break;
		case 3:
			fnResample = &Resample::ResampleRowHV2;
			break;
		case 4:
			fnResample = &Resample::ResampleRowGeneric;
			break;
		default:
			assert(0);
			break;
		}
	}

	uint8* Resample::operator()(uint8* out, uint8* in_near, uint8* in_far, int w, int hs)
	{
		if (fnResample)
		{
			return (this->*fnResample)(out, in_near, in_far, w, hs);
		}
		else
		{
			assert(0);
		}

		return 0;
	}

	uint8* Resample::ResampleRow1(uint8* out, uint8* in_near, uint8* in_far, int w, int hs)
	{
		return in_near;
	}

	uint8* Resample::ResampleRowV2(uint8* out, uint8* in_near, uint8* in_far, int w, int hs)
	{
		for (int i = 0; i < w; i++)
		{
			out[i] = div4(3 * in_near[i] + in_far[i] + 2);
		}
		return out;
	}

	uint8* Resample::ResampleRowH2(uint8* out, uint8* in_near, uint8* in_far, int w, int hs)
	{
		uint8* input = in_near;

		if (w == 1)
		{
			out[0] = out[1] = input[0];
			return out;
		}

		out[0] = input[0];
		out[1] = div4(input[0] * 3 + input[1] + 2);
		int i = 0;
		for (i = 1; i < w - 1; i++)
		{
			int n = input[i] * 3 + 2;
			out[i * 2 + 0] = div4(n + input[i - 1]);
			out[i * 2 + 1] = div4(n + input[i + 1]);
		}

		out[i * 2 + 0] = div4(input[w - 2] * 3 + input[w - 1] + 2);
		out[i * 2 + 1] = input[w - 1];

		return out;
	}

	uint8* Resample::ResampleRowHV2(uint8* out, uint8* in_near, uint8* in_far, int w, int hs)
	{
		if (w == 1)
		{
			out[0] = out[1] = div4(3 * in_near[0] + in_far[0] + 2);
			return out;
		}

		int t1 = 3 * in_near[0] + in_far[0];
		out[0] = div4(t1 + 2);

		for (int i = 1; i < w; i++)
		{
			int t0 = t1;
			t1 = 3 * in_near[i] + in_far[i];
			out[i * 2 - 1] = div16(3 * t0 + t1 + 8);
			out[i * 2 + 0] = div16(3 * t1 + t0 + 8);
		}

		out[w * 2 - 1] = div4(t1 + 2);

		return out;
	}

	uint8* Resample::ResampleRowGeneric(uint8* out, uint8* in_near, uint8* in_far, int w, int hs)
	{
		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < hs; j++)
			{
				out[i * hs + j] = in_near[i];
			}
		}

		return out;
	}
}
