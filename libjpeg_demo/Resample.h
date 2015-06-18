#ifndef __CORE_RESAMPLE_H__
#define __CORE_RESAMPLE_H__
#include "Defines.h"

namespace e
{
	class Resample
	{
	public:
		Resample();
		~Resample();
		void BindHandle(int index);
		uint8* operator()(uint8*, uint8*, uint8*, int, int);
	private:
		uint8* ResampleRow1(uint8* out, uint8* in_near, uint8* in_far, int w, int hs);
		uint8* ResampleRowV2(uint8* out, uint8* in_near, uint8* in_far, int w, int hs);
		uint8* ResampleRowH2(uint8* out, uint8* in_near, uint8* in_far, int w, int hs);
		uint8* ResampleRowHV2(uint8* out, uint8* in_near, uint8* in_far, int w, int hs);
		uint8* ResampleRowGeneric(uint8* out, uint8* in_near, uint8* in_far, int w, int hs);
	public:
		uint8* line0;
		uint8* line1;
		int hs;
		int vs;
		int w_lores;
		int ystep;
		int ypos;
	private:
		typedef uint8* (Resample::*ResampleHandle)(uint8*, uint8*, uint8*, int, int);
		ResampleHandle fnResample;
	};
}

#endif