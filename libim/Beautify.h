#ifndef __LIBIM_BEAUTIFY_H__
#define __LIBIM_BEAUTIFY_H__
#include "Defines.h"
#include "Object.h"
#include "AutoPtr.h"

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
		void Process(void* dst, void* src, int width, int height, int channels);
	public:
		void Blur(void* dst, void* src, int width, int height, int channels);
		void HighPass(void* dst, void* src, int width, int height, int channels);
		void CalcMatte(void* dst, void* src, int width, int height, int channels);
		void Smooth(void* dst, void* src, void* mte, int width, int height, int channels);
		void AdjustSample(void* dst, void* src, int width, int height, int channels);
	protected:
		void InitCurves(void);
	protected:
		BlurFilter* filter;
		Blender* blender;
		CurvesConfig* curves;
		uint8 samples[256];
	};

	typedef AutoPtr<Beautify> BeautifyPtr;
}

#endif