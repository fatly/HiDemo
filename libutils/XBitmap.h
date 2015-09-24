#ifndef __CORE_XBITMAP_H__
#define __CORE_XBITMAP_H__
#include "XImage.h"
#include "FileIO.h"

//////////////////////////////////////////////////////////////////////////
//--------------------defined of XBitmap<T>------------------------
//////////////////////////////////////////////////////////////////////////
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
			, const T* data = 0
			, bool init = false
			, bool alloc = true);
		XBitmap(const XBitmap& other);
		virtual ~XBitmap(void);
	public:
		bool Load(const char* fileName);
		bool Save(const char* fileName);
		virtual bool Resize(int width, int height, int channels) override;
	protected:
		virtual bool Create(int width
			, int height
			, int channels
			, const T* data = 0
			, bool init = false
			, bool alloc = true) override;
	};
}

//////////////////////////////////////////////////////////////////////////
//------------------implements of XBitmap<T>---------------------
//////////////////////////////////////////////////////////////////////////
namespace e
{
	template<class T>
	XBitmap<T>::XBitmap(void):Image<T>()
	{
	}

	template<class T>
	XBitmap<T>::XBitmap(const char* fileName)
	{
		data = 0;
		size = 0;
		width = 0;
		height = 0;
		channels = 0;
		pixels = 0;
		lineBytes = 0;

		if (!Load(fileName))
		{
			Clear();
			assert(0);
		}
		else
		{
			pixels = width * height * channels;
			lineBytes = WIDTHBYTES(width*channels * sizeof(T)*8);
		}
	}

	template<class T>
	XBitmap<T>::XBitmap(int width
		, int height
		, int channels
		, const T* data/* = 0 */
		, bool init /*= false*/
		, bool alloc /*= true*/)
	{
		this->data = 0;
		this->size = 0;

		if (Create(width, height, channels, data, init, alloc))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->pixels = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * sizeof(T)*8);
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
		this->data = 0;
		this->size = 0;

		if (Create(other.width, other.height, other.channels, other.data))
		{
			this->width = other.width;
			this->height = other.height;
			this->channels = other.channels;
			this->pixels = other.pixels;
			this->lineBytes = other.lineBytes;
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
			this->pixels = width * height * channels;
			this->lineBytes = WIDTHBYTES(width*channels * sizeof(T) * 8);
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
		, const T* data /* = 0 */
		, bool init /* = false */
		, bool alloc /* = true */)
	{
		assert(width > 0 && height > 0 && channels > 0);
		int bytes = WIDTHBYTES(width*channels * sizeof(T) * 8) * height;
		return XImage<T>::Create(bytes, data, init, alloc);
	}

	template<class T>
	bool XBitmap<T>::Load(const char* fileName)
	{
		if (sizeof(T) == sizeof(uint8))
		{
			if (FileIO::_LoadBitmap(fileName, (void**)&data, size, width, height, channels))
			{
				pixels = width * height * channels;
				lineBytes = WIDTHBYTES(width*channels * sizeof(T) * 8);
				return true;
			}

			Clear();
			return false;
		}
		else// float pixels
		{
			uint8* bits = 0;
			bool ret = false;
			if (FileIO::_LoadBitmap(fileName, (void**)&bits, size, width, height, channels))
			{
				if (Create(width, height, channels, 0, false, true))
				{
					pixels = width * height * channels;
					lineBytes = WIDTHBYTES(width*channels * sizeof(T) * 8);
					//convert uint8 to float
					XBitmap<uint8> tmp(width, height, channels, bits, false, false);
					for (int y = 0; y < height; y++)
					{
						T* d = this->Ptr(0, y);
						uint8* s = tmp.Ptr(0, y);
						for (int x = 0; x < width; x++)
						{
							for (int c = 0; c < channels; c++)
							{
								d[c] = T(s[c]);
							}

							s += channels;
							d += channels;
						}
					}
					ret = true;
				}
			}
			else
			{
				Clear();
				if (bits) free(bits);
				ret = false;
			}
		
			return ret;
		}
	}

	template<class T>
	bool XBitmap<T>::Save(const char* fileName)
	{
		assert(data && size);
		if (sizeof(T) == sizeof(uint8))
		{
			return FileIO::_SaveBitmap(fileName, data, size, width, height, channels);
		}
		else//convert float to uint8
		{
			XBitmap<uint8> tmp(width, height, channels);
			for (int y = 0; y < height; y++)
			{
				T* s = this->Ptr(0, y);
				uint8* d = tmp.Ptr(0, y);
				for (int x = 0; x < width; x++)
				{
					for (int c = 0; c < channels; c++)
					{
						d[c] = clamp0255((uint8)(s[c] + 0.5f));
					}

					s += channels;
					d += channels;
				}
			}
			return FileIO::_SaveBitmap(fileName, tmp.Ptr(0), tmp.Size(), width, height, channels);
		}
	}
}

#endif