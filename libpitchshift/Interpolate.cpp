#include "Interpolate.h"
#include <assert.h>
#include "RateScale.h"

#define SCALE 65536

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	//
	//implements of interpolateimpl
	//
	//////////////////////////////////////////////////////////////////////////
	class InterpolateImpl
	{
	public:
		InterpolateImpl(void);
		virtual ~InterpolateImpl(void);
	public:
		void SetRate(float rate);
		void SetChannels(int channels);
		int ProcessSamples(SampleBuffer* dst, SampleBuffer* src);
		int GetChannels(void) const { return channels; }
		float GetRate(void) const {	return fRate; }
	protected:
		void Reset(void);
		int ProcessMono(sample_t* dst, const sample_t* src, int &samples);
		int ProcessStereo(sample_t* dst, const sample_t* src, int &samples);
	protected:
		float fRate;
		int channels;
#ifdef INTEGER_SAMPLES
		int iRate;
		int iFract;
#else
		float fFract;
#endif
	};

	InterpolateImpl::InterpolateImpl(void)
	{
		fRate = 1.0;
		channels = 0;
		Reset();
		SetRate(1.0f);
	}

	InterpolateImpl::~InterpolateImpl(void)
	{

	}

	void InterpolateImpl::SetChannels(int channels)
	{
		this->channels = channels;
		Reset();
	}

	void InterpolateImpl::SetRate(float rate)
	{
		iRate = (int)(rate * SCALE + 0.5);
		fRate = rate;
	}

	int InterpolateImpl::ProcessSamples(SampleBuffer* dst, SampleBuffer* src)
	{
		int samples = src->GetSampleCount();
		int todo = (int)((float)samples / fRate) + 8;
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
		src->FetchSamples(samples);
		return count;
	}

#ifdef INTEGER_SAMPLES
	void InterpolateImpl::Reset(void)
	{
		iFract = 0;
	}

	int InterpolateImpl::ProcessMono(sample_t* dst, const sample_t* src, int &samples)
	{
		int i = 0, j = 0;
		for (; j < samples-1;)
		{
			lsample_t temp = (SCALE - iFract) * src[0] + iFract * src[1];
			dst[i++] = (sample_t)(temp / SCALE);

			iFract += iRate;

			int whole = iFract / SCALE;
			iFract -= whole * SCALE;
			j += whole;
			src += whole;
		}

		samples = j;
		return i;
	}

	int InterpolateImpl::ProcessStereo(sample_t* dst, const sample_t* src, int &samples)
	{
		int i = 0, j= 0;
		for (; j<samples-1;)
		{
			lsample_t temp0 = (SCALE - iFract) * src[0] + iFract * src[2];
			lsample_t temp1 = (SCALE - iFract) * src[1] + iFract * src[3];
			dst[0] = (sample_t)(temp0 / SCALE);
			dst[1] = (sample_t)(temp1 / SCALE);
			dst += 2;
			i++;

			iFract += iRate;

			int whole = iFract / SCALE;
			iFract -= whole * SCALE;
			j += whole;
			src += 2 * whole;
		}

		samples = j;
		return i;
	}
#elif defined(FLOAT_SAMPLES)	//FLOAT_SAMPLES
	static const float g_coeffs[] = {
		-0.5f, 1.0f, -0.5f, 0.0f, 1.5f, -2.5f, 0.0f, 1.0f,
		-1.5f, 2.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f
	};

	void InterpolateImpl::Reset(void)
	{
		fFract = 0.0f;
	}

	int InterpolateImpl::ProcessMono(sample_t* dst, const sample_t* src, int &samples)
	{
//linear interpolate
#ifdef INTERPOLATE_LINEAR
		int i = 0, j = 0;
		for (; j<samples-1;)
		{
			double temp = (1.0 - fFract) * src[0] + fFract * src[1];
			dst[i++] = (sample_t)temp;

			fFract += rate;
			int whole = (int)fFract;
			fFract -= whole;
			src += whole;
			j += whole;
		}

		samples = j;
		return i;
//cubio interpolate
#elif defined(INTERPOLATE_CUBIC)
		const float x3 = 1.0f;
		const float x2 = fFract;
		const float x1 = x2 * x2;
		const float x0 = x1 * x2;

		int i = 0, j = 0;
		for (; j<samples-4; j++)
		{

			float y0 = g_coeffs[0] * x0 +  g_coeffs[1] * x1 +  g_coeffs[2] * x2 +  g_coeffs[3] * x3;
			float y1 = g_coeffs[4] * x0 +  g_coeffs[5] * x1 +  g_coeffs[6] * x2 +  g_coeffs[7] * x3;
			float y2 = g_coeffs[8] * x0 +  g_coeffs[9] * x1 + g_coeffs[10] * x2 + g_coeffs[11] * x3;
			float y3 = g_coeffs[12] * x0 + g_coeffs[13] * x1 + g_coeffs[14] * x2 + g_coeffs[15] * x3;

			float temp = y0 * src[0] + y1 * src[1] + y2 * src[2] + y3 * src[3];
			dst[i++] = (sample_t)temp;

			fFract += rate;
			int whole = (int)fFract;
			fFract -= whole;
			src += whole;
			j += whole;
		}

		samples = j;
		return i;
#endif
	}

	int InterpolateImpl::ProcessStereo(sample_t* dst, const sample_t* src, int &samples)
	{
//interpolate linear
#ifdef INTERPOLATE_LINEAR
		int i = 0, j = 0;
		for (; j<samples-1;)
		{
			double temp0 = (1.0 - fFract) * src[0] + fFract * src[2];
			double temp1 = (1.0 - fFract) * src[1] + fFract * src[3];
			dst[0] = (sample_t)temp0;
			dst[1] = (sample_t)temp1;
			dst += 2;

			fFract += rate;
			int whole = (int)fFract;
			fFract -= whole;
			src += 2 * whole;
			j += whole;
		}

		samples = j;
		return i;
	}
#elif defined(INTERPOLATE_CUBIC)
		const float x3 = 1.0f;
		const float x2 = fFract;
		const float x1 = x2 * x2;
		const float x0 = x1 * x2;

		int i = 0, j = 0;
		for (; j < samples - 4; j++)
		{

			float y0 = g_coeffs[0] * x0 + g_coeffs[1] * x1 + g_coeffs[2] * x2 + g_coeffs[3] * x3;
			float y1 = g_coeffs[4] * x0 + g_coeffs[5] * x1 + g_coeffs[6] * x2 + g_coeffs[7] * x3;
			float y2 = g_coeffs[8] * x0 + g_coeffs[9] * x1 + g_coeffs[10] * x2 + g_coeffs[11] * x3;
			float y3 = g_coeffs[12] * x0 + g_coeffs[13] * x1 + g_coeffs[14] * x2 + g_coeffs[15] * x3;

			float temp0 = y0 * src[0] + y1 * src[2] + y2 * src[4] + y3 * src[6];
			float temp1 = y0 * src[1] + y1 * src[3] + y2 * src[5] + y3 * src[7];
			dst[2*i+0] = (sample_t)temp0;
			dst[2*i+1] = (sample_t)temp1;
			i++;

			fFract += rate;
			int whole = (int)fFract;
			fFract -= whole;
			src += 2 * whole;
			j += whole;
		}

		samples = j;
		return i;
#endif

#endif // end FLOAT_SAMPLES
	//////////////////////////////////////////////////////////////////////////
	//
	//implements of interpolate
	//
	//////////////////////////////////////////////////////////////////////////
	Interpolate::Interpolate(void)
	{
		impl = new InterpolateImpl();
		assert(impl);
	}


	Interpolate::~Interpolate(void)
	{
		if (impl) delete impl;
	}

	void Interpolate::SetRate(float rate)
	{
		assert(impl);
		impl->SetRate(rate);
	}

	void Interpolate::SetChannels(int channels)
	{
		assert(impl);
		impl->SetChannels(channels);
	}

	int Interpolate::ProcessSamples(SampleBuffer* dst, SampleBuffer* src)
	{
		assert(impl);
		return impl->ProcessSamples(dst, src);
	}

	int Interpolate::GetChannels(void) const
	{
		assert(impl);
		return impl->GetChannels();
	}

	float Interpolate::GetRate(void) const
	{
		assert(impl);
		return impl->GetRate();
	}
}