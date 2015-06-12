#ifndef __CORE_DIAGNOSIS_H__
#define __CORE_DIAGNOSIS_H__
#include "IString.h"
#include <stdlib.h>

namespace e
{
	//base assert
#ifdef _DEBUG
#	ifdef _MSC_VER
#		define DEBUG_BREAK __debugbreak()
#	else
#		define DEBUG_BREAK asm("int $3")
#	endif
#	define BASIC_ASSERT(x) do {if (!(x)) {DEBUG_BREAK;} }while(0);
#else
#	define DEBUG_BREAK ((void)0)
#	define BASIC_ASSERT(x) ((void)0)
#endif

#ifdef _DEBUG
	void OnAssertFailed(const String & msg, const String & file, const int line);
	void DebugWrite(const String & msg);
	void DebugWrite(const String & msg, const String & file, const int line);
	void DebugWriteLine(const String & msg);
	void DebugWriteLine(const String & msg, const String & file, const int line);

#ifndef ASSERT
#	define ASSERT(x) do {if (!(x)){ e::OnAssertFailed("Assert Failed : " #x, __FILE__, __LINE__); DEBUG_BREAK; }} while (0);
#endif
//#	define TRACE(x) e::DebugWrite((x), __FILE__, __LINE__)
//#	define TRACE_LINE(x) e::DebugWriteLine((x), __FILE__, __LINE__)
#	define TRACE(x) e::DebugWrite(x)
#	define TRACE_LINE(x) e::DebugWriteLine(x)
#else
#	define ASSERT(x)		((void)0)
#	define TRACE(x)			((void)0)
#	define TRACE_LINE(x)	((void)0)
#endif

	class Exception
	{
	public:
		Exception(const String & what, const char* file = 0, int line = -1);
		virtual ~Exception(void);
		String GetText(void);
	private:
		String _what;
		const char* _file;
		int _line;
	};

	String GetOSErrorText(void);
	//–¥»’÷æ
	bool WriteLog(const String & category, const String & text);

#define THROW(msg) throw(e::Exception((msg), __FILE__, __LINE__))
}

//memory diagnosis
#if defined(_DEBUG) && defined(MEMTRACE)

namespace e
{
	enum MemoryAllocType
	{
		MAT_C,
		MAT_NEW,
		MAT_NEWARRAY
	};

	void  set_debug_memory_break_at_alloc(int order);
	void* debug_malloc(size_t sz, const char* file, const int line, MemoryAllocType type);
	void* debug_realloc(void* p, size_t sz, const char* file, const int line);
	void* debug_calloc(size_t num, size_t sz, const char* file, const int line);
	void  debug_free(void* p, MemoryAllocType type);
	void  debug_change_location(void* p, const char* file, const int line);

#define malloc(sz)		e::debug_malloc((sz), __FILE__, __LINE__, e::MAT_C)
#define realloc(p, sz)	e::debug_realloc((p), (sz), __FILE__, __LINE__)
#define calloc(num, sz)	e::debug_calloc((num), (sz), __FILE__, __LINE__)
#define free(p)			e::debug_free((p), e::MAT_C)
}
	void* operator new(size_t);
	void operator delete(void*);
	void* operator new[](size_t);
	void operator delete[](void*);
	void* operator new(size_t, const char*, int);
	void operator delete(void*, const char*, int);
	void* operator new[](size_t, const char*, int);
	void operator delete[](void*, const char*, int);

#endif//end of ifdef _DEBUG

#endif
