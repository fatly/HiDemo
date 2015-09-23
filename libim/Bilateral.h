#ifndef __LIBIM_BILATERLA_H__
#define __LIBIM_BILATERLA_H__
#include "Defines.h"
#include "BaseFilter.h"

namespace e
{
	class Bilateral : public BaseFilter
	{
	public:
		Bilateral(void);
		virtual ~Bilateral(void);
#ifdef INTEGER_CHANNELS
		virtual void Process(uint8* dst
			, uint8* src
			, int width
			, int height
			, int channels) override;
#else
		virtual void Process(float* dst
			, float* src
			, int width
			, int height
			, int channels) override;
#endif
	protected:
		void Init(int d);
		void Clear(void);
	protected:
		int radius;
		int spaceSize;
		int colorSize;
		float spaceSigma;
		float colorSigma;
		float** spaceKernal;
		float* colorKernal;
		float scaleFactor;

	};
}

#endif