#ifndef __LIBIM_BLENDER_H__
#define __LIBIM_BLENDER_H__

#include "Defines.h"
#define MakeIndex(a, b) ((((a)<<8)|(b)) & 0xffff)

namespace e
{
	typedef enum{
		BM_NORMAL,
		BM_SOFTLIGHT,
		BM_HARDLIGHT,
		BM_SCREEN, 
		BM_HIGHPASS
	}BlendMode;

	class Blender 
	{
	public:
		Blender(void);
		virtual ~Blender(void);
	public:
		void SetMode(int mode);
		void GetValues(uint8* results) const;
		void GetValues(uint8* results, int mode);

		template<class T>
		void Process(T* dst, const T* src, int width, int height, int channels)
		{
			int bitCount = channels * sizeof(T) * 8;
			int lineBytes = WIDTHBYTES(bitCount * width);
			channels = channels > 1 ? 3 : channels;

			for (int y = 0; y < height; y++)
			{
				const T* s = src + y * lineBytes;
				T* d = dst + y * lineBytes;
				for (int x = 0; x < width; x++)
				{
					for (int c = 0; c < channels; c++)
					{
						d[c] = values[MakeIndex(s[c], d[c])];
					}

					s += channels;
					d += channels;
				}
			}
		}
	protected:
		void Update(void);
		void Normal(void);
		void SoftLight(void);
		void HardLight(void);
		void Screen(void);
		void HighPass(void);
	private:
		int mode;
		uint8* values;
	};
}

#endif