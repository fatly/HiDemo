#ifndef __CORE_AUTOPTR_H__
#define __CORE_AUTOPTR_H__
#include "Object.h"

namespace e
{
	class RefCountObj;
	template<typename T> class AutoPtr
	{
	public:
		AutoPtr(T * p = 0)
		{
			this->p = p;

			if (this->p)
			{
				_Ptr()->AddRef();
			}
		}

		AutoPtr(const AutoPtr<T> & t)
		{
			this->p = t.p;

			if (this->p)
			{
				_Ptr()->AddRef();
			}
		}

		virtual ~AutoPtr(void)
		{
			if (this->p)
			{
				_Ptr()->Release();
			}
		}

		const AutoPtr<T> & operator =(const AutoPtr<T> & p)
		{
			if (this->p != p)
			{
				if (this->p)
				{
					_Ptr()->Release();
				}

				this->p = p;

				if (p)
				{
					_Ptr()->AddRef();
				}
			}

			return (*this);
		}

		const AutoPtr<T> & operator =(T * p)
		{
			if (this->p)
			{
				_Ptr()->Release();
			}

			this->p = p;

			if (this->p)
			{
				_Ptr()->AddRef();
			}

			return (*this);
		}

		void Clear(void)
		{
			if (this->p)
			{
				_Ptr()->Release();
			}

			this->p = 0;
		}
	public:
		T * operator->(void) const
		{
			return p;
		}

		T & operator * (void) const
		{
			return *p;
		}

		operator T * (void) const
		{
			return p;
		}

		bool operator==(const T * p)
		{
			return this->p == p;
		}

		bool operator!=(const T * p)
		{
			return this->p != p;
		}

		bool operator==(const AutoPtr<T> & t)
		{
			return this->p == t.p;
		}

		bool operator!=(const AutoPtr<T> & t)
		{
			return this->p != t.p;
		}

		RefCountObj * _Ptr(void)
		{
			return (RefCountObj*)(p);
		}
	public:
		T * p;
	};
}

#endif
