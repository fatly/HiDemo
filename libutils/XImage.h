#ifndef __CORE_XImage_H__
#define __CORE_XImage_H__
#include <assert.h>
#include <math.h>
#include <memory.h>
#include "Defines.h"
#include "Object.h"

//////////////////////////////////////////////////////////////////////////
//defined of XImage<T>
//////////////////////////////////////////////////////////////////////////
namespace e
{
	template<class T>class XImage : public RefCountObj
	{
	public:
		XImage(void);
		XImage(int width
			, int height
			, int channels
			, const T* data = 0
			, bool init = false
			, bool alloc = true);
		XImage(const XImage<T>& other);
		virtual ~XImage(void);
	public:
		int Width(void) const;
		int Height(void) const;
		int Channels(void) const;
		int PixelSize(void) const;
		int Size(void) const { return size; }
		virtual void Set(int index, const T& value);
		virtual void Set(int x, int y, const T& value);
		virtual T Get(int index) const;
		virtual T Get(int x, int y) const;
		virtual T* Ptr(int index) const;
		virtual T* Ptr(int x, int y) const;
		virtual bool Resize(int width, int height, int channels);
		XImage<T>* Clone(void) const;
		XImage<T>* Clone(int channel) const;
		XImage<T>* Clone(int x0, int y0, int x1, int y1) const;
		void Scale(T factor);// pixel *= factor
		void Swap(const XImage<T>& other);
		void SwapChannel(int c1, int c2);
		void Clear(void);
	protected:
		bool IsValid(void) const;
		virtual bool Create(int size, const T* data, bool init, bool alloc);
		virtual bool Create(int width
			, int height
			, int channels
			, const T* data = 0
			, bool init = false
			, bool alloc = true);
	protected:
		T *data;
		int size;
		int width;
		int height;
		int channels;
		int pixels;
		int lineBytes;
	};
}

//////////////////////////////////////////////////////////////////////////
//implements of XImage<T>
//////////////////////////////////////////////////////////////////////////
namespace e
{
	template<class T>
	XImage<T>::XImage(void)
	{
		data = 0;
		size = 0;
		width = 0;
		height = 0;
		channels = 0;
		pixels = 0;
		lineBytes = 0;
	}

	template<class T>
	XImage<T>::XImage(int width
		, int height
		, int channels
		, const T* src/* = 0 */
		, bool init/* =false */
		, bool alloc/* =true */)
	{
		data = 0;
		size = 0;

		if (Create(width, height, channels, src, init, alloc))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->pixels = width * height * channels;
			this->lineBytes = width * channels * sizeof(T);
		}
		else
		{
			Clear();
			assert(0);
		}
	}

	template<class T>
	XImage<T>::XImage(const XImage<T>& other)
	{
		data = 0;
		size = 0;

		if (Create(other.width, other.height, other.channels, other.data))
		{
			width = other.width;
			height = other.height;
			channels = other.channels;
			pixels = other.pixels;
			lineBytes = other.lineBytes;
		}
		else
		{
			Clear();
			assert(0);
		}
	}

	template<class T>
	XImage<T>::~XImage(void)
	{
		Clear();
	}

	template<class T>
	int XImage<T>::Width(void) const
	{
		return width;
	}

	template<class T>
	int XImage<T>::Height(void) const
	{
		return height;
	}

	template<class T>
	int XImage<T>::Channels(void) const
	{
		return channels;
	}

	template<class T>
	int XImage<T>::PixelSize(void) const
	{
		return channels * sizeof(T); 
	}

	template<class T>
	void XImage<T>::Set(int index, const T &value)
	{
		assert(data);
		assert(index >= 0 && index < pixels);
		data[index] = value;
	}

	template<class T>
	void XImage<T>::Set(int x, int y, const T& value)
	{
		assert(data);
		assert(x >= 0 && x < width);
		assert(y >= 0 && y < height);
		data[(y*lineBytes + x*channels*sizeof(T)/sizeof(T))] = value;
	}

	template<class T>
	T XImage<T>::Get(int index) const
	{
		assert(data);
		assert(index >= 0 && index < pixels);
		return data[index];
	}

	template<class T>
	T XImage<T>::Get(int x, int y) const
	{
		assert(data);
		assert(x >= 0 && x < width);
		assert(y >= 0 && y < height);
		return data[(y*lineBytes + x*channels*sizeof(T))/sizeof(T)];
	}

	template<class T>
	T* XImage<T>::Ptr(int index) const
	{
		assert(data);
		assert(index >= 0 && index < pixels);
		return &data[index];
	}

	template<class T>
	T* XImage<T>::Ptr(int x, int y) const
	{
		assert(data);
		assert(x >= 0 && x < width);
		assert(y >= 0 && y < height);
		return &data[(y*lineBytes + x*channels*sizeof(T))/sizeof(T)];
	}

	template<class T>
	XImage<T>* XImage<T>::Clone(void) const
	{
		return new XImage<T>(*this);
	}

	template<class T>
	XImage<T>* XImage<T>::Clone(int channel) const
	{
		assert(IsValid());
		assert(channel >= 0 && channel < channels);
		XImage<T>* im = new XImage<T>(width, height, 1);
		if (im)
		{
			for (int y = 0; y < height; y++)
			{
				T* s = Ptr(0, y) + channel;
				T* d = im->Ptr(0, y);
				for (int x = 0; x < width; x++)
				{
					*d++ = *s;
					s += channels;
				}
			}
		}

		return im;
	}

	template<class T>
	XImage<T>* XImage<T>::Clone(int x0, int y0, int x1, int y1) const
	{
		assert(IsValid());
		assert(x0 >= 0 && x0 < width);
		assert(x1 >= 0 && x1 < width);
		assert(y0 >= 0 && y0 < height);
		assert(y1 >= 0 && y1 < height);

		limit(x0, 0, width - 1);
		limit(x1, 0, width - 1);
		limit(y0, 0, height - 1);
		limit(y1, 0, height - 1);
		if (x0 > x1) swap(x0, x1);
		if (y0 > y1) swap(y0, y1);

		int w = x1 - x0 + 1;
		int h = y1 - y0 + 1;

		XImage<T>* im = new XImage<T>(w, h, channels);
		if (im)
		{
			for (int y = y0; y <= y1; y++)
			{
				T* s = this->Ptr(x0, y);
				T* d = im->Ptr(0, y - y0);
				memcpy(d, s, w * channels * sizeof(T));
			}
		}

		return im;
	}

	template<class T>
	void XImage<T>::Scale(T factor)
	{
		assert(IsValid());
		for (int y = 0; y < height; y++)
		{
			T* p = Ptr(0, y);
			for (int x = 0; x < width; x++)
			{
				for (int c = 0; c < channels; c++)
				{
					p[c] *= factor;
				}

				p += channels;
			}
		}
	}

	template<class T>
	void XImage<T>::Swap(const XImage<T>& other)
	{
		swap(data, other.data);
		swap(size, other.size);
		swap(width, other.width);
		swap(height, other.height);
		swap(channels, other.channels);
		swap(pixels, other.pixels);
		swap(lineBytes, other.lineBytes);
	}

	template<class T>
	void XImage<T>::SwapChannel(int c1, int c2)
	{
		assert(c1 >= 0 && c1 < channels);
		assert(c2 >= 0 && c2 < channels);

		for (int y = 0; y < height; y++)
		{
			T	*p = Get(0, y);
			for (int x = 0; x < width; x++)
			{
				swap(*(p + c1), *(p + c2));
				p += channels;
			}
		}
	}

	template<class T>
	bool XImage<T>::Resize(int width, int height, int channels)
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
			this->lineBytes = width * channels * sizeof(T);
			return true;
		}
		else
		{
			Clear();
			return false;
		}
	}

	template<class T>
	void XImage<T>::Clear(void)
	{
		if (data) free(data);
		data = 0;
		size = 0;
		width = 0;
		height = 0;
		channels = 0;
		pixels = 0;
		lineBytes = 0;
	}

	template<class T>
	bool XImage<T>::IsValid(void) const
	{
		return data != 0;
	}

	template<class T>
	bool XImage<T>::Create(int size, const T* data, bool init, bool alloc)
	{
		assert(size > 0);
		if (alloc)
		{
			this->data = (T*)realloc(this->data, size);
			if (!this->data) return false;
		}
		else
		{
			this->data = (T*)data;
		}

		if (alloc)
		{
			if (data)
			{
				memcpy(this->data, data, size);
			}
			else if (init)
			{
				memset(this->data, 0, size);
			}
		}
// 		else if (init)
// 		{
// 			memset(data, 0, size);
// 		}

		this->size = size;
		return true;
	}

	template<class T>
	bool XImage<T>::Create(int width
		, int height
		, int channels
		, const T* data/* =0 */
		, bool init/* =false */
		, bool alloc/* =true */)
	{
		assert(width >= 0 && height >= 0);
		assert(channels >= 0 && channels <= 4);

		int size = width * height * channels * sizeof(T);
		return Create(size, data, init, alloc);
	}
}

#endif