#pragma once
#include "Defines.h"
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
		void Process(uint8* dst, const uint8* src, int width, int height, int bitCount);
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