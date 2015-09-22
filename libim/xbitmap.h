#ifndef __CORE_XBITMAP_H__
#define __CORE_XBITMAP_H__
#include "XImage.h"
#include "FileIO.h"

namespace e
{
	template<class T>
	class XBitmap : public XImage<T>
	{
	public:
		XBitmap(void);
		XBitmap(const char* fileName);
		XBitmap(int width
			, int height
			, int channels
			, T* src = 0
			, bool init = false
			, bool alloc = true);
		XBitmap(const XBitmap& other);
		virtual ~XBitmap(void);
	public:
		virtual bool Resize(int width, int height, int channels) override;
		int BitCount(void) const
		{	return channels * sizeof(T);}
		bool Save(const char* fileName);
	protected:
		virtual bool Create(int width
			, int height
			, int channels
			, T* src = 0
			, bool init = false
			, bool alloc = true);
		bool Load(const char* fileName);
	};

	template<class T>
	XBitmap<T>::XBitmap(void):Image<T>()
	{
	}

	template<class T>
	XBitmap::XBitmap(const char* fileName)
	{
		data = 0;
		size = 0;
		width = 0;
		height = 0;
		channels = 0;
		samples = 0;
		lineBytes = 0;

		if (!Load(fileName))
		{
			Clear();
			assert(0);
		}
		else
		{
			samples = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * sizeof(T));
		}
	}

	template<class T>
	XBitmap<T>::XBitmap(int width
		, int height
		, int channels
		, T* src/* = 0 */
		, bool init /*= false*/
		, bool alloc /*= true*/)
	{
		data = 0;
		size = 0;

		if (Create(width, height, channels, src, init, alloc))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->samples = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * sizeof(T));
		}
		else
		{
			Clear();
			assert(0);
		}
	}

	template<class T>
	XBitmap<T>::XBitmap(const XBitmap& other)
	{
		data = 0;
		size = 0;

		if (Create(other.width, other.height, other.channels, other.data))
		{
			width = other.width;
			height = other.height;
			channels = other.channels;
			samples = other.samples;
			lineBytes = other.lineBytes;
		}
		else
		{
			Clear();
			assert(0);
		}
	}

	template<class T>
	XBitmap<T>::~XBitmap(void)
	{

	}

	template<class T>
	bool XBitmap<T>::Resize(int width, int height, int channels)
	{
		assert(width > 0 && height > 0 && channels > 0);
		if (this->width == width && this->height == height && this->channels == channels)
		{
			return true;
		}

		if (Create(width, height, channels))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->samples = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * sizeof(T));
			return true;
		}
		else
		{
			Clear();
			return false;
		}
	}

	template<class T>
	bool XBitmap<T>::Create(int width
		, int height
		, int channels
		, T* src /* = 0 */
		, bool init /* = false */
		, bool alloc /* = true */)
	{
		assert(width > 0 && height > 0 && channels > 0);
		int lineSize = WIDTHBYTES(width*channels * sizeof(T));
		return Create(lineSize * height, src, init, alloc);
	}

	template<class T>
	bool XBitmap<T>::Load(const char* fileName)
	{
		if (sizeof(T) == sizeof(char))
		{
			if (FileIO::LoadBitmap(fileName, data, size, width, height, channels))
			{
				samples = width * height * channels;
				lineBytes = WIDTHBYTES(width*channels * sizeof(T));
				return true;
			}

			Clear();
			return false;
		}
		else
		{
			char* bits = 0;
			bool ret = false;
			if (FileIO::LoadBitmap(fileName, bits, size, width, height, channels))
			{
				samples = width * height * channels;
				lineBytes = WIDTHBYTES(width*channels * sizeof(T));
				if (Alloc(width, height, channels))
				{
					XBitmap<uint8> tmp(width, height, channels, bits);
					C2F(*this, tmp);
					ret = true;
				}
			}
			else
			{
				Clear();
				ret = false;
			}
			
			if (bits) free(bits);
			return ret;
		}
	}

	template<class T>
	bool XBitmap<T>::Save(const char* fileName)
	{
		return true;
	}
}

#endif