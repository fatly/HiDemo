#pragma once
#include "Define.h"

namespace e
{
	class SampleBuffer;
	class FIRFilter
	{
	public:
		FIRFilter();
		virtual ~FIRFilter();
	public:
		uint GetLength(void) const;
		static FIRFilter* GetInstance(void);
		virtual uint Process(sample_t* dst, const sample_t* src, uint samples, uint channels);
		virtual void SetCoeffs(sample_t* coeffs, uint length, uint resultDivFactor);
	protected:
		virtual uint ProcessMono(sample_t* dst, const sample_t* src, uint samples);
		virtual uint ProcessStereo(sample_t* dst, const sample_t* src, uint samples);
	protected:
		uint length;
		uint lengthDiv8;
		uint resultDivFactor;
		sample_t resultDivider;
		sample_t* filterCoeffs;
	};

	class AAFilter
	{
	public:
		AAFilter(uint length);
		virtual ~AAFilter(void);
	public:
		void SetLength(uint length);
		uint GetLength(void) const;
		void SetCutoffFrequency(double frequency);
		uint Process(sample_t* dst, const sample_t* src, uint samples, uint channels) const;
		uint Process(SampleBuffer* dst, SampleBuffer* src) const;
	protected:
		void CalcCoeffs(void);
	protected:
		uint length; 
		double cut_off_frequency;
		FIRFilter* filter;
	};

}

