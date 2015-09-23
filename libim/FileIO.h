#ifndef __CORE_FILEIO_H__
#define __CORE_FILEIO_H__

namespace e
{
	class FileIO
	{
	public:
		// 从文件加载bmp文件
		static bool LoadBitmap(const char* fileName, void** bits, int& size, int&width, int&height, int&channels);
		//保存bmp到文件
		static bool SaveBitmap(const char* fileName, const void* bits, int size, int width, int height, int channels);
	};
}

#endif
