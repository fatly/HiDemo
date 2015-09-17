#ifndef __LIBPITCH_SHIFT_SOUNDPITCH_H__
#define __LIBPITCH_SHIFT_SOUNDPITCH_H__

#include "SamplePipe.h"

#define SETTING_USE_AA_FILTER       0
#define SETTING_AA_FILTER_LENGTH    1
#define SETTING_USE_QUICKSEEK       2
#define SETTING_SEQUENCE_MS         3
#define SETTING_SEEKWINDOW_MS       4
#define SETTING_OVERLAP_MS          5

namespace e
{
	class TimeScale;
	class RateScale;
	class SoundPitch : public FIFOAdapter
	{
	public:
		SoundPitch(void);
		virtual ~SoundPitch(void);
		void SetRate(float rate);
		void SetTempo(float tempo);
		void SetPitch(float pitch);
		void SetChannels(uint channels);
		void SetSampleRate(uint rate);
		void PutSamples(const sample_t* samples, uint count);
		uint RemainSamplesCount(void) const;
		bool SetSetting(uint id, int value);
		void Flush(void);
		void Clear(void);
	protected:
		void Calculate(void);
	protected:
		float rate;
		float tempo;
		uint channels;
	private:
		TimeScale* timeScaler;
		RateScale* rateScaler;
		float vRate;
		float vTempo;
		float vPitch;
		bool  inited_sample_rate;
	};
}

#endif;