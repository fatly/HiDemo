#include "private.h"
#include "Thread.h"
#include "Diagnosis.h"

#ifdef MEMTRACE
#undef malloc
#undef free
#endif

namespace e
{
	Mutex::Mutex(void)
	{
		native = malloc(sizeof(CRITICAL_SECTION));
		::InitializeCriticalSection((CRITICAL_SECTION*)native);
	}

	Mutex::~Mutex(void)
	{
		::DeleteCriticalSection((CRITICAL_SECTION*)native);
		free(native);
	}

	void Mutex::Lock(void)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)native);
	}

	void Mutex::Unlock(void)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)native);
	}

	ScopeLock::ScopeLock(Mutex & m) 
		: mutex(m)
	{
		mutex.Lock();
	}

	ScopeLock::~ScopeLock(void)
	{
		mutex.Unlock();
	}
}
