#ifndef __LIBIM_BEAUTIFY_H__
#define __LIBIM_BEAUTIFY_H__
#include "Defines.h"
#include "Object.h"

namespace e
{
	class BlurFilter;
	class Blender;
	class CurvesConfig;
	class Beautify : public RefCountObj
	{
	public:
		Beautify(void);
		virtual ~Beautify(void);
	public:
#ifdef INTEGER_CHANNELS
		void HighPass(uint8* dst, uint8* src, int width, int height, int channels);
		void CalcMatte(uint8* dst, uint8* src, int width, int height, int channels);
		void Smooth(uint8* dst, uint8* src, uint8* mte, int width, int height, int channels);
		void AdjustSample(uint8* dst, uint8* src, int width, int height, int channels);
#else
		void HighPass(float* dst, float* src, int width, int height, int channels);
		void CalcMatte(float* dst, float* src, int width, int height, int channels);
		void Smooth(float* dst, float* src, float* mte, int width, int height, int channels);
		void AdjustSample(float* dst, float* src, int width, int height, int channels);
#endif
	protected:
		void InitCurves(void);
	protected:
		BlurFilter* filter;
		Blender* blender;
		CurvesConfig* curves;
		uint8 samples[256];
	};
}

#endif