#ifndef __LIBPITCH_SHIFT_INTERPOLATE_H__
#define __LIBPITCH_SHIFT_INTERPOLATE_H__

namespace e
{
	class SampleBuffer;
	class InterpolateImpl;
	class Interpolate
	{
	public:
		Interpolate(void);
		virtual ~Interpolate(void);
	public:
		void SetRate(float rate);
		void SetChannels(int channels);
		int Process(SampleBuffer* dst, SampleBuffer* src);
		int GetChannels(void) const;
		float GetRate(void) const;
	protected:
		InterpolateImpl* impl;
	};

}

#endif