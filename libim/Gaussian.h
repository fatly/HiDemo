#ifndef __CORE_GAUSSIAN_H__
#define __CORE_GAUSSIAN_H__

#include "Defines.h"
#include "Filter.h"

namespace e
{
	typedef enum { 
		GM_SIMPLE = 0, GM_HIGH = 1 
	}GaussianMode;

	class Gaussian : public FilterBase
	{
	public:
		Gaussian(void);
		virtual ~Gaussian(void);
	public:
		void SetMode(int mode);
		void SetSigma(float sigma);
		virtual void Process(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);
	protected:
		void CalcParameters(float sigma);
		void SimpleGaussian8(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);
		void SimpleGaussian24(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);

		void RecursiveGaussian8(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);

		void RecursiveGaussian24(uint8* dst
			, const uint8* src
			, const int width
			, const int height
			, const int bitCount);
	private:
		int mode;
		float sigma;
		float a0, a1, a2, a3;
		float b1, b2;
		float coefp, coefn;
	};
}

#endif