#ifndef __LIBPITCH_SHIFT_SOUNDPITCH_H__
#define __LIBPITCH_SHIFT_SOUNDPITCH_H__

#include "SamplePipe.h"

namespace e
{
	class TDStretch;
	class RateTransposer;
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
		int UnProcessSamples(void) const;
		void Flush(void);
		void Clear(void);
	protected:
		void Calculate(void);
	protected:
		float rate;
		float tempo;
		uint channels;
	private:
		TDStretch* stretch;
		RateTransposer* transpose;
		float vRate;
		float vTempo;
		float vPitch;
		bool  isSrateSet;
	};
}

#endif;