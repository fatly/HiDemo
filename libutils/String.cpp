#include "private.h"
#include "IString.h"
#include "Diagnosis.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#undef malloc
#undef realloc
#undef calloc
#undef free

namespace e
{
	static int g_loacale_inited = false;
	static inline void InitLocale(void)
	{
		const char* loacale_string = setlocale(LC_CTYPE, "");
		g_loacale_inited = true;
		TRACE(TEXT("[library] : locale set to:" + StringW(loacale_string) + StringW("\n")));
	}

	template<typename T>
	static inline int CompareNoCase(const T* a, const T* b)
	{
		T left, right;
		while (*a)
		{
			left = *a++;
			if (left >= 'A' && left <= 'Z')
			{
				left += (int)'a' - (int)'A';
			}
			right = *b++;
			if (right >= 'A' && right <= 'Z')
			{
				right += (int)'a' - (int)'A';
			}

			int i = left - right;
			if (i != 0)
			{
				return i;
			}
		}
		return *b ? -1 : 0;
	}

	StringA::StringA(void) throw()
	{
		buffer = (char*)malloc(1);
		buffer[0] = 0;
		bufferSize = 1;
	}

	StringA::StringA(const char* r) throw()
	{
		if (r == 0)
		{
			buffer = (char*)malloc(1);
			buffer[0] = 0;
			bufferSize = 1;
			return;
		}

		int len = strlen(r);
		buffer = (char*)malloc(len + 1);
		bufferSize = len + 1;
		strcpy_s(buffer, bufferSize, r);
	}

	StringA::StringA(const StringA & r) throw()
	{
		bufferSize = strlen(r.buffer) + 1;
		buffer = (char*)malloc(bufferSize);
		strcpy_s(buffer, bufferSize, r.buffer);
	}

	StringA::StringA(const wchar_t* p, Charset charset) throw()
	{
		Initialize(p, charset);
	}

	StringA::StringA(const StringW & r, Charset charset) throw()
	{
		Initialize(r.c_str(), charset);
	}

	StringA::StringA(char ch)
	{
		bufferSize = 2;
		buffer = (char*)malloc(bufferSize);
		buffer[0] = ch;
		buffer[1] = 0;
	}

	StringA::StringA(int value)
	{
		char buf[32] = { 0 };
		sprintf_s(buf, "%d", value);
		bufferSize = strlen(buf) + 1;
		buffer = (char*)malloc(bufferSize);
		strcpy_s(buffer, bufferSize, buf);
	}

	StringA::StringA(uint value)
	{
		char buf[32] = { 0 };
		sprintf_s(buf, "%u", value);
		bufferSize = strlen(buf) + 1;
		buffer = (char*)malloc(bufferSize);
		strcpy_s(buffer, bufferSize, buf);
	}

	StringA::StringA(float value)
	{
		char buf[32] = { 0 };
		sprintf_s(buf, "%f", value);
		bufferSize = strlen(buf) + 1;
		buffer = (char*)malloc(bufferSize);
		strcpy_s(buffer, bufferSize, buf);
	}

	StringA::StringA(double value)
	{
		char buf[32] = { 0 };
		sprintf_s(buf, "%lf", value);
		bufferSize = strlen(buf) + 1;
		buffer = (char*)malloc(bufferSize);
		strcpy_s(buffer, bufferSize, buf);
	}

	StringA::StringA(bool value)
	{
		bufferSize = 6;
		buffer = (char*)malloc(bufferSize);
		strcpy_s(buffer, bufferSize, value ? "true" : "false");
	}

	StringA::~StringA(void)
	{
		if (buffer != 0)
		{
			free(buffer);
			buffer = 0;
		}
		bufferSize = 0;
	}

	int StringA::Length(void) const
	{
		return strlen(buffer);
	}

	int StringA::Compare(const StringA & r) const
	{
		return strcmp(buffer, r.buffer);
	}

	int StringA::CompareNoCase(const StringA & r) const
	{
		return e::CompareNoCase<char>(buffer, r.buffer);
	}

	void StringA::Reserve(int capacity)
	{
		int length = Length();
		if (capacity < length)
		{
			capacity = length;
		}

		if (bufferSize - 1 < capacity)
		{
			bufferSize = capacity + 1;
			buffer = (char*)realloc(buffer, bufferSize);
		}
	}

	char* StringA::c_str(void) const
	{
		return buffer;
	}

	char* StringA::GetData(void)
	{
		return buffer;
	}

	void StringA::Append(char ch)
	{
		int len = Length();
		Reserve(len + 1);
		buffer[len++] = ch;
		buffer[len] = 0;
	}

	void StringA::Append(const StringA & r)
	{
		int len = Length() + r.Length();
		Reserve(len);
		strcat_s(buffer, bufferSize, r.buffer);
	}

	void StringA::Insert(int index, char ch)
	{
		int len = Length();
		ASSERT(index >= 0 && index <= len);
		Reserve(len + 1);
		for (int i = len + 1; i >= index; i--)
		{
			buffer[i] = buffer[i - 1];
		}
		buffer[index] = ch;
	}

	void StringA::Insert(int index, const StringA & r)
	{
		if (r.IsEmpty())
		{
			return;
		}

		int len = Length();
		ASSERT(index >= 0 && index <= len);
		int len1 = r.Length();
		Reserve(len + len1);
		char * p0 = buffer + len;
		char * pe = buffer + index;
		char * p1 = buffer + len + len1;

		while (p0 >= pe)
		{
			*p1-- = *p0--;
		}
		memcpy(pe, r.buffer, len1 * sizeof(char));
	}

	int StringA::FindChar(char ch, int pos /* = 0 */) const
	{
		ASSERT(pos >= 0);

		if (pos >= 0 && pos < Length())
		{
			char * p = strchr(buffer + pos, ch);

			if (p != 0)
			{
				return (int)(p - buffer);
			}
		}

		return -1;
	}

	int StringA::FindChar(const char* p, int pos /* = 0 */) const
	{
		ASSERT(pos >= 0);
		if (pos < Length())
		{
			char * p1 = 0;

			while (*p)
			{
				char * p2 = strchr(buffer + pos, *p);
				p1 = (p2 == 0) ? p1 : (p1 < p2&&p1 != 0 ? p1 : p2);
				p++;
			}

			if (p1 != 0)
			{
				return (int)(p1 - buffer);
			}
		}

		return -1;
	}

	int StringA::FindString(const char* p, int pos /* = 0 */) const
	{
		ASSERT(pos >= 0);
		if (pos >= 0 && pos < Length())
		{
			const char* p1 = buffer + pos;
			const char* p2 = strstr(p1, p);

			if (p2 != 0)
			{
				return pos + (p2 - p1);
			}
		}

		return -1;
	}

	int StringA::ReverseFindChar(char ch, int pos /* = -1 */) const
	{
		ASSERT(pos >= 0 || pos == -1);

		if (pos == -1)
		{
			pos = Length();
		}

		for (; pos >= 0; pos--)
		{
			if (buffer[pos] == ch)
			{
				return pos;
			}
		}

		return -1;
	}

	int StringA::Replace(char from, char to)
	{
		int count = 0;
		char * p = buffer;

		while (*p)
		{
			if (*p == from)
			{
				*p = to;
				count++;
			}
			p++;
		}

		return count;
	}

	int StringA::Replace(const char * from, const char* to)
	{
		int len1 = strlen(from);
		int len2 = strlen(to);
		int len3 = len2 - len1;
		int count = 0;
		const char* p;

		for (p = buffer; *p;)
		{
			if (memcmp(p, from, len1 * sizeof(char)))
			{
				count++;
				p += len1;
			}
			else
			{
				p++;
			}
		}

		size_t newSize = bufferSize + count * len3;
		char* newBuffer = (char*)malloc(newSize * sizeof(char));
		memset(newBuffer, 0, newSize * sizeof(char));
		char* p1 = newBuffer;
		for (p = buffer; *p;)
		{
			if (memcmp(p, from, len1 * sizeof(char)) == 0)
			{
				memcpy(p1, to, len2 * sizeof(char));
				p1 += len2;
				p += len1;
			}
			else
			{
				*p1 = *p;
				p++;
				p1++;
			}
		}

		free(buffer);
		buffer = newBuffer;
		bufferSize = newSize;

		return count;
	}

	StringA StringA::SubStr(int from, int len) const
	{
		int len1 = Length();
		if (from >= len1)
		{
			return "";
		}

		if (len == -1 || from + len > len1)
		{
			len = len1 - from;
		}

		StringA ret;
		ret.Reserve(len);
		memcpy(ret.buffer, buffer + from, len * sizeof(char));
		ret.buffer[len] = 0;

		return ret;
	}

	bool StringA::IsEmpty(void) const
	{
		return Length() == 0;
	}

	void StringA::Clear(void)
	{
		free(buffer);
		buffer = (char*)malloc(1);
		buffer[0] = 0;
		bufferSize = 1;
	}

	void StringA::Trim(void)
	{
		char * a = buffer;
		char * b = buffer + strlen(buffer) - 1;
		while (*a && isspace(*a)) a++;
		while (b >= a && isspace(*b)) b--;
		int len = b - a + 1;

		if (len > 0)
		{
			memmove(buffer, a, len);
			buffer[len] = 0;
		}
		else
		{
			buffer[0] = 0;
		}
	}

	void StringA::Trim(const char * p)
	{
		char * a = buffer;
		char * b = buffer + strlen(buffer) - 1;
		while (*a && strchr(p, *a)) a++;
		while (b >= a && strchr(p, *b)) b--;
		int len = b - a + 1;

		if (len > 0)
		{
			memmove(buffer, a, len);
			buffer[len] = 0;
		}
		else
		{
			buffer[0] = 0;
		}
	}

	void StringA::ToUpper(void)
	{
		char* p = buffer;

		while (*p)
		{
			*p = toupper(*p);
			p++;
		}
	}

	void StringA::ToLower(void)
	{
		char* p = buffer;

		while (*p)
		{
			*p = tolower(*p);
			p++;
		}
	}

	int StringA::ToInt(void) const
	{
		return atoi(buffer);
	}

	uint StringA::ToUint(void) const
	{
		return static_cast<uint>(atoi(buffer));
	}

	float StringA::ToFloat(void) const
	{
		return (float)atof(buffer);
	}

	double StringA::ToDouble(void) const
	{
		return atof(buffer);
	}

	bool StringA::ToBool(void) const
	{
		if (Length() == 0)
		{
			return false;
		}

		char ch = buffer[0];
		return ch == 'T' || ch == 't' || ch == '1';
	}

	char StringA::operator[](int index) const
	{
		ASSERT(index >= 0 && index <= Length());
		return buffer[index];
	}

	char & StringA::operator[](int index)
	{
		ASSERT(index >= 0 && index <= Length());
		return buffer[index];
	}

	bool StringA::operator==(const StringA & r) const
	{
		return Compare(r) == 0;
	}

	bool StringA::operator!=(const StringA & r) const
	{
		return Compare(r) != 0;
	}

	bool StringA::operator<(const StringA & r) const
	{
		return Compare(r) < 0;
	}

	bool StringA::operator>(const StringA & r) const
	{
		return Compare(r) > 0;
	}

	StringA StringA::operator+(const StringA & r) const
	{
		StringA s(*this);
		s.Reserve(s.Length() + r.Length());
		strcat_s(s.buffer, s.bufferSize, r.buffer);
		return s;
	}

	const StringA & StringA::operator=(const StringA & r) throw()
	{
		if (this != &r)
		{
			Reserve(strlen(r.buffer) + 1);
			strcpy_s(buffer, bufferSize, r.buffer);
		}

		return *this;
	}

	const StringA & StringA::operator+=(const StringA & r) throw()
	{
		if (this != &r)
		{
			Reserve(Length() + r.Length() + 1);
			strcat_s(buffer, bufferSize, r.buffer);
		}

		return *this;
	}

	StringA StringA::Format(const char * format, va_list argptr)
	{
		StringA ret;
#ifdef _MSC_VER
		int totalBytes = _vscprintf(format, argptr);
		ret.Reserve(totalBytes);
		vsprintf_s(ret.c_str(), totalBytes, format, argptr);
#else
		int totalBytes = vsnprintf(0, 0, format, argptr) + 1;
		ret.Reserve(totalBytes);
		vsnprintf(ret.c_str(), totalBytes, format, argptr);
#endif
		return ret;
	}

	StringA StringA::Format(const char * format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		String ret = Format(format, argptr);
		va_end(argptr);
		return ret;
	}

	static inline int wsctombs_len(const wchar_t * p)
	{
		if (!g_loacale_inited)
		{
			InitLocale();
		}

		int ret = 0;
		char buffer[10] = { 0 };

		while (*p != 0)
		{
			int n;
			if (wctomb_s(&n, buffer, sizeof(buffer), *p) != 0)
			{
				return -1;
			}
			ret += n;
			p++;
		}

		return ret;
	}

	void StringA::Initialize(const wchar_t* r, Charset charset /* = CHARSET_LOCALE */) throw()
	{
		if (r == 0)
		{
			buffer = (char*)malloc(1);
			buffer[0] = '\0';
			bufferSize = 1;
			return;
		}

		switch (charset)
		{
		case CHARSET_LOCALE:
		{
			bufferSize = wsctombs_len(r) + 1;
			if (bufferSize == 0)
			{
				buffer = (char*)malloc(1);
				bufferSize = 1;
				buffer[0] = 0;
				return;
			}

			size_t i = 0;
			buffer = (char*)malloc(bufferSize);
			size_t len = wcstombs_s(&i, buffer, bufferSize, r, bufferSize);
			if (len == (size_t)-1)
			{
				buffer[0] = 0;
				TRACE(L"[library] StringA:Initialize() fail to convert from Unicode to Locale.");
			}
		}
		break;
		case CHARSET_UTF8:
		{	/*
			U-00000000 - U-0000007F:  0xxxxxxx
			U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
			U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
			U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			*/

			const wchar_t * p = r;
			int charCount = 0;

			while (*p != 0)
			{
				uint32 ch = *p;
				if (ch <= 0x0000007F)
				{
					charCount += 1;
				}
				else if (ch <= 0x000007FF)
				{
					charCount += 2;
				}
				else if (ch <= 0x0000FFFF)
				{
					charCount += 3;
				}
				else if (ch <= 0x001FFFFF)
				{
					charCount += 4;
				}
				else if (ch <= 0x03FFFFFF)
				{
					charCount += 5;
				}
				else if (ch <= 0x7FFFFFFF)
				{
					charCount += 6;
				}
				else
				{
					TRACE(L"([library] StringA:Initialize() This char can not convert to UTF-8: ch = " + String((int)(ch)));
					charCount = -1;
					break;
				}
				p++;
			}

			if (charCount < 0)
			{
				bufferSize = 1;
				buffer = (char*)malloc(bufferSize);
				buffer[0] = 0;
				return;
			}

			bufferSize = charCount + 1;
			buffer = (char*)malloc(bufferSize);
			p = r;
			char * p1 = buffer;
			while (*p)
			{
				/*
				U-00000000 - U-0000007F:  0xxxxxxx
				U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
				U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
				U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
				U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

				*/
				uint32 ch = *p;
				if (ch <= 0x0000007F)
				{
					p1[0] = ch & 0x7f;
					p1++;
				}
				else if (ch <= 0x000007FF)
				{
					p1[0] = ((ch >> 6) & 0x1f) | 0xc0;
					p1[1] = ((ch >> 0) & 0x3f) | 0x80;
					p1 += 2;
				}
				else if (ch <= 0x0000FFFF)
				{
					p1[0] = ((ch >> 12) & 0x0f) | 0xe0;
					p1[1] = ((ch >> 6) & 0x3f) | 0x80;
					p1[2] = ((ch >> 0) & 0x3f) | 0x80;
					p1 += 3;
				}
				else if (ch <= 0x001FFFFF)
				{
					p1[0] = ((ch >> 18) & 0x07) | 0xf0;
					p1[1] = ((ch >> 12) & 0x3f) | 0x80;
					p1[2] = ((ch >> 6) & 0x3f) | 0x80;
					p1[3] = ((ch >> 0) & 0x3f) | 0x80;
					p1 += 4;
				}
				else if (ch <= 0x03FFFFFF)
				{
					p1[0] = ((ch >> 24) & 0x03) | 0xf8;
					p1[1] = ((ch >> 18) & 0x3f) | 0x80;
					p1[2] = ((ch >> 12) & 0x3f) | 0x80;
					p1[3] = ((ch >> 6) & 0x3f) | 0x80;
					p1[4] = ((ch >> 0) & 0x3f) | 0x80;
					p1 += 5;
				}
				else if (ch <= 0x7FFFFFFF)
				{
					p1[0] = ((ch >> 30) & 0x01) | 0xfc;
					p1[1] = ((ch >> 24) & 0x3f) | 0x80;
					p1[2] = ((ch >> 18) & 0x3f) | 0x80;
					p1[3] = ((ch >> 12) & 0x3f) | 0x80;
					p1[4] = ((ch >> 6) & 0x3f) | 0x80;
					p1[5] = ((ch >> 0) & 0x3f) | 0x80;
					p1 += 6;
				}
				else
				{
					ASSERT(0);
				}
				p++;
			}
			*p1 = 0;

			break;
		}
		default:
		{
			TRACE(L"[library] (StringA::Initialize() Unsupported Charset.");
			bufferSize = wcslen(r) + 1;
			buffer = (char*)malloc(bufferSize * sizeof(char));
			char * p = buffer;
			do
			{
				*p++ = (char)(*r);
			} while (*r++);
		}
		break;
		}
	}

	/////////////////////////////////StringW//////////////////////////////////
	static inline int mbstowcs_len(const char* r)
	{
		if (!g_loacale_inited)
		{
			InitLocale();
		}

		size_t len = strlen(r);
		int ret = 0;
		wchar_t buf[10];
		while (*r != 0)
		{
			size_t n = mbtowc(buf, r, len);
			if (n == (size_t)-1)
			{
				return -1;
			}
			ret++;
			r += n;
			len -= n;
		}

		return ret;
	}

	StringW::StringW(void) throw()
	{
		bufferSize = 1;
		buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
		buffer[0] = 0;
	}

	StringW::StringW(const wchar_t * r) throw()
	{
		if (r == 0)
		{
			bufferSize = 1;
			buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
			buffer[0] = 0;
			return;
		}

		bufferSize = wcslen(r) + 1;
		buffer = (wchar_t *)malloc(bufferSize * sizeof(wchar_t));
		wcscpy_s(buffer, bufferSize, r);
	}

	StringW::StringW(const StringW & r) throw()
	{
		bufferSize = wcslen(r.buffer) + 1;
		buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
		wcscpy_s(buffer, bufferSize, r.buffer);
	}

	StringW::StringW(const char * p, Charset charset) throw()
	{
		Initialize(p, charset);
	}

	StringW::StringW(const StringA & r, Charset charset) throw()
	{
		Initialize(r.c_str(), charset);
	}

	void InitTextWFromBuffer(wchar_t * & buffer, int & size, const char * src)
	{
		size = strlen(src) + 1;
		buffer = (wchar_t*)malloc(size * sizeof(wchar_t));
		const char* p1 = src;
		wchar_t* p2 = buffer;
		do{
			*p2++ = *p1++;
		} while (*p1);
		*p2 = 0;
	}

	StringW::StringW(wchar_t ch)
	{
		bufferSize = 2;
		buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
		buffer[0] = ch;
		buffer[1] = 0;
	}

	StringW::StringW(int value)
	{
		char src[64] = { 0 };
		sprintf_s(src, "%d", value);
		InitTextWFromBuffer(buffer, bufferSize, src);
	}

	StringW::StringW(uint value)
	{
		char src[64] = { 0 };
		sprintf_s(src, "%u", value);
		InitTextWFromBuffer(buffer, bufferSize, src);
	}

	StringW::StringW(float value)
	{
		char src[64] = { 0 };
		sprintf_s(src, "%f", value);
		InitTextWFromBuffer(buffer, bufferSize, src);
	}

	StringW::StringW(double value)
	{
		char src[64] = { 0 };
		sprintf_s(src, "%lf", value);
		InitTextWFromBuffer(buffer, bufferSize, src);
	}

	StringW::StringW(bool value)
	{
		bufferSize = 6;
		buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
		wcscpy_s(buffer, bufferSize, value ? L"true" : L"false");
	}

	StringW::~StringW(void)
	{
		if (buffer != 0)
		{
			free(buffer);
			buffer = 0;
		}

		bufferSize = 0;
	}

	int StringW::Length(void) const
	{
		return wcslen(buffer);
	}

	void StringW::Reserve(int capacity)
	{
		int length = Length();

		if (capacity < length)
		{
			capacity = length;
		}

		if (bufferSize - 1 < capacity)
		{
			bufferSize = capacity + 1;
			buffer = (wchar_t*)realloc(buffer, bufferSize * sizeof(wchar_t));
		}
	}

	wchar_t* StringW::c_str(void) const
	{
		return buffer;
	}

	wchar_t* StringW::GetData(void) const
	{
		return buffer;
	}

	int StringW::Compare(const StringW & r) const
	{
		return wcscmp(buffer, r.buffer);
	}

	int StringW::CompareNoCase(const StringW & r) const
	{
		return e::CompareNoCase<wchar_t>(buffer, r.buffer);
	}

	void StringW::Append(wchar_t ch)
	{
		int length = Length();
		Reserve(length + 1);
		buffer[length++] = ch;
		buffer[length] = 0;
	}

	void StringW::Append(const StringW & r)
	{
		int length = Length() + r.Length();
		Reserve(length + 1);
		wcscat_s(buffer, bufferSize, r.buffer);
	}

	void StringW::Insert(int index, const StringW & r)
	{
		if (r.IsEmpty()) return;
		int length = Length();
		ASSERT(index >= 0 && index <= length);
		int length1 = r.Length();
		Reserve(length + length1);
		wchar_t* p0 = buffer + length;
		wchar_t* pe = buffer + index;
		wchar_t* p1 = buffer + length + length1;
		while (p0 >= pe)
		{
			*p1-- = *p0--;
		}
		memcpy(pe, r.buffer, length1 * sizeof(wchar_t));
	}

	void StringW::Insert(int index, const wchar_t ch)
	{
		int length = Length();
		ASSERT(index >= 0 && index <= length);
		Reserve(length + 1);
		for (int i = length + 1; i > index; i--)
		{
			buffer[i] = buffer[i - 1];
		}
		buffer[index] = ch;
	}

	int StringW::FindChar(wchar_t ch, int from) const
	{
		ASSERT(from >= 0);
		if (from >= 0 && from <= Length())
		{
			wchar_t* p = wcschr(buffer + from, ch);
			if (p != 0)
			{
				return (int)(p - buffer);
			}
		}

		return -1;
	}

	int StringW::FindChar(const wchar_t * p, int from /* = 0 */) const
	{
		ASSERT(from >= 0);
		if (from < Length())
		{
			wchar_t* p1 = 0;
			while (*p)
			{
				wchar_t* p2 = wcschr(buffer + from, *p);
				p1 = p2 == 0 ? p1 : (p1 < p2 && p1 != 0 ? p1 : p2);
				p++;
			}
			if (p1 != 0)
			{
				return (int)(p1 - buffer);
			}
		}

		return -1;
	}

	int StringW::FindString(const wchar_t * p, int from /* = 0 */) const
	{
		ASSERT(from >= 0);
		if (from >= 0 && from < Length())
		{
			wchar_t* p1 = buffer + from;
			wchar_t* p2 = wcsstr(p1, p);
			if (p2 != 0)
			{
				return from + (p2 - p1);
			}
		}

		return -1;
	}

	int StringW::ReverseFindChar(wchar_t ch, int from /* = -1 */) const
	{
		ASSERT(from >= 0 || from == -1);
		if (from == -1)
		{
			from = Length();
		}

		for (; from >= 0; from--)
		{
			if (buffer[from] == ch)
			{
				return from;
			}
		}

		return -1;
	}

	int StringW::Replace(wchar_t from, wchar_t to)
	{
		int count = 0;
		int length = Length();
		for (int i = 0; i < length; i++)
		{
			if (buffer[i] == from)
			{
				buffer[i] = to;
				count++;
			}
		}

		return count;
	}

	int StringW::Replace(const wchar_t * from, const wchar_t * to)
	{
		int len1 = wcslen(from);
		int len2 = wcslen(to);
		int delta = len2 - len1;
		int count = 0;
		wchar_t* p = 0;
		for (p = buffer; *p;)
		{
			if (memcmp(p, from, len1 * sizeof(wchar_t)) == 0)
			{
				count++;
				p += len1;
			}
			else
			{
				p++;
			}
		}

		size_t newSize = bufferSize + delta * count;
		wchar_t* newBuffer = (wchar_t*)malloc(newSize * sizeof(wchar_t));
		memset(newBuffer, 0, newSize*sizeof(wchar_t));

		wchar_t* p1 = newBuffer;
		for (p = buffer; *p;)
		{
			if (memcmp(p, from, len1 * sizeof(wchar_t)) == 0)
			{
				memcpy(p1, to, len2*sizeof(wchar_t));
				p += len1;
				p1 += len2;
			}
			else
			{
				*p1++ = *p++;
			}
		}

		free(buffer);
		buffer = newBuffer;
		bufferSize = newSize;

		return count;
	}

	StringW StringW::SubStr(int from, int len) const
	{
		int length = Length();
		if (from >= length)
		{
			return L"";
		}
		if (len == -1 || from + len > length)
		{
			len = length - from;
		}

		StringW ret;
		ret.Reserve(len);
		memcpy(ret.buffer, buffer + from, len * sizeof(wchar_t));
		ret.buffer[len] = 0;
		return ret;
	}

	bool StringW::IsEmpty(void) const
	{
		return Length() == 0;
	}

	void StringW::Clear(void)
	{
		free(buffer);
		bufferSize = 1;
		buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
		buffer[0] = 0;
	}

	StringW StringW::Format(const wchar_t * format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		StringW ret = Format(format, argptr);
		va_end(argptr);

		return ret;
	}

	StringW StringW::Format(const wchar_t * format, va_list argptr)
	{
		StringW ret;
#ifdef _MSC_VER
		int len = _vscwprintf(format, argptr) + 1;
		ret.Reserve(len);
		vswprintf_s(ret.c_str(), len, format, argptr);
#else
		int len = 4096;
		ret.Reserve(len);
		vswprintf(ret.c_str(), len, format, argptr);
#endif
		return ret;
	}

	void StringW::Trim(void)
	{
		wchar_t * a = buffer;
		wchar_t * b = buffer + Length() - 1;
		while (*a && isspace(*a)) a++;
		while (b > a && isspace(*b)) b--;
		int len = b - a + 1;
		if (len > 0)
		{
			memmove(buffer, a, len * sizeof(wchar_t));
			buffer[len] = 0;
		}
		else
		{
			buffer[0] = 0;
		}
	}

	void StringW::Trim(const wchar_t* p)
	{
		wchar_t* a = buffer;
		wchar_t* b = buffer + Length() - 1;
		while (*a && wcschr(p, *a)) a++;
		while (b > a && wcschr(p, *b)) b--;
		int len = b - a + 1;
		if (len > 0)
		{
			memmove(buffer, a, len * sizeof(wchar_t));
			buffer[len] = 0;
		}
		else
		{
			buffer[0] = 0;
		}
	}

	void StringW::ToUpper(void)
	{
		wchar_t* p = buffer;
		while (*p)
		{
			*p = toupper(*p);
			p++;
		}
	}

	void StringW::ToLower(void)
	{
		wchar_t* p = buffer;
		while (*p)
		{
			*p = tolower(*p);
			p++;
		}
	}

	int StringW::ToInt(void) const
	{
		int value = 0;
		bool negative = false;
		wchar_t* p = buffer;
		while (*p  && !iswdigit(*p) && *p != '-') p++;

		if (*p == '-')
		{
			negative = true;
			p++;
		}
		else
		{
			negative = false;
		}

		while (*p >= '0' && *p <= '9')
		{
			value *= 10;
			value += *p++ - '0';
		}

		if (negative)
		{
			value = -value;
		}

		return value;
	}

	uint StringW::ToUint(void) const
	{
		return 0;
	}

	float StringW::ToFloat(void) const
	{
		wchar_t* p = 0;
		return (float)wcstod(buffer, &p);
	}

	double StringW::ToDouble(void) const
	{
		wchar_t* p = 0;
		return wcstod(buffer, &p);
	}

	bool StringW::ToBool(void) const
	{
		if (Length() == 0) return false;

		wchar_t ch = buffer[0];

		return ch == L'T' || ch == L't' || ch == L'1';
	}

	wchar_t StringW::operator[](int index) const
	{
		ASSERT(index >= 0 && index < bufferSize);
		return buffer[index];
	}

	wchar_t & StringW::operator[](int index)
	{
		ASSERT(index >= 0 && index < bufferSize);
		return buffer[index];
	}

	bool StringW::operator==(const StringW & r) const
	{
		return Compare(r) == 0;
	}

	bool StringW::operator!=(const StringW & r) const
	{
		return Compare(r) != 0;
	}

	bool StringW::operator<(const StringW & r) const
	{
		return Compare(r) < 0;
	}

	bool StringW::operator>(const StringW & r) const
	{
		return Compare(r) > 0;
	}

	StringW StringW::operator+(const StringW & r) const
	{
		StringW ret(*this);
		ret.Reserve(ret.Length() + r.Length());
		wcscat_s(ret.buffer, ret.bufferSize, r.buffer);
		return ret;
	}

	const StringW & StringW::operator=(const StringW & r) throw()
	{
		if (this != &r)
		{
			Reserve(r.Length() + 1);
			wcscpy_s(buffer, bufferSize, r.buffer);
		}

		return *this;
	}

	const StringW & StringW::operator+=(const StringW & r) throw()
	{
		if (this != &r)
		{
			Reserve(Length() + r.Length() + 1);
			wcscat_s(buffer, bufferSize, r.buffer);
		}

		return *this;
	}

	void StringW::Initialize(const char* p, Charset charset /* = CHARSET_LOCALE*/) throw()
	{
		if (p == 0)
		{
			bufferSize = 1;
			buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
			buffer[0] = 0;
			return;
		}

		switch (charset)
		{
		case CHARSET_LOCALE:
		{
			bufferSize = mbstowcs_len(p) + 1;

			if (bufferSize == 0)
			{
				bufferSize = 1;
				buffer = (wchar_t*)malloc(bufferSize * sizeof(bufferSize));
				buffer[0] = 0;
				return;
			}

			size_t i = 0;
			size_t totalBytes = bufferSize * sizeof(wchar_t);
			buffer = (wchar_t*)malloc(totalBytes);
			size_t len = mbstowcs_s(&i, buffer, bufferSize, p, totalBytes);
			if (len == (size_t)-1)
			{
				TRACE(L"[library] Fail to convert String from Locale to Unicode.");
				buffer[0] = 0;
			}
		}
		break;
		case CHARSET_UTF8:
		{
			const unsigned char* s = (unsigned char*)p;
			int charCount = 0;

			while (*s)
			{
				unsigned char ch = *s;
				if ((ch & 0xf8) == 0xf0)
				{
					s += 4;
				}
				else if ((ch & 0xf0) == 0xe0)
				{
					s += 3;
				}
				else if ((ch & 0xe0) == 0xc0)
				{
					s += 2;
				}
				else if ((ch & 0x80) == 0)
				{
					s += 1;
				}
				else
				{
					bufferSize = 1;
					buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
					buffer[0] = 0;
					return;
				}

				charCount++;
			}

			bufferSize = charCount + 1;
			size_t totalBytes = bufferSize * sizeof(wchar_t);
			buffer = (wchar_t*)malloc(totalBytes);
			s = (unsigned char*)p;
			wchar_t * s1 = buffer;

			while (*s != 0)
			{
				unsigned char ch = *s;
				if ((ch & 0xf8) == 0xf0)
				{
					*s1 = (wchar_t(ch & 0x07) << 18) | (wchar_t(s[1] & 0x3f) << 12) | (wchar_t(s[2] & 0x3f) << 6) | (wchar_t(s[3] & 0x3f) << 0);
					s += 4;
				}
				else if ((ch & 0xf0) == 0xe0)
				{
					*s1 = (wchar_t(ch & 0x0f) << 12) | (wchar_t(s[1] & 0x3f) << 6) | (wchar_t(s[2] & 0x3f) << 0);
					s += 3;
				}
				else if ((ch & 0xe0) == 0xc0)
				{
					*s1 = (wchar_t(ch & 0x1f) << 6) | (wchar_t(ch & 0x3f) << 0);
					s += 2;
				}
				else if ((ch & 0x80) == 0)
				{
					*s1 = ch & 0x7f;
					s += 1;
				}
				else
				{
					ASSERT(0);
				}
				s1++;
			}
			*s1 = 0;
		}
		break;
		default:
		{
			TRACE(L"[library] (StringW) Unsupported charset.");
			bufferSize = strlen(p) + 1;
			buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
			wchar_t* s = buffer;
			do
			{
				*s++ = *p++;
			} while (*p);
		}
		break;
		}
	}

	String BytesToMKBText(uint bytes)
	{
		if (bytes >= 900000)
		{
			float f = float(bytes) / float(1024 * 1024);
			int a = (int)f;
			int b = (int)(f * 10 + 0.5f) % 10;
			return String::Format(TEXT("%4d.%1dM"), a, b);
		}
		else if (bytes >= 900)
		{
			float f = float(bytes) / float(1024);
			int a = (int)f;
			int b = (int)(f * 10 + 0.5f) % 10;
			return String::Format(TEXT("%4d.%1dK"), a, b);
		}
		else
		{
			return String::Format(TEXT("%6dB"), int(bytes));
		}
	}


	String ByteToHex(uint8 byte)
	{
		//buf_size = 3;
		//buf = (wchar_t*) malloc(buf_size * sizeof(wchar_t));
		wchar_t buf[3];
		int i;
		i = (byte >> 4) & 0x0f;
		buf[0] = i > 9 ? 'A' + i - 10 : '0' + i;
		i = byte & 0x0f;
		buf[1] = i > 9 ? 'A' + i - 10 : '0' + i;
		buf[2] = 0;
		return buf;
	}

	String PointerToHex(void* p)
	{
		const int n = sizeof(p) * 2;
		wchar_t buf[n + 1];
		int j = sizeof(p) - 1;
		int m = 0;
		for (; m < n; j--)
		{
			uint8 b = ((uint8*)&p)[j];
			int i;
			i = (b >> 4) & 0x0f;
			buf[m++] = i > 9 ? 'A' + i - 10 : '0' + i;
			i = b & 0x0f;
			buf[m++] = i > 9 ? 'A' + i - 10 : '0' + i;
		}
		buf[m] = 0;
		return buf;
	}
}
