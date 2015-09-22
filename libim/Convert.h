#ifndef __LIBIM_CONVERT_H__
#define __LIBIM_CONVERT_H__
#include "Defines.h"
#include "XImage.h"
#include "Bitmap.h"
#include <assert.h>

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	void Normalize(XImage<float>& im)
	{
		int width = im.Width();
		int height = im.Height();
		int channels = im.Channels();

		for (int y = 0; y < height; y++)
		{
			float* p = im.Ptr(0, y);
			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					p[c] = p[c] / 255.0f;
				}

				p += channels;
			}
		}
	}

	inline void Normalize(XImage<float>* im)
	{
		assert(im != 0);
		Normalize(*im);
	}

	void INormalize(XImage<float>& im)
	{
		int width = im.Width();
		int height = im.Height();
		int channels = im.Channels();

		for (int y = 0; y < height; y++)
		{
			float* p = im.Ptr(0, y);
			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					p[c] = p[c] * 255.0f;
				}

				p += channels;
			}
		}
	}

	inline void INormalize(XImage<float>* im)
	{
		assert(im);
		INormalize(*im);
	}

	template<class T>
	void B2I(XImage<T>& dst, const Bitmap& src)
	{
		dst.Resize(src.Width(), src.Height(), src.PixelBytes());

		if (sizeof(T) == sizeof(uint8))
		{
			int height = dst.Height();
			int lineBytes = dst.Width() * dst.Channels();
			for (int y = 0; y < height; y++)
			{
				memcpy(dst.Ptr(0, y), src.Get(0, y), lineBytes);
			}
		}
		else
		{
			int width = dst.Width();
			int height = dst.Height();
			int channels = dst.Channels();
			for (int y = 0; y < height; y++)
			{
				T* d = dst.Ptr(0, y);
				uint8* s = src.Get(0, y);
				for (int x = 0; x < width; x++)
				{
					for (int c = 0; c < channels; c++)
					{
						d[c] = static_cast<T>(s[c]);
					}
					d += channels;
					s += channels;
				}
			}
		}
	}

	template<class T>
	void I2B(Bitmap& dst, const XImage<T>& src)
	{
		int width = src.Width();
		int height = src.Height();
		int channels = src.Channels();
		if (!dst.Alloc(width, height, channels * 8))
		{
			assert(0);
			return;
		}

		if (sizeof(T) == sizeof(uint8))
		{
			for (int y = 0; y < height; y++)
			{
				memcpy(src.Ptr(0, y), dst.Get(0, y), width * channels);
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				T* s = src.Ptr(0, y);
				uint8* d = dst.Get(0, y);
				for (int x = 0; x < width; x++)
				{
					for (int c = 0; c < channels; c++)
					{
						d[c] = clamp0255((uint8)(s[c] + 0.5f));
					}

					s += channels;
					d += channels;
				}
			}
		}
	}

	void C2F(XImage<float>& dst, const XImage<uint8>& src)
	{
		assert(src.Width() == dst.Width());
		assert(src.Height() == dst.Height());
		assert(src.Channels() == dst.Channels());

		int width = src.Width();
		int height = src.Height();
		int channels = src.Channels();

		for (int y = 0; y < height; y++)
		{
			float* d = dst.Ptr(0, y);
			const uint8* s = src.Ptr(0, y);
			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					d[c] = s[c];
				}

				s += channels;
				d += channels;
			}
		}
	}
}

#endif