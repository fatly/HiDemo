#ifndef __LIBPITCH_SHIFT_SAMPLEPIPE_H__
#define __LIBPITCH_SHIFT_SAMPLEPILE_H__
#include "Define.h"

namespace e
{
	class SamplePipe
	{
	public:
		virtual ~SamplePipe() {};
	public:
		virtual sample_t* Begin(void) = 0;
		virtual void PutSamples(uint count) = 0;
		virtual void PutSamples(sample_t* samples, uint count) = 0;
		virtual uint GetSamples(uint count) = 0;
		virtual uint GetSamples(sample_t* output, uint count) = 0;
		virtual uint GetSampleCount(void) const = 0;
		virtual bool IsEmpty(void) const = 0;
		virtual void Clear(void) = 0;
		virtual uint AdjustSampleCount(uint count) = 0;
	};
}


#endif