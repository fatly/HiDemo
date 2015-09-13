#ifndef __LIBPITCH_SHIFT_RATETRANSPOSER_H__
#define __LIBPITCH_SHIFT_RATETRANSPOSER_H__
#include "Define.h"
#include "SamplePipe.h"
#include "SampleBuffer.h"
#include "FIRFilter.h"

namespace e
{
	class TransposerBase
	{
	public:
		enum ALGORITHM { LINEAR = 0, CUBIC, SHANNON };
		TransposerBase(void);
		virtual ~TransposerBase(void);
		virtual void SetRate(float rate);
		virtual void SetChannels(int channels);
		virtual int Process(SampleBuffer* dst, SampleBuffer* src);
		static TransposerBase* GetInstance(void);
		static void SetAlgorithm(ALGORITHM type);
	protected:
		virtual void Reset(void) = 0;
		virtual int ProcessMono(sample_t* dst, const sample_t* src, int &samples) = 0;
		virtual int ProcessStereo(sample_t* dst, const sample_t* src, int &samples) = 0;
		static ALGORITHM algorithm;
	public:
		int channels;
		float rate;
	};

	class RateTransposer : public FIFOAdapter
	{
	public:
		RateTransposer(void);
		virtual ~RateTransposer(void);
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
		TransposerBase* transpser;
		SampleBuffer* inputBuffer;
		SampleBuffer* tempBuffer;
		SampleBuffer* outputBuffer;
		bool isUseAAFilter;
	};
}


#endif