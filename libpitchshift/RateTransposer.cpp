#include "RateTransposer.h"
#include "Interpolate.h"

namespace e
{	
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
		transpser = new Interpolate();
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
		if (transpser) delete transpser;
		if (filter) delete filter;
	}

	RateTransposer* RateTransposer::GetInstance(void)
	{
		return new RateTransposer();
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

	SamplePipe* RateTransposer::GetOutput(void) const
	{
		return outputBuffer;
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
		if (transpser->GetChannels() == channels) return;
		transpser->SetChannels(channels);
		inputBuffer->SetChannels(channels);
		tempBuffer->SetChannels(channels);
		outputBuffer->SetChannels(channels);
	}

	void RateTransposer::PutSamples(const sample_t* samples, uint count)
	{
		ProcessSamples(samples, count);
	}

	void RateTransposer::ProcessSamples(const sample_t* src, uint count)
	{
		if (count == 0) return;
		inputBuffer->PutSamples(src, count);

		int ret = 0;
		if (!isUseAAFilter)
		{
			ret = transpser->Process(outputBuffer, inputBuffer);
			return;
		}

		assert(filter);
		if (transpser->GetRate() < 1.0f)
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