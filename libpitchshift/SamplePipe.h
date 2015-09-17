#ifndef __LIBPITCH_SHIFT_SAMPLEPIPE_H__
#define __LIBPITCH_SHIFT_SAMPLEPIPE_H__
#include "Define.h"
#include <assert.h>

namespace e
{
	class SamplePipe
	{
	public:
		virtual ~SamplePipe() {};
	public:
		virtual sample_t* Begin(void) = 0;
		virtual void PutSamples(const sample_t* samples, uint count) = 0;
		virtual uint FetchSamples(uint count) = 0;
		virtual uint FetchSamples(sample_t* output, uint count) = 0;
		virtual uint GetSampleCount(void) const = 0;
		virtual uint AdjustSampleCount(uint count) = 0;
		virtual void MoveSamples(SamplePipe* other)
		{
			assert(other);
			uint count = other->GetSampleCount();
			PutSamples(other->Begin(), count);
			other->FetchSamples(count);
		}
		virtual bool IsEmpty(void) const = 0;
		virtual void Clear(void) = 0;
	};

	class FIFOAdapter : public SamplePipe
	{
	public:
		FIFOAdapter(void)
		{
			output = 0;
		}

		FIFOAdapter(SamplePipe* output)
		{
			SetOutput(output);
		}

		virtual ~FIFOAdapter(void)
		{

		}

		void SetOutput(SamplePipe* output)
		{
			assert(output!=0 && this->output==0);
			this->output = output;
		}

// 		virtual void PutSamples(const sample_t* samples, uint count)
// 		{
// 			assert(output);
// 			output->PutSamples(samples, count);
// 		}

		virtual uint FetchSamples(sample_t* samples, uint count)
		{
			assert(output);
			return output->FetchSamples(samples, count);
		}

		virtual uint FetchSamples(uint count)
		{
			assert(output);
			return output->FetchSamples(count);
		}

		virtual uint GetSampleCount(void) const
		{
			assert(output);
			return output->GetSampleCount();
		}

		virtual bool IsEmpty(void) const
		{
			assert(output);
			return output->IsEmpty();
		}

		virtual uint AdjustSampleCount(uint count)
		{
			assert(output);
			return output->AdjustSampleCount(count);
		}
	protected:
		virtual sample_t* Begin(void)
		{
			assert(output);
			return output->Begin();
		}
	protected:
		SamplePipe* output;
	};
}

#endif