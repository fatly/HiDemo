#ifndef __E_OBJECT_H__
#define __E_OBJECT_H__

namespace e
{
	class Object
	{
	public:
		virtual ~Object(void) = 0
		{

		}
	};

	class RefCountObj : public Object
	{
	public:
		RefCountObj(void) : refCount(0)
		{

		}
		virtual ~RefCountObj(void)
		{

		}
		void AddRef(void)
		{
			refCount++;
		}
		void Release(void)
		{
			if (--refCount <= 0)
			{
				delete this;
			}
		}
	private:
		volatile int refCount;
	};
}

#endif