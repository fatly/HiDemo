#ifndef __LIBIM_BEAUTIFY_H__
#define __LIBIM_BEAUTIFY_H__
#include "Defines.h"
#include "Object.h"

namespace e
{
	class BlurFilter;
	class Blender;
	class FeatherFilter;
	class CurvesConfig;
	class Beautify : public RefCountObj
	{
	public:
		Beautify(void);
		virtual ~Beautify(void);
	public:
		void Beautify::HighPass(uint8* dst
			, const uint8* src
			, int width
			, int height
			, int channels);

		void CalcMatte(uint8* dst
			, const uint8* src
			, int width
			, int height
			, int channels);

		void Smooth(uint8* dst
			, const uint8* src
			, const uint8* mte
			, int width
			, int height
			, int channels);

		void AdjustSample(uint8* dst
			, const uint8* src
			, int width
			, int height
			, int channels);

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