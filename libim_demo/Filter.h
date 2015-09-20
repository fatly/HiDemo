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
			, const int lineBytes) = 0;
	};


	class BlurFilter : public FilterBase
	{
	public:
		BlurFilter(void);
		virtual ~BlurFilter(void);
	public:
		virtual void Process(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int lineBytes) override;
	protected:
		void Gaussian(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int channels
			, const int lineBytes);
	protected:
		uint8* tmp;
	};
}
