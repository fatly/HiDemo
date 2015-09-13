#include "RateTransposer.h"

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	//
	//TransposerBase implements
	//
	//////////////////////////////////////////////////////////////////////////
	TransposerBase::ALGORITHM TransposerBase::algorithm = TransposerBase::CUBIC;
	TransposerBase::TransposerBase(void)
	{
		rate = 1.0f;
		channels = 0;
	}

	TransposerBase::~TransposerBase(void)
	{

	}

	int TransposerBase::Process(SampleBuffer* dst, SampleBuffer* src)
	{
		int samples = src->GetSampleCount();
		int todo = (int)((float)samples / rate) + 8;
		sample_t* s = src->Begin();
		sample_t* e = dst->End(todo);

		int count = 0;
		if (channels == 1)
		{
			count = ProcessMono(e, s, samples);
		}
		else if (channels == 2)
		{
			count = ProcessStereo(e, s, samples);
		}

		dst->PutSamples(count);
		src->GetSamples(count);
		return count;
	}

	void TransposerBase::SetRate(float rate)
	{
		this->rate = rate;
	}

	void TransposerBase::SetChannels(int channels)
	{
		this->channels = channels;
		Reset();
	}

	void TransposerBase::SetAlgorithm(ALGORITHM type)
	{
		algorithm = type;
	}
	
	TransposerBase* TransposerBase::GetInstance(void)
	{
		// TODO : 
#ifdef INTEGER_SAMPLES
		assert(0);
#else
		assert(0);
		switch (algorithm)
		{
		case LINEAR:

			break;
		case CUBIC:

			break;
		case SHANNON:

			break;
		default:
			assert(0);
			return 0;
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//RateTransposer
	//
	//////////////////////////////////////////////////////////////////////////
	RateTransposer::RateTransposer(void)
	{
		isUseAAFilter = true;
		filter = new AAFilter(64);
		assert(filter);
		transpser = TransposerBase::GetInstance();
		assert(transpser);

		inputBuffer = new SampleBuffer();
		assert(inputBuffer);
		tempBuffer = new SampleBuffer();
		assert(tempBuffer);
		outputBuffer = new SampleBuffer();
		assert(outputBuffer);
		//set output buffer of base class
		FIFOAdapter::SetOutput(outputBuffer);
	}

	RateTransposer::~RateTransposer()
	{
		if (inputBuffer) delete inputBuffer;
		if (tempBuffer) delete tempBuffer;
		if (outputBuffer) delete outputBuffer;
		if (filter) delete filter;
		if (transpser) delete transpser;
	}

	void RateTransposer::EnableAAFilter(bool enable)
	{
		isUseAAFilter = enable;
	}

	bool RateTransposer::IsAAFilterEnable(void) const
	{
		return isUseAAFilter;
	}

	AAFilter* RateTransposer::GetAAFilter(void) const
	{
		assert(filter);
		return filter;
	}

	void RateTransposer::SetRate(float rate)
	{
		assert(transpser);
		double frequence = 0;
		transpser->SetRate(rate);

		if (rate > 1.0)
		{
			frequence = 0.5 / rate;
		}
		else
		{
			frequence = 0.5 * rate;
		}

		assert(filter);
		filter->SetCutoffFrequency(frequence);
	}

	void RateTransposer::SetChannels(int channels)
	{
		assert(channels > 0);
		if (transpser->channels == channels) return;
		transpser->SetChannels(channels);
		inputBuffer->SetChannels(channels);
		tempBuffer->SetChannels(channels);
		outputBuffer->SetChannels(channels);
	}

	void RateTransposer::PutSamples(const sample_t* samples, uint count)
	{
		if (count == 0) return;
		inputBuffer->PutSamples(samples, count);

		int ret = 0;
		if (!isUseAAFilter)
		{
			ret = transpser->Process(outputBuffer, inputBuffer);
			return;
		}

		assert(filter);
		if (transpser->rate < 1.0f)
		{
			transpser->Process(tempBuffer, inputBuffer);
			filter->Process(outputBuffer, tempBuffer);
		}
		else
		{
			filter->Process(tempBuffer, inputBuffer);
			transpser->Process(outputBuffer, tempBuffer);
		}
	}

	bool RateTransposer::IsEmpty(void) const 
	{
		if (FIFOAdapter::IsEmpty())
			return true;
		else
			return inputBuffer->IsEmpty();
	}

	void RateTransposer::Clear(void)
	{
		outputBuffer->Clear();
		tempBuffer->Clear();
		inputBuffer->Clear();
	}
}