#pragma once
#include "Defines.h"

namespace e
{
	class Bilateral
	{
	public:
		Bilateral(void);
		~Bilateral(void);
		void SetRadius(int radius);
		void Process(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma, int mode);
		void KeyDown(int key);
		void CalcParam(void);
	private:
		void Init(int d);
		void FindMaxMin(uint8* src, int width, int height, int bitCount, uint8 &max, uint8 &min);
	private:
		int size[2];
		int radius;
		float s_sigma;
		float c_sigma;
		float* s_table;
		float** c_table;
		float scaleIndex;
	};
}


