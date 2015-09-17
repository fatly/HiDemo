#ifndef __LIBPITCH_SHIFT_RATETRANSPOSER_H__
#define __LIBPITCH_SHIFT_RATETRANSPOSER_H__
#include "Define.h"
#include "SamplePipe.h"
#include "SampleBuffer.h"
#include "FIRFilter.h"

namespace e
{
	class Interpolate;
	class RateScale : public FIFOAdapter
	{
	public:
		RateScale(void);
		virtual ~RateScale(void);
		static RateScale* GetInstance(void);
	public:
		AAFilter* GetAAFilter(void) const;
		SamplePipe* GetOutput(void) const;
		void EnableAAFilter(bool enable);
		bool IsAAFilterEnable(void) const;
		void SetRate(float rate);
		void SetChannels(int channels);
		void ClearTempBuffer(void);
		void ClearInputBuffer(void);
		virtual void PutSamples(const sample_t* samples, uint count) override;
		virtual bool IsEmpty(void) const override;
		virtual void Clear(void) override;
	protected:
		void ProcessSamples(const sample_t* src, uint count);
	protected:
		AAFilter* filter;
		Interpolate* transpser;
		SampleBuffer* inputBuffer;
		SampleBuffer* tempBuffer;
		SampleBuffer* outputBuffer;
		bool isUseAAFilter;
	};
}


#endif