#ifndef __CORE_IMAGE_H__
#define __CORE_IMAGE_H__
#include <assert.h>

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	//defined of image<T>
	//////////////////////////////////////////////////////////////////////////
	template<class T>class Image
	{
	public:
		Image(void);
		Image(int width, int height, int channels, bool init=false);
		Image(const Image<T>& other);
		virtual ~Image(void);
	public:
		int Width(void) const;
		int Height(void) const;
		int Channels(void) const;
		virtual void Set(int index, const T& value);
		virtual void Set(int x, int y, const T& value);
		virtual T Get(int index) const;
		virtual T Get(int x, int y) const;
		virtual T* Ptr(int index) const;
		virtual T* Ptr(int x, int y) const;
		virtual bool Resize(int width, int height, int channels);
		Image<T>* Clone(void) const;
		Image<T>* Clone(int channel) const;
		Image<T>* Clone(int x0, int y0, int x1, int y1) const;
		void Swap(const Image<T>& other);
		void Clear(void);
	protected:
		bool IsValid(void) const;
		bool Alloc(int size, const T* src, bool init = false);
		virtual bool Alloc(int width, int height, int channels, const T* src=0, bool init=false);
	protected:
		T *data;
		int size;
		int width;
		int height;
		int channels;
		int samples;
		int lineBytes;
	};
}


//////////////////////////////////////////////////////////////////////////
//implements of image<T>
//////////////////////////////////////////////////////////////////////////

namespace e
{
	template<class T>
	Image<T>::Image(void)
	{
		data = 0;
		size = 0;
		width = 0;
		height = 0;
		channels = 0;
		samples = 0;
		lineBytes = 0;
	}

	template<class T>
	Image<T>::Image(int width, int height, int channels, bool init/* =false */)
	{
		data = 0;
		size = 0;
		if (Alloc(width, height, channels))
		{
			this->width = width;
			this->height = height;
			this->channels = channels;
			this->samples = width * height * channels;
			this->lineBytes = width * channels * sizeof(T);
		}
		else
		{
			Clear();
			assert(0);
		}
	}

	template<class T>
	Image<T>::Image(const Image<T>& other)
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
	Image<T>::~Image(void)
	{
		if (data) free(data);
	}

	template<class T>
	int Image<T>::Width(void) const
	{
		return width;
	}

	template<class T>
	int Image<T>::Height(void) const
	{
		return height;
	}

	template<class T>
	int Image<T>::Channels(void) const
	{
		return channels;
	}

	template<class T>
	void Image<T>::Set(int index, const T &value)
	{
		assert(data);
		assert(index >= 0 && index < samples);
		data[index] = value;
	}

	template<class T>
	void Image<T>::Set(int x, int y, const T& value)
	{
		assert(data);
		assert(x >= 0 && x < width);
		assert(y >= 0 && y < height);
		data[y*lineBytes + x*channels] = value;
	}

	template<class T>
	T Image<T>::Get(int index) const
	{
		assert(data);
		assert(index >= 0 && index < samples);
		return data[index];
	}

	template<class T>
	T Image<T>::Get(int x, int y) const
	{
		assert(data);
		assert(x >= 0 && x < width);
		assert(y >= 0 && y < height);
		return data[y*lineBytes + x*channels];
	}

	template<class T>
	T* Image<T>::Ptr(int index) const
	{
		assert(data);
		assert(index >= 0 && index < samples);
		return &data[index];
	}

	template<class T>
	T* Image<T>::Ptr(int x, int y) const
	{
		assert(data);
		assert(x >= 0 && x < width);
		assert(y >= 0 && y < height);
		return &data[y*lineBytes + x*channels];
	}

	template<class T>
	Image<T>* Image<T>::Clone(void) const
	{
		return new Image<T>(*this);
	}

	template<class T>
	Image<T>* Image<T>::Clone(int channel) const
	{
		assert(IsValid());
		assert(channel >= 0 && channel < channels);
		Image<T>* im = new Image<T>(width, height, 1);
		if (im)
		{
			for (int y = 0; y < height; y++)
			{
				T* s = Ptr(0, y) + channel;
				T* d = im->Get(0, y);
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
	Image<T>* Image<T>::Clone(int x0, int y0, int x1, int y1) const
	{
		assert(IsValid());
		assert(x0 >= 0 && x0 < width);
		assert(x1 >= 0 && x1 < width);
		assert(y0 >= 0 && y0 < height);
		assert(y1 >= 0 && y1 < height);

		if (x0 > x1) swap(x0, x1);
		if (y0 > y1) swap(y0, y1);

		int w = x1 - x0 + 1;
		int h = y1 - y0 + 1;
		if (w > width) w = width;
		if (h > height) h = height;

		Image<T>* im = new Image<T>(w, h, channels);
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
	void Image<T>::Swap(const Image<T>& other)
	{
		swap(data, other.data);
		swap(size, other.size);
		swap(width, other.width);
		swap(height, other.height);
		swap(channels, other.channels);
		swap(samples, other.samples);
		swap(lineBytes, other.lineBytes);
	}

	template<class T>
	bool Image<T>::Resize(int width, int height, int channels)
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
	void Image<T>::Clear(void)
	{
		width = 0;
		height = 0;
		channels = 0;
		samples = 0;
		lineBytes = 0;
	}

	template<class T>
	bool Image<T>::IsValid(void) const
	{
		return data != 0;
	}

	template<class T>
	bool Image<T>::Alloc(int size, const T* src, bool init /* = false */)
	{
		assert(size > 0);
		data = (T*)realloc(data, size);
		if (!data) return false;

		if (src)
			memcpy(data, src, size);
		else if (init)
			memset(data, 0, size);

		this->size = size;
		return true;
	}

	template<class T>
	bool Image<T>::Alloc(int width, int height, int channels, const T* src/* =0 */, bool init/* =false */)
	{
		assert(width >= 0 && height >= 0);
		assert(channels >= 0 && channels <= 4);

		int size = width * height * channels * sizeof(T);
		return Alloc(size, src, init);
	}
}

#endif