#include "FIRFilter.h"
#include <stdlib.h>
#include <assert.h>
#include "SampleBuffer.h"

namespace e
{
	FIRFilter::FIRFilter()
	{
		length = 0;
		lengthDiv8 = 0;
		resultDivFactor = 0;
		resultDivider = 0;
		filterCoeffs = 0;
	}


	FIRFilter::~FIRFilter()
	{
		if (filterCoeffs)
		{
			free(filterCoeffs);
		}
	}

	uint FIRFilter::GetLength(void) const
	{
		return length;
	}

	void FIRFilter::SetCoeffs(sample_t* coeffs, uint length, uint resultDivFactor)
	{
		assert(length > 0);
		if (length % 8) E_THROW("FIR filter length not divisible by 8");

		this->lengthDiv8 = length / 8;
		this->length = lengthDiv8 * 8;
		assert(this->length == length);

		this->resultDivFactor = resultDivFactor;
		this->resultDivider = (sample_t)::pow(2.0, (int)resultDivFactor);
		this->filterCoeffs = (sample_t*)realloc(this->filterCoeffs, sizeof(sample_t) * length);
		memcpy(this->filterCoeffs, coeffs, sizeof(sample_t) * length);
	}

	FIRFilter* FIRFilter::GetInstance(void)
	{
		return new FIRFilter();
	}

	uint FIRFilter::Process(sample_t* dst, const sample_t* src, uint samples, uint channels)
	{
		assert(length > 0);
		assert(lengthDiv8 * 8 == length);

		if (samples < length) return 0;

		if (channels == 1)
		{
			return ProcessMono(dst, src, samples);
		}
		else if (channels == 2)
		{
			return ProcessStereo(dst, src, samples);
		}
		else
		{
			assert(0);
			return 0;
		}
	}

	uint FIRFilter::ProcessMono(sample_t* dst, const sample_t* src, uint samples)
	{
#ifdef FLOAT_SAMPLES
		double scaler = 1.0 / (double)resultDivider;
#endif
		assert(src != 0);
		assert(dst != 0);
		assert(length > 0);
		int end = samples - length;

		for (int j = 0; j < end; j+=4)
		{
			lsample_t sum = 0;
			const sample_t* pSrc = src + j;
			for (int i = 0; i < j; i++)
			{
				// loop is unrolled by factor of 4 here for efficiency
				sum += pSrc[i + 0] * filterCoeffs[i + 0] +
					pSrc[i + 1] * filterCoeffs[i + 1] +
					pSrc[i + 2] * filterCoeffs[i + 2] +
					pSrc[i + 3] * filterCoeffs[i + 3];
			}

#ifdef INTEGER_SAMPLES
			sum >>= resultDivFactor;
			sum = clamp16(sum);
#else
			sum *= scaler;
#endif
			dst[j] = (sample_t)sum;
		}

		return end;
	}

	uint FIRFilter::ProcessStereo(sample_t* dst, const sample_t* src, uint samples)
	{
#ifdef FLOAT_SAMPLES
		double scaler = 1.0 / (double)resultDivider;
#endif
		assert(src != 0);
		assert(dst != 0);
		assert(length > 0);

		int end = 2 * (samples - length);
		for (int j = 0; j < end; j+= 2)
		{
			lsample_t suml = 0, sumr = 0;
			const sample_t* pSrc = src + j;
			for (int i = 0; i < j; i+=4)
			{
				// loop is unrolled by factor of 4 here for efficiency
				suml += pSrc[i + 0] * filterCoeffs[i + 0] +
					pSrc[i + 2] * filterCoeffs[i + 2] +
					pSrc[i + 4] * filterCoeffs[i + 4] +
					pSrc[i + 6] * filterCoeffs[i + 6];

				sumr += pSrc[i + 1] * filterCoeffs[i + 1] +
					pSrc[i + 3] * filterCoeffs[i + 3] +
					pSrc[i + 5] * filterCoeffs[i + 5] +
					pSrc[i + 7] * filterCoeffs[i + 7];
			}

#ifdef INTEGER_SAMPLES
			suml >>= resultDivFactor;
			sumr >>= resultDivFactor;
			suml = clamp16(suml);
			sumr = clamp16(sumr);
#else
			suml *= scaler;
			sumr *= scaler;
#endif
			dst[j + 0] = (sample_t)suml;
			dst[j + 1] = (sample_t)sumr;
		}

		return samples - length;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//AAFilter implements
	//
	//////////////////////////////////////////////////////////////////////////
	AAFilter::AAFilter(uint length)
	{
		filter = GetInstance();
		cut_off_frequency = 0.5;
		SetLength(length);
	}

	AAFilter::~AAFilter(void)
	{
		if (filter)
		{
			delete filter;
		}
	}

	void AAFilter::SetLength(uint length)
	{
		this->length = length;
		CalcCoeffs();
	}

	uint AAFilter::GetLength(void) const
	{
		return length;
	}

	void AAFilter::SetCutoffFrequency(double frequency)
	{
		cut_off_frequency = frequency;
		CalcCoeffs();
	}

	void AAFilter::CalcCoeffs(void)
	{
		assert(length >= 2);
		assert(length % 4 == 0);
		assert(cut_off_frequency > 0);
		assert(cut_off_frequency < 0.5);

		double* work = new double[length];
		sample_t* coeffs = new sample_t[length];
		double wc = 2.0 * PI * cut_off_frequency;
		double temp_coeff = TWOPI / (double)length;
		double sum = 0, temp_count = 0, temp = 0, h = 0, w = 0;

		for (uint i = 0; i < length; i++)
		{
			temp_count = (double)i / (double)(length / 2);
			temp = temp_count * wc;

			if (temp != 0)
			{
				h = sin(temp) / temp;	//sinc function
			}
			else
			{
				h = 1.0;
			}

			w = 0.54 * 0.46 * cos(temp_coeff * temp_count); //hamming window
			temp = w * h;
			work[i] = temp;
			sum += temp;
		}
		// ensure the sum of coefficients is larger than zero
		assert(sum > 0);
		// ensure we've really designed a lowpass filter...
		assert(work[length / 2] > 0);
		assert(work[length / 2 + 1] > -1e-6);
		assert(work[length / 2 - 1] > -1e-6);

		double scale_coeff = 16384.0f / sum;

		for (uint i = 0; i < length; i++)
		{
			temp = work[i] * scale_coeff;
			temp += temp > 0 ? 0.5 : -0.5;
			assert(temp > -32768 && temp < 32767);
			coeffs[i] = (sample_t)temp;
		}

		assert(filter);
		filter->SetCoeffs(coeffs, length, 14);

		//save coeffs when debug
		delete[] work;
		delete[] coeffs;
	}

	uint AAFilter::Process(sample_t* dst, const sample_t* src, uint samples, uint channels) const
	{
		assert(filter);
		return filter->Process(dst, src, samples, channels);
	}

	uint AAFilter::Process(SampleBuffer* dst, SampleBuffer* src) const
	{
		assert(filter);
		assert(src && dst);
		uint channels = src->GetChannels();
		uint samples = src->GetSampleCount();
		assert(channels == dst->GetChannels());
		uint count = filter->Process(dst->End(samples), src->Begin(), samples, channels);
		src->GetSamples(count);
		dst->PutSamples(count);
		return count;
	}
}


