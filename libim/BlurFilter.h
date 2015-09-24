#ifndef __LIBIM_BLURFILTER_H__
#define __LIBIM_BLURFILTER_H__
#include "Defines.h"
#include "BaseFilter.h"

namespace e
{
	enum{
		BT_GAUSSIAN = 0, 
		BT_BILATERAL = 1
	};

	class BaseFilter;
	class BlurFilter
	{
	public:
		BlurFilter(void);
		BlurFilter(float sigma);
		virtual ~BlurFilter(void);
	public:
		void SetSetting(int id, void* value);
		void Process(void* dst, void* src, int width, int height, int channels);
	protected:
		void SetType(int type);
	protected:
		int type;
		BaseFilter* filter;
	};
}

#endif