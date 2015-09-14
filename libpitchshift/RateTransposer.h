#ifndef __LIBPITCH_SHIFT_RATETRANSPOSER_H__
#define __LIBPITCH_SHIFT_RATETRANSPOSER_H__
#include "Define.h"
#include "SamplePipe.h"
#include "SampleBuffer.h"
#include "FIRFilter.h"

namespace e
{
	class Interpolate;
	class RateTransposer : public FIFOAdapter
	{
	public:
		RateTransposer(void);
		virtual ~RateTransposer(void);
		static RateTransposer* GetInstance(void);
	public:
		AAFilter* GetAAFilter(void) const;
		SamplePipe* GetOutput(void) const;
		void EnableAAFilter(bool enable);
		bool IsAAFilterEnable(void) const;
		virtual void SetRate(float rate);
		void SetChannels(int channels);
		void PutSamples(const sample_t* samples, uint count);
		bool IsEmpty(void) const;
		void Clear(void);
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