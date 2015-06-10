#ifndef __CORE_THREAD_H__
#define __CORE_THREAD_H__

namespace e
{
	class Mutex
	{
	public:
		Mutex(void);
		~Mutex(void);
		void Lock(void);
		void Unlock(void);
	private:
		void* native;
	};

	class ScopeLock
	{
	public:
		ScopeLock(Mutex & mutex);
		~ScopeLock(void);
	private:
		Mutex & mutex;
	};
}

#endif
