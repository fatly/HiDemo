#ifndef __CORE_XBITMAP_H__
#define __CORE_XBITMAP_H__
#include "Image.h"

namespace e
{
	template<class T>
	class XBitmap : public Image<T>
	{
	public:
		XBitmap(void);
		XBitmap(const char* fileName);
		XBitmap(int width, int height, int channels, bool init = false);
		XBitmap(const XBitmap& other);
		virtual ~XBitmap(void);
	public:
		virtual bool Resize(int width, int height, int channels) override;
		bool Save(const char* fileName, bool reserve=true);
	protected:
		bool Load(const char* fileName, bool reserve=true);
		virtual bool Alloc(int width, int height, int channels, const T* src = 0, bool init = false);
	};

	template<class T>
	XBitmap<T>::XBitmap(void) :Image<T>()
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
			lineBytes = WIDTHBYTES(width*channels * 8);
		}
	}

	template<class T>
	XBitmap<T>::XBitmap(int width, int height, int channels, bool init = false)
	{
		data = 0;
		size = 0;

		if (Alloc(width, height, channels, init))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->samples = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * 8);
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

		if (Alloc(other.width, other.height, other.channels, other.data))
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
	bool XBitmap<T>::Resize(int width, int height, int channels)
	{
		assert(width > 0 && height > 0 && channels > 0);
		if (this->width == width && this->height == height && this->channels == channels)
		{
			return true;
		}

		if (Alloc(width, height, channels))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->samples = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * 8);
			return true;
		}
		else
		{
			Clear();
			return false;
		}
	}

	template<class T>
	bool XBitmap<T>::Alloc(int width, int height, int channels, const T* src /* = 0 */, bool init /* = false */)
	{
		assert(width > 0 && height > 0 && channels > 0);
		int lineSize = WIDTHBYTES(width*channels * 8);
		return Alloc(lineSize * height, src, init);
	}

	template<class T>
	bool XBitmap<T>::Load(const char* fileName, bool reserve/*=true*/)
	{

	}

	template<class T>
	bool XBitmap<T>::Save(const char* fileName, bool reserve/* =true */)
	{

	}
}

#endif