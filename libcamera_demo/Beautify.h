#ifndef __CORE_BEAUTIFY_H__
#define __CORE_BEAUTIFY_H__
#include "Defines.h"

namespace e
{
	class Beautify 
	{
	public:
		Beautify(int width, int height);
		virtual ~Beautify(void);
	public:
		void Process(uint8* buffer, int width, int height, int bitCount);

		void KeyDown(int key);

		float GetSigma(void) const
		{
			return sigma;
		}

	private:
		float sigma;
		int width;
		int height;

		uint8* src;
		uint8* tmp;

		bool enable;
	};
}


#endif