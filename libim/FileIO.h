#ifndef __CORE_FILEIO_H__
#define __CORE_FILEIO_H__

namespace e
{
	class FileIO
	{
	public:
		// ���ļ�����bmp�ļ�
		static bool LoadBitmap(const char* fileName, char* bits, int& size, int&width, int&height, int&channels);
		//����bmp���ļ�
		static bool SaveBitmap(const char* fileName, const char* bits, int size, int width, int height, int channels);
	};
}

#endif