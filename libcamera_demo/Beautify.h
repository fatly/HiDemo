#ifndef __CORE_BEAUTIFY_H__
#define __CORE_BEAUTIFY_H__
#include "Defines.h"

namespace e
{
	class Beautify
	{
	public:
		Beautify(int width, int height);
		virtual ~Beautify(void);
	public:
		void Process(uint8* buffer, int width, int height, int bitCount);

		void KeyDown(int key);

		float GetSigma(void) const
		{
			return sigma;
		}

		float GetAlpha0(void) const
		{
			return alpha0;
		}

		float GetAlpha1(void) const
		{
			return alpha1;
		}

	private:
		uint8 softlight(uint8 a, uint8 b);
		uint8 overlying(uint8 a, uint8 b);
		void Screen(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		void SoftLight(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		void Different(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		void Overlying(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		typedef void(Beautify::*BlendHandle)(uint8*, uint8*, int, int, int, int);
		void Reset(void);
	private:

		float sigma;
		float alpha0;
		float alpha1;

		int width;
		int height;

		uint8* blr;
		uint8* tmp;
		uint8* yuv;

		bool enable;
		bool useBlur;
		bool useBlend;
		bool useYCbCr;
		bool swapBlend;
		bool blurLevel;
		int  blendIndex;
		int  blendCount;
		BlendHandle handles[10];
	};
}


#endif