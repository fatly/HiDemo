#include "SoundPitch.h"
#include "TimeScale.h"
#include "RateScale.h"

#define FE(a,b) (fabs((a)-(b))<(1e-10))

namespace e
{
	SoundPitch::SoundPitch()
	{
		rate = 0;
		tempo = 0;
		channels = 0;
		vRate = 1.0f;
		vTempo = 1.0f;
		vPitch = 1.0f;
		inited_sample_rate = false;

		timeScaler = TimeScale::GetInstance();
		assert(timeScaler);
		rateScaler = RateScale::GetInstance();
		assert(rateScaler);

		FIFOAdapter::SetOutput(timeScaler);

		Calculate();
	}


	SoundPitch::~SoundPitch()
	{
		if (timeScaler) delete timeScaler;
		if (rateScaler) delete rateScaler;
	}

	void SoundPitch::SetRate(float rate)
	{
		vRate = rate;
		Calculate();
	}

	void SoundPitch::SetTempo(float tempo)
	{
		vTempo = tempo;
		Calculate();
	}

	void SoundPitch::SetPitch(float pitch)
	{
		vPitch = pitch;
		Calculate();
	}

	void SoundPitch::SetChannels(uint channels)
	{
		this->channels = channels;
		timeScaler->SetChannels(channels);
		rateScaler->SetChannels(channels);
	}

	void SoundPitch::SetSampleRate(uint rate)
	{
		inited_sample_rate = true;
		timeScaler->SetParameters(rate);
	}

	void SoundPitch::Calculate(void)
	{
		float _rate = rate;
		float _tempo = tempo;
		rate = vPitch * vRate;
		tempo = vTempo / vPitch;

		if (!FE(_rate, rate)) rateScaler->SetRate(rate);
		if (!FE(_tempo, tempo)) timeScaler->SetTempo(tempo);

		if (rate <= 1.0f)
		{
			if (output != timeScaler)
			{
				assert(output == rateScaler);
				SamplePipe* temp = timeScaler->GetOutput();
				temp->MoveSamples(output);
				output = timeScaler;
			}
		}
		else
		{
			if (output != rateScaler)
			{
				assert(output == timeScaler);
				SamplePipe* temp = rateScaler->GetOutput();
				temp->MoveSamples(output);
				rateScaler->MoveSamples(timeScaler->GetInput());
				output = rateScaler;
			}
		}
	}

	void SoundPitch::PutSamples(const sample_t* samples, uint count)
	{
		if (!inited_sample_rate){
			E_THROW("never set sample rate!!!");
		}else if (channels == 0){
			E_THROW("never set number of channel!!!");
		}else if (rate <= 1.0f){
			// transpose the rate down, output the transposed sound to tempo changer buffer
			assert(output == timeScaler);
			rateScaler->PutSamples(samples, count);
			timeScaler->MoveSamples(rateScaler);
		}else {
			// evaluate the tempo changer, then transpose the rate up,
			assert(output == rateScaler);
			timeScaler->PutSamples(samples, count);
			rateScaler->MoveSamples(timeScaler);
		}
	}

	void SoundPitch::Flush(void)
	{
		sample_t* buffer = (sample_t*)malloc(channels * 64 * sizeof(sample_t));
		assert(buffer);
		memset(buffer, 0, channels * 64 * sizeof(sample_t));

		uint remain = RemainSamplesCount();
		remain = (uint)((double)remain / (tempo * rate) + 0.5);
		uint count = GetSampleCount();
		count += remain;

		for (int i = 0; i < 128; i++)
		{
			PutSamples(buffer, 64);
			if (GetSampleCount() >= count)
			{
				AdjustSampleCount(count);
				break;
			}
		}

		free(buffer);
		rateScaler->Clear();
		timeScaler->ClearInputBuffer();
	}

	bool SoundPitch::SetSetting(uint id, int value)
	{
		int sampleRate, sequenceMs, seekWindowMs, overlapMs;

		// read current tdstretch routine parameters
		timeScaler->GetParameters(&sampleRate, &sequenceMs, &seekWindowMs, &overlapMs);

		switch (id)
		{
		case SETTING_USE_AA_FILTER:
			// enables / disabless anti-alias filter
			rateScaler->EnableAAFilter((value != 0) ? true : false);
			return true;

		case SETTING_AA_FILTER_LENGTH:
			// sets anti-alias filter length
			rateScaler->GetAAFilter()->SetLength(value);
			return true;

		case SETTING_USE_QUICKSEEK:
			// enables / disables tempo routine quick seeking algorithm
			timeScaler->EnableQuickSeek((value != 0) ? true : false);
			return true;

		case SETTING_SEQUENCE_MS:
			// change time-stretch sequence duration parameter
			timeScaler->SetParameters(sampleRate, value, seekWindowMs, overlapMs);
			return true;

		case SETTING_SEEKWINDOW_MS:
			// change time-stretch seek window length parameter
			timeScaler->SetParameters(sampleRate, sequenceMs, value, overlapMs);
			return true;

		case SETTING_OVERLAP_MS:
			// change time-stretch overlap length parameter
			timeScaler->SetParameters(sampleRate, sequenceMs, seekWindowMs, value);
			return true;

		default:
			return false;
		}
	}

	uint SoundPitch::RemainSamplesCount(void) const
	{
		if (timeScaler)
		{
			SamplePipe* p = timeScaler->GetInput();

			if (p)
			{
				return p->GetSampleCount();
			}
		}

		return 0;
	}

	void SoundPitch::Clear(void)
	{
		timeScaler->Clear();
		rateScaler->Clear();
	}
}
