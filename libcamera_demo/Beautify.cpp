#include "Beautify.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <memory.h>
#include "FastBlur.h"
#include "Gaussian.h"
#include "Eclosion.h"
#include "libutils.h"
#include "ColorSpace.h"
#include "Bilateral.h"
#include <windows.h>

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#else
#pragma comment(lib, "libutils.lib")
#endif

#pragma warning(disable:4244)

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))

namespace e
{
	//1.0 - (1.0 - a) * (1.0 - b);	
	inline uint8 Beautify::screen(uint8 a, uint8 b)
	{
		return clamp0255(255 - (255 - a) * (255 - b) / 255);
	}

	void Beautify::Screen(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = screen(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = screen(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = screen(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

// 					*(p1 + 0) = softlight(*(p1 + 0), screen(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0));
// 					*(p1 + 1) = softlight(*(p1 + 1), screen(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0));
// 					*(p1 + 2) = softlight(*(p1 + 2), screen(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					//screen op
					*p1 = screen(*p1, *p0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	inline float g(float a)
	{
		if (a < 0.25f)
		{
			return ((16 * a - 12) * a + 4) * a;
		}
		else
		{
			return sqrt(a);
		}
	}

	inline uint8 Beautify::softlight(uint8 _a, uint8 _b)
	{
//		if (this->swapBlend) swap(_a, _b);

// 		if (b <= 128)
// 		{
// 			return clamp0255((a * b) / 128 + (a / 255) * (a / 255) * (255 - 2 * b));
// 		}
// 		else
// 		{
// 			return clamp0255(a * (255 - b) / 128 + sqrt((float)a / 255.0f) * (2 * b - 255));
// 		}		

		float a = (float)_a / 255.0f, b = (float)_b / 255.0f;

		if (b < 0.5f)
		{
			return (a - (1 - 2 * b) * a * (1 - a)) * 255;
		}
		else
		{
			return (a + (2 * b - 1) * (g(a) - a)) * 255;
		}
	}

	void Beautify::SoftLight(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
// 					*(p1 + 0) = softlight(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
// 					*(p1 + 1) = softlight(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
// 					*(p1 + 2) = softlight(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

#define BLEND(a, b) blend[(int(a)<<8 | int(b)) & 0xffff]
					*(p1 + 0) = BLEND(*(p1 + 0), *(p0 + 0));
					*(p1 + 1) = BLEND(*(p1 + 1), *(p0 + 1));
					*(p1 + 2) = BLEND(*(p1 + 2), *(p0 + 2));

// 					*(p1 + 0) = BLEND(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
// 					*(p1 + 1) = BLEND(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
// 					*(p1 + 2) = BLEND(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = softlight(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	inline uint8 Beautify::hardlight(uint8 a, uint8 b)
	{
		if (b <= 128)
			return clamp0255((a * b) / 128);
		else
			return clamp0255(255 - (255 - a)*(255 - b) / 128);
	}

	void Beautify::HardLight(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = hardlight(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = hardlight(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = hardlight(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = hardlight(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	inline uint8 Beautify::overlay(uint8 a, uint8 b)
	{
		if (this->swapBlend) swap(a, b);

		if (a < 128)
		{
			return a * b / 128;
		}
		else
		{
			return 255 - (255 - a) * (255 - b) / 128;
		}
	}

	void Beautify::Overlay(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = overlay(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = overlay(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = overlay(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);
// 					*(p1 + 0) = softlight(*(p1 + 0), overlay(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0));
// 					*(p1 + 1) = softlight(*(p1 + 1), overlay(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0));
// 					*(p1 + 2) = softlight(*(p1 + 2), overlay(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = overlay(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	int R = 11;
	inline uint8 Beautify::highpass(uint8 a, uint8 b)
	{
		uint8 v = a - b;
		return 128 + abs(v) * v / (2 * R);
	}

	void Beautify::HighPass(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = highpass(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = highpass(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = highpass(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = highpass(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	inline uint8 Beautify::multiply(uint8 a, uint8 b)
	{
		return a * b / 255;
	}

	void Beautify::Multiply(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = multiply(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = multiply(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = multiply(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

// 					*(p1 + 0) = multiply(*(p1 + 0), screen(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0));
// 					*(p1 + 1) = multiply(*(p1 + 1), screen(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0));
// 					*(p1 + 2) = multiply(*(p1 + 2), screen(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = multiply(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	inline uint8 Beautify::vividlight(uint8 a, uint8 b)
	{
		if (b <= 128)
			return clamp0255(a - (255 - a) * (255 - 2 * b) / (2 * b));
		else
			return clamp0255(a + a *(2 * b - 255) / (2 * (255 - b)));

//		return clamp0255(a - (255 - a)*(255 - b) / b);

//		return clamp0255(a + (a * b) / (255 - b));
		
//		return clamp0255(a + b - (a * b) / 128);
	}

	void Beautify::VividLight(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = vividlight(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = vividlight(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = vividlight(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = vividlight(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	Beautify::Beautify(int width, int height)
	{
		this->sigma = 5.0f;
		this->width = width;
		this->height = height;
		this->enable = true;
		this->useBlur = true;
		this->useBlend = true;
		this->useYCbCr = false;
		this->swapBlend = false;
		this->blurLevel = true;
		this->blurIndex = 0;
		this->blr = new uint8[width * height * 4];
		this->tmp = new uint8[width * height * 4];
		this->yuv = new uint8[width * height * 4];
		this->blend = new uint8[256 * 256];
		this->alpha0 = 1.0f;
		this->alpha1 = 1.0f;
		this->blendIndex = 0;
		this->blendCount = 6;
		int i = 0;
		this->handles[i++] = &Beautify::Overlay;
		this->handles[i++] = &Beautify::SoftLight;
		this->handles[i++] = &Beautify::HardLight;
		this->handles[i++] = &Beautify::Screen;
		this->handles[i++] = &Beautify::Multiply;
		this->handles[i++] = &Beautify::VividLight;
		this->handles[i++] = &Beautify::HighPass;

		this->filter = new Bilateral();

		Preset();
	}

	Beautify::~Beautify(void)
	{
		if (blr)
		{
			delete[] blr;
		}

		if (tmp)
		{
			delete[] tmp;
		}

		if (yuv)
		{
			delete[] yuv;
		}

		if (blend)
		{
			delete[] blend;
		}

		if (filter)
		{
			delete filter;
		}
	}

	void Beautify::Reset(void)
	{
		this->sigma = 3.0f;
		this->enable = true;
		this->useBlur = true;
		this->useBlend = true;
		this->useYCbCr = false;
		this->swapBlend = false;
		this->blurLevel = true;
		this->blurIndex = 0;
		this->alpha0 = 1.0f;
		this->alpha1 = 1.0f;
		this->blendIndex = 0;
	}

	void Beautify::Preset(void)
	{
		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < 256; j++)
			{
				int index = ((i << 8) | j) & 0xffff;
				blend[index] = softlight(i, j);
			}
		}
	}

	void Beautify::Process(uint8* buffer, int width, int height, int bitCount)
	{
		if (!enable) return;

		int bytes = WIDTHBYTES(width * bitCount) * height;

		if (!useYCbCr)
		{
			memcpy(blr, buffer, bytes);

			if (useBlur)
			{
				DWORD s = GetTickCount();

				if (blurIndex == 0)
				{
					Gaussian(tmp, blr, width, height, bitCount, sigma, blurLevel ? HIGHT24 : SIMPLE24);
					Gaussian(blr, tmp, height, width, bitCount, sigma, blurLevel ? HIGHT24 : SIMPLE24);
				}
				else
				{
					FastBlur(tmp, blr, width, height, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
					FastBlur(blr, tmp, height, width, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
				}

				DWORD t = GetTickCount() - s;
				TCHAR buf[32] = { 0 };
				_stprintf_s(buf, _T("dt = %u\n"), t);
				OutputDebugString(buf);
			}
// 			else
// 			{
// 				Eclosion(blr, buffer, width, height, bitCount);
// 			}

			if (useBlend)
			{
				(this->*handles[blendIndex])(buffer, blr, width, height, bitCount, HIGHT24);
			}
			else
			{
				memcpy(buffer, blr, bytes);
			}
		}
		else
		{
			ColorSpace::BGRA2YCC(yuv, buffer, width, height, bitCount);

			memcpy(blr, yuv, bytes);

			if (useBlur)
			{
				if (blurIndex == 0)
				{
					Gaussian(tmp, blr, width, height, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
					Gaussian(blr, tmp, height, width, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
				}
				else
				{
					FastBlur(tmp, blr, width, height, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
					FastBlur(blr, tmp, height, width, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
				}
			}

			if (useBlend)
			{
				(this->*handles[blendIndex])(yuv, blr, width, height, bitCount, HIGHT8);
			}
			else
			{
				memcpy(yuv, blr, bytes);
			}

			ColorSpace::YCC2BGRA(buffer, yuv, width, height, bitCount);
		}
	}

	void Beautify::KeyDown(int key)
	{
		if (key == 65) sigma -= 0.02f;				//	'a'
		else if (key == 68) sigma += 0.02f;			//	'd'
		else if (key == 37) alpha0 -= 0.02f;		//	'left'
		else if (key == 39) alpha0 += 0.02f;		//	'right'
		else if (key == 40) alpha1 -= 0.02f;		//	'down'
		else if (key == 38) alpha1 += 0.02f;		//	'up'
		else if (key == 66) useBlur = !useBlur;		//	'b'
		else if (key == 70) useBlend = !useBlend;	//	'f'
		else if (key == 89) useYCbCr = !useYCbCr;	//	'y'
		else if (key == 76) blurLevel = !blurLevel;	//	'l'
		else if (key == 69)	enable = !enable;		//	'e'
		else if (key == 88) swapBlend = !swapBlend;	//	'x'
		else if (key == 73) blurIndex = (blurIndex + 1) % 2;	// 'i'
		else if (key == 87) blendIndex = (blendIndex + 1) % blendCount;	// 'w'
		else if (key == 81) blendIndex = (blendIndex - 1) % blendCount; // 'q'
		else if (key == 82) Reset();

		if (sigma < 0.1f) sigma = 0.1f;
		if (alpha0 > 1.0f) alpha0 = 1.0f;
		if (alpha0 < 0.0f) alpha0 = 0.0f;
		if (alpha1 > 1.0f) alpha1 = 1.0f;
		if (alpha1 < 0.0f) alpha1 = 0.0f;
		if (blendIndex < 0) blendIndex = 0;

		filter->KeyDown(key);
	}

	String Beautify::GetParamText()
	{
		String result, line;

		TCHAR szBlend[10][256] = { 
			TEXT("overlay"), 
			TEXT("softlight"), 
			TEXT("hardlight"),
			TEXT("screen"), 
			TEXT("multiply"), 
			TEXT("vividlight"), 
			TEXT("highpass") 
		};

		line = String::Format(TEXT("sigma = %f\n"), sigma);
		result.Append(line);
		line = String::Format(TEXT("alpha0 = %f\n"), alpha0);
		result.Append(line);
		line = String::Format(TEXT("alpha1 = %f\n"), alpha1);
		result.Append(line);
		line = String::Format(TEXT("useBlur = %s\n"), useBlur ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("useBlend = %s\n"), useBlend ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("useYCbCr = %s\n"), useYCbCr ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("blurLevel = %s\n"), blurLevel ? TEXT("hight") : TEXT("simple"));
		result.Append(line);
		line = String::Format(TEXT("blurIndex = %s\n"), blurIndex ? TEXT("SSE-Blur") : TEXT("CPU-Blur"));
		result.Append(line);
		line = String::Format(TEXT("swapBlend = %s\n"), swapBlend ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("blendMode = %s\n"), szBlend[blendIndex]);
		result.Append(line);
		line = String::Format(TEXT("enable = %s\n"), enable ? TEXT("true") : TEXT("false"));
		result.Append(line);

		return result;
	}
}

