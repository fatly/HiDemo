#ifndef __CORE_ALLOCATOR_H__
#define __CORE_ALLOCATOR_H__

#include "List.h"
#include "Diagnosis.h"

namespace e
{
	template<typename T> class Alloctator
	{
	public:
		Alloctator(void)
		{
			allocCount = 0;
			maxAllocCount = 1024;
		}

		Alloctator(size_t maxAllocCount)
		{
			this->allocCount = 0;
			this->maxAllocCount = maxAllocCount;
		}

		virtual ~Alloctator(void)
		{
			Clear();
		}

		T* Alloc(void)
		{
			if (!list.empty())
			{
				List<T*>::iterator it = list.begin();
				list.erase(it);
				return *it;
			}
			else
			{
				T * p = (T*)malloc(sizeof(T));
				if (p != 0)
				{
					new(p)T();
					allocCount++;
				}
				return p;
			}
		}

		void Release(T * v)
		{
			if (allocCount <= maxAllocCount)
			{
				list.push_back(v);
			}
			else
			{
				v->~T();
				free(v);
				allocCount--;
			}
		}

		void Clear(void)
		{
			List<T*>::iterator it = list.begin();
			while (it != list.end())
			{
				(*it)->~T();
				free(*it);
				it++;
			}
		}
	private:
		List<T*> list;
		size_t allocCount;
		size_t maxAllocCount;
	};
}

#endif
