#ifndef __LIBIM_BLENDER_H__
#define __LIBIM_BLENDER_H__

#include "Defines.h"
#define MakeIndex(a, b) ((((a)<<8)|(b)) & 0xffff)

namespace e
{
	typedef enum{
		BM_NORMAL,
		BM_SOFTLIGHT,
		BM_HARDLIGHT,
		BM_SCREEN, 
		BM_HIGHPASS
	}BlendMode;

	class Blender 
	{
	public:
		Blender(void);
		virtual ~Blender(void);
	public:
		void SetMode(int mode);
		void GetValues(uint8* results) const;
		void GetValues(uint8* results, int mode);
#ifdef INTEGER_CHANNELS
		void Process(uint8* dst, uint8* src, int width, int height, int channels);
#else
		void Process(float* dst, float* src, int width, int height, int channels);
#endif
	protected:
		void Update(void);
		void Normal(void);
		void SoftLight(void);
		void HardLight(void);
		void Screen(void);
		void HighPass(void);
	private:
		int mode;
		uint8* values;
	};
}

#endif