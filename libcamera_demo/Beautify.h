#ifndef __CORE_BEAUTIFY_H__
#define __CORE_BEAUTIFY_H__
#include "Defines.h"
#include "libutils.h"

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

		String GetParamText(void);

	private:
		uint8 softlight(uint8 a, uint8 b);
		uint8 overlay(uint8 a, uint8 b);
		uint8 screen(uint8 a, uint8 b);
		void Screen(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		void SoftLight(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		void Overlay(uint8* dst, uint8* src, int width, int height, int bitCount, int mode);
		void Reset(void);
		void Preset(void);
	private:

		float sigma;
		float alpha0;
		float alpha1;

		int width;
		int height;

		uint8* blr;
		uint8* tmp;
		uint8* yuv;
		uint8* blend;

		bool enable;
		bool useBlur;
		bool useBlend;
		bool useYCbCr;
		bool swapBlend;
		bool blurLevel;
		int  blendIndex;
		int  blendCount;

		typedef void(Beautify::*BlendHandle)(uint8*, uint8*, int, int, int, int);
		BlendHandle handles[10];
	};
}


#endif