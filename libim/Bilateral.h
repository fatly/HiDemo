#ifndef __LIBIM_BILATERLA_H__
#define __LIBIM_BILATERLA_H__
#include "Defines.h"
#include "BaseFilter.h"

namespace e
{
	enum{
		BM_SIMPLE = 0,
		BM_HIGH = 1,
	};

	class Bilateral : public BaseFilter
	{
	public:
		Bilateral(void);
		virtual ~Bilateral(void);
	public:
		virtual void SetSetting(int id, void* value) override;
		virtual void Process(void* dst
			, void* src
			, int width
			, int height
			, int channels) override;
	protected:
		void Init(int d);
		void CalcKernals(void);
		void SetSigma(float spaceSigma, float colorSigma);
		void SetRadius(int radius);
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