#ifndef __CORE_ARRAY_H__
#define __CORE_ARRAY_H__

#include <new>
#include "Diagnosis.h"

namespace e
{
	template<typename T> class Array
	{
	public:
		Array(void)
		{
			buffer = 0;
			length = 0;
			bufferSize = 0;
		}

		Array(size_t size, const T & v = T())
		{
			buffer = 0;
			length = 0;
			bufferSize = 0;
			resize(size, v);
		}

		Array(const Array & r)
		{
			length = r.length;
			bufferSize = length;
			buffer = (T*)malloc(bufferSize * sizeof(T));
			ASSERT(buffer != 0);

			for (size_t i = 0; i < length; i++)
			{
				new(buffer + i) T(r.buffer[i]);
			}
		}

		virtual ~Array(void)
		{
			clear();
		}

		const Array & operator=(const Array & r)
		{
			if (this != &r)
			{
				clear();
				length = r.length;
				bufferSize = length;
				buffer = (T*)malloc(bufferSize * sizeof(T));
				ASSERT(buffer != 0);
				
				for (size_t i = 0; i < length; i++)
				{
					new (buffer + i) T(r.buffer[i]);
				}
			}

			return *this;
		}

		void resize(size_t size, const T & v = T())
		{
			if (length < size)
			{
				if (size >= bufferSize)
				{
					bufferSize = size + 1 + size >> 1;
					buffer = (T*)realloc(buffer, bufferSize * sizeof(T));
					ASSERT(buffer != 0);
				}

				for (size_t i = length; i < bufferSize; i++)
				{
					new (buffer + i) T(v);
				}
			}
			else
			{
				for (size_t i = 0; i < length; i++)
				{
					(buffer + i)->~T();
				}
			}

			length = size;
		}

		void remove(const size_t index)
		{
			ASSERT(index >= 0 && index < length);
			for (size_t i = index; i < length - 1; i++)
			{
				buffer[i] = buffer[i + 1];
			}
			length--;
			buffer[length].~T();
		}

		void insert(size_t index, const T & v)
		{
			if (index >= 0 && index <= length)
			{
				resize(length + 1, T());
				for (size_t i = length - 1; i > index; i--)
				{
					buffer[i] = buffer[i - 1];
				}

				buffer[index] = v;
			}
			else
			{
				push_back(v);
			}
		}

		void push_back(const T & v)
		{
			length++;
			if (length >= bufferSize)
			{
				bufferSize = length + 1 + (length >> 1);
				buffer = (T*)realloc(buffer, bufferSize * sizeof(T));
				ASSERT(buffer != 0);
			}
			new(buffer + length - 1) T(v);
		}

		void pop_back(void)
		{
			ASSERT(length > 0);
			length--;
			(buffer + length)->~T();
		}

		size_t size(void) const
		{
			return length;
		}

		bool empty(void)
		{
			return length == 0;
		}

		void clear(void)
		{
			if (buffer != 0)
			{ 
				for (size_t i = 0; i < length; i++)
				{
					(buffer + i)->~T();
				}
				free(buffer);
			}
			buffer = 0;
			length = 0;
			bufferSize = 0;
		}

		void swap(Array & r)
		{
			T * p = buffer;
			buffer = r.buffer;
			r.buffer = p;

			size_t t = length;
			length = r.length;
			r.length = t;

			t = bufferSize;
			bufferSize = r.bufferSize;
			r.bufferSize = t;
		}

		T & operator[](size_t index) 
		{ 
			ASSERT(length > index); 
			return buffer[index];
		}

		const T & operator[](size_t index) const
		{
			ASSERT(length > index);
			return buffer[index];
		}

		T & front(void)
		{
			ASSERT(length > 0);
			return buffer[0];
		}

		const T & front(void) const
		{
			ASSERT(length > 0);
			return buffer[0];
		}
		
		T & back(void)
		{
			ASSERT(length > 0);
			return buffer[length - 1];
		}

		const T & back(void) const
		{
			ASSERT(length > 0);
			return buffer[length - 1];
		}

	private:
		T * buffer;
		size_t length;
		size_t bufferSize;
	};
}

#endif
