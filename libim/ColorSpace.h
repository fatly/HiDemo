#pragma once
namespace e
{
	class ColorSpace
	{
	public:
		ColorSpace();
		virtual ~ColorSpace();
	public:
		static void RGB2HSV(void* dst, void* src, int width, int height, int channels);
		static void RGB2HSL(void* dst, void* src, int width, int height, int channels);
		static void HSV2RGB(void* dst, void* src, int width, int height, int channels);
		static void HSL2RGB(void* dst, void* src, int width, int height, int channels);
	};
}
