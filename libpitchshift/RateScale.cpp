#include "RateScale.h"
#include "Interpolate.h"

namespace e
{	
	//////////////////////////////////////////////////////////////////////////
	//
	//RateScale
	//
	//////////////////////////////////////////////////////////////////////////
	RateScale::RateScale(void)
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

	RateScale::~RateScale()
	{
		if (inputBuffer) delete inputBuffer;
		if (tempBuffer) delete tempBuffer;
		if (outputBuffer) delete outputBuffer;
		if (transpser) delete transpser;
		if (filter) delete filter;
	}

	RateScale* RateScale::GetInstance(void)
	{
		return new RateScale();
	}

	void RateScale::EnableAAFilter(bool enable)
	{
		isUseAAFilter = enable;
	}

	bool RateScale::IsAAFilterEnable(void) const
	{
		return isUseAAFilter;
	}

	AAFilter* RateScale::GetAAFilter(void) const
	{
		assert(filter);
		return filter;
	}

	SamplePipe* RateScale::GetOutput(void) const
	{
		return outputBuffer;
	}

	void RateScale::SetRate(float rate)
	{
		assert(transpser);
		double frequence = 0;
		transpser->SetRate(rate);

		if (rate > 1.0)
		{
			frequence = 0.5f / rate;
		}
		else
		{
			frequence = 0.5f * rate;
		}

		assert(filter);
		filter->SetCutoffFrequency(frequence);
	}

	void RateScale::SetChannels(int channels)
	{
		assert(channels > 0);
		if (transpser->GetChannels() == channels) return;
		transpser->SetChannels(channels);
		inputBuffer->SetChannels(channels);
		tempBuffer->SetChannels(channels);
		outputBuffer->SetChannels(channels);
	}

	void RateScale::PutSamples(const sample_t* samples, uint count)
	{
		ProcessSamples(samples, count);
	}

	void RateScale::ProcessSamples(const sample_t* src, uint count)
	{
		if (count == 0) return;
		inputBuffer->PutSamples(src, count);

		int ret = 0;
		if (!isUseAAFilter)
		{
			ret = transpser->ProcessSamples(outputBuffer, inputBuffer);
			return;
		}

		assert(filter);
		if (transpser->GetRate() < 1.0f)
		{
			transpser->ProcessSamples(tempBuffer, inputBuffer);
			filter->ProcessSamples(outputBuffer, tempBuffer);
		}
		else
		{
			filter->ProcessSamples(tempBuffer, inputBuffer);
			transpser->ProcessSamples(outputBuffer, tempBuffer);
		}
	}

	bool RateScale::IsEmpty(void) const 
	{
		if (FIFOAdapter::IsEmpty())
			return true;
		else
			return inputBuffer->IsEmpty();
	}

	void RateScale::ClearTempBuffer(void)
	{
		tempBuffer->Clear();
	}

	void RateScale::ClearInputBuffer(void)
	{
		assert(inputBuffer);
		inputBuffer->Clear();
		ClearTempBuffer();
	}

	void RateScale::Clear(void)
	{
		tempBuffer->Clear();
		inputBuffer->Clear();
		outputBuffer->Clear();
	}
}