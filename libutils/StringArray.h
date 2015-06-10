#ifndef __CORE_STRINGARRAY_H__
#define __CORE_STRINGARRAY_H__
#include "String.h"
#include "Array.h"

namespace e
{
	typedef Array<String> StringArray;

	StringArray Split(const String & src, const Char* ch, int from = 0, int len = -1);
}

#endif
