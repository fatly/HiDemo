#include "SoundPitch.h"
#include "TDStretch.h"
#include "RateTransposer.h"

#define FE(a,b) ((a)-(b)<(1e-10))

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
		isSrateSet = false;

		stretch = TDStretch::GetInstance();
		assert(stretch);
		transpose = RateTransposer::GetInstance();
		assert(transpose);

		SetOutput(stretch);

		Calculate();
	}


	SoundPitch::~SoundPitch()
	{
		if (stretch) delete stretch;
		if (transpose) delete transpose;
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
		stretch->SetChannels(channels);
		transpose->SetChannels(channels);
	}

	void SoundPitch::SetSampleRate(uint rate)
	{
		isSrateSet = true;
		stretch->SetParamters(rate);
	}

	void SoundPitch::Calculate(void)
	{
		float _rate = rate;
		float _tempo = tempo;
		rate = vPitch * vRate;
		tempo = vTempo / vPitch;

		if (!FE(_rate, rate)) transpose->SetRate(rate);
		if (!FE(_tempo, tempo)) stretch->SetTempo(tempo);

		if (rate <= 1.0f)
		{
			if (output != stretch)
			{
				assert(output == transpose);
				SamplePipe* temp = stretch->GetOutput();
				temp->MoveSamples(output);
				output = stretch;
			}
		}
		else
		{
			if (output != transpose)
			{
				assert(output == stretch);
				SamplePipe* temp = transpose->GetOutput();
				temp->MoveSamples(output);
				transpose->MoveSamples(stretch->GetInput());
				output = transpose;
			}
		}
	}

	void SoundPitch::PutSamples(const sample_t* samples, uint count)
	{
		if (!isSrateSet){
			E_THROW("never set sample rate!!!");
		}else if (channels == 0){
			E_THROW("never set number of channel!!!");
		}else if (rate <= 1.0f){
			assert(output == stretch);
			transpose->PutSamples(samples, count);
			stretch->MoveSamples(transpose);
		}else {
			assert(output == transpose);
			stretch->PutSamples(samples, count);
			transpose->MoveSamples(stretch);
		}
	}

	void SoundPitch::Flush(void)
	{
		sample_t* buffer = (sample_t*)malloc(channels * 64 * sizeof(sample_t));
		assert(buffer);
		memset(buffer, 0, channels * 64 * sizeof(sample_t));

		int count = UnProcessSamples();
		count = (int)((double)count / (tempo * rate) + 0.5);
		int samples = GetSampleCount();
		samples += count;

		for (int i = 0; i < 128; i++)
		{
			PutSamples(buffer, 64);
			if ((int)GetSampleCount() >= samples)
			{
				AdjustSampleCount(samples);
				break;
			}
		}

		free(buffer);
		transpose->Clear();
		stretch->ClearInputBuffer();
	}

	int SoundPitch::UnProcessSamples(void) const
	{
		if (stretch)
		{
			SamplePipe* p = stretch->GetInput();

			if (p)
			{
				return p->GetSampleCount();
			}
		}

		return 0;
	}

	void SoundPitch::Clear(void)
	{
		stretch->Clear();
		transpose->Clear();
	}
}
