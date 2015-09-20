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
			, const int lineBytes
			, const int channels) = 0;
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
			, const int lineBytes
			, const int channels) override;
	protected:
		void Gaussian(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int lineBytes
			, const int channels);
	protected:
		uint8* tmp;
	};
}
