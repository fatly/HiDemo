#pragma once
#include "Defines.h"

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	//---------------------------base filter interface----------------------//
	//////////////////////////////////////////////////////////////////////////
	class FilterBase
	{
	public:
		virtual ~FilterBase(void){};
	public:
		virtual void Process(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount) = 0;
	};

	class Gaussian;
	class BlurFilter
	{
	public:
		BlurFilter(void);
		BlurFilter(float sigma);
		virtual ~BlurFilter(void);
	public:
		void SetSize(int width, int height);
		void SetSigma(float sigma);
		void Process(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);
	protected:
		int width;
		int height;
		uint8* tmp;
		Gaussian* filter;
	};
}
