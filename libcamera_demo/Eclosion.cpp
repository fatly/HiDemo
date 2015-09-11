#include "Eclosion.h"

namespace e
{
	float size = 0.5f;
	void Eclosion(uint8* dst, uint8* src, int width, int height, int bitCount)
	{
		int lineBytes = WIDTHBYTES(width * bitCount);
		int bpp = bitCount / 8;
		int cx = width >> 1;
		int cy = height >> 1;
		int maxV = cx*cx + cy*cy;
		int minV = (int)(maxV * (1 - size));
		int diff = maxV - minV;
		float radio = (width < height) ? ((float)width / (float)height) : ((float)height / (float)width);

		for (int y = 0; y < height; y++)
		{
			uint8* p = src + y * lineBytes;
			uint8* d = dst + y * lineBytes;
			for (int x = 0; x < width; x++)
			{
				float dx = cx - x;
				float dy = cy - y;

				if (width > height)
					dx = dx * radio;
				else
					dy = dy * radio;

				float v = ((dx*dx + dy*dy) / diff) * 255;

				d[0] = clamp0255(p[0] + v);
				d[1] = clamp0255(p[1] + v);
				d[2] = clamp0255(p[2] + v);

				p += bpp;
				d += bpp;
			}
		}
	}
}
