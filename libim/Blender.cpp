#include "Blender.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

#define MakeIndex(a, b) ((((a)<<8)|(b)) & 0xffff)

namespace e
{
	static inline uint8 normal(int &a, int &b)
	{
		return a;
	}

	static inline uint8 softlight(int &a, int &b)
	{
		return 0;
	}

	static inline uint8 hardlight(int &a, int &b)
	{
		if (b <= 128)
			return a * b / 128;
		else
			return 255 - (255 - a)*(255 - b) / 128;
	}

	static inline uint8 screen(int &a, int &b)
	{
		return 0;
	}

	static inline uint8 highpass(int &a, int &b)
	{
		return a - b  + 128;
	}

	Blender::Blender(void)
	{
		values = (uint8*)malloc(sizeof(uint8) * (256 * 256));
		SetMode(BM_NORMAL);
	}

	Blender::~Blender(void)
	{
		if (values) free(values);
	}

	void Blender::SetMode(int mode)
	{
		this->mode = mode;
		Update();
	}

	void Blender::GetValues(uint8* results) const
	{
		assert(results);
		memcpy(results, values, sizeof(uint8) * (256 * 256));
	}

	void Blender::GetValues(uint8* results, int mode) 
	{
		SetMode(mode);
		GetValues(results);
	}

	void Blender::Update(void)
	{
		switch (mode)
		{
		case BM_NORMAL:
			Normal();
			break;
		case BM_SOFTLIGHT:
			SoftLight();
			break;
		case BM_HARDLIGHT:
			HardLight();
			break;
		case BM_SCREEN:
			Screen();
			break;
		case BM_HIGHPASS:
			HighPass();
			break;
		default:
			assert(0);
			break;
		}
	}

	void Blender::Normal(void)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int index = MakeIndex(a, b);
				values[index] = normal(a, b);
			}
		}
	}

	void Blender::SoftLight(void)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int index = MakeIndex(a, b);
				values[index] = softlight(a, b);
			}
		}
	}

	void Blender::HardLight(void)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int index = MakeIndex(a, b);
				values[index] = hardlight(a, b);
			}
		}
	}

	void Blender::Screen(void)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int index = MakeIndex(a, b);
				values[index] = screen(a, b);
			}
		}
	}

	void Blender::HighPass(void)
	{
		for (int a = 0; a < 256; a++)
		{
			for (int b = 0; b < 256; b++)
			{
				int index = MakeIndex(a, b);
				values[index] = highpass(a, b);
			}
		}
	}

	void Blender::Process(uint8* dst, const uint8* src, int width, int height, int bitCount)
	{
		int bpp = bitCount / 8;
		int lineBytes = WIDTHBYTES(bitCount * width);
		int channels = bpp > 1 ? 3 : bpp;

		for (int y = 0; y < height; y++)
		{
			const uint8* s = src + y * lineBytes;
			uint8* d = dst + y * lineBytes;
			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					d[c] = values[MakeIndex(s[c], d[c])];
				}

				s += bpp;
				d += bpp;
			}
		}
	}
}