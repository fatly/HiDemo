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
		void Screen(uint8* dst, uint8* src, int width, int height, int bitCount);
		void SoftLight(uint8* dst, uint8* src, int width, int height, int bitCount);
		typedef void(Beautify::*BlendHandle)(uint8*, uint8*, int, int, int);
	private:
		float sigma;
		int width;
		int height;

		float alpha0;
		float alpha1;

		uint8* src;
		uint8* tmp;

		bool enable;
		bool mode;
		BlendHandle blendHandle;
	};
}


#endif