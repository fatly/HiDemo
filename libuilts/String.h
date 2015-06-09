#ifndef __E_STRING_H__
#define __E_STRING_H__

#include <stdarg.h>
#include "Defines.h"

namespace e
{
	enum Charset
	{
		CHARSET_UNKOWN,
		CHARSET_LOCALE = CHARSET_UNKOWN, // LC_CTYPE
		CHARSET_UTF8,
		CHARSET_UTF16LE, // Windows Unicode
		CHARSET_UTF16 = CHARSET_UTF16LE,
		CHARSET_UTF16BE,
		CHARSET_UTF32LE,
		CHARSET_UTF32 = CHARSET_UTF32LE,
		CHARSET_UTF32BE,
#ifdef LINUX
		CHARSET_INTERNAL = CHARSET_UTF32,
#else
		CHARSET_INTERNAL = CHARSET_UTF16,
#endif
	};

	class StringW;
	class StringA
	{
	public:
		StringA(void) throw();
		StringA(const char* r) throw();
		StringA(const StringA & r) throw();
		StringA(const wchar_t* r, Charset charset = CHARSET_LOCALE) throw();
		StringA(const StringW & r, Charset charset = CHARSET_LOCALE) throw();
		//explicit 不允许隐式调用构造函数
		explicit StringA(char ch);
		explicit StringA(int value);
		explicit StringA(uint value);
		explicit StringA(float value);
		explicit StringA(double value);
		explicit StringA(bool value);
		virtual ~StringA(void);
	public:
		int Length(void) const;
		int Compare(const StringA & r) const;
		int CompareNoCase(const StringA & r) const;
		void Reserve(int capacity);
		char* c_str(void) const;
		char* GetData(void);
		void Append(char ch);
		void Append(const StringA & r);
		void Insert(int index, char ch);
		void Insert(int index, const StringA & r);
		int FindChar(char ch, int pos = 0) const;
		int FindChar(const char* p, int pos = 0) const;
		int FindString(const char* p, int pos = 0) const;
		int ReverseFindChar(char ch, int pos = -1) const;
		int Replace(const char* from, const char* to);
		int Replace(char from, char to);
		StringA SubStr(int from, int len) const;
		bool IsEmpty(void) const;
		void Clear(void);
		void Trim(void);
		void Trim(const char* p);
		void ToUpper(void);
		void ToLower(void);
		//转换函数
		int ToInt(void) const;
		uint ToUint(void) const;
		float ToFloat(void) const;
		double ToDouble(void) const;
		bool ToBool(void) const;
		//重载操作符函数
		char operator[](int index) const;
		char & operator[](int index);
		bool operator==(const StringA & r) const;
		bool operator!=(const StringA & r) const;
		bool operator<(const StringA & r) const;
		bool operator>(const StringA & r) const;
		StringA operator+(const StringA & r) const;
		const StringA & operator=(const StringA & r) throw();
		const StringA & operator+=(const StringA & _r) throw();

		static StringA Format(const char* format, va_list argptr);
		static StringA Format(const char* format, ...);

	private:
		void Initialize(const wchar_t* r, Charset charset = CHARSET_LOCALE) throw();
	private:
		char* buffer;
		int bufferSize;
	};

	inline StringA operator+(const char* l, const StringA & r)
	{
		return StringA(l) + r;
	}

	inline StringA operator+(const wchar_t* l, const StringA & r)
	{
		return StringA(l) + r;
	}

	class StringW
	{
	public:
		StringW(void) throw();
		StringW(const wchar_t* r) throw();
		StringW(const StringW & r) throw();
		StringW(const char* r, Charset charset = CHARSET_LOCALE) throw();
		StringW(const StringA & r, Charset charset = CHARSET_LOCALE) throw();
		//explicit 不允许隐式调用构造函数
		explicit StringW(wchar_t ch);
		explicit StringW(int value);
		explicit StringW(uint value);
		explicit StringW(float value);
		explicit StringW(double value);
		explicit StringW(bool value);
		virtual ~StringW(void);

		int Length(void) const;
		void Reserve(int capacity);
		wchar_t* c_str(void) const;
		wchar_t* GetData(void) const;
		int Compare(const StringW & r) const;
		int CompareNoCase(const StringW & r) const;
		void Append(wchar_t ch);
		void Append(const StringW & r);
		void Insert(int index, const StringW & r);
		void Insert(int index, const wchar_t ch);
		int FindChar(wchar_t ch, int from = 0) const;
		int FindChar(const wchar_t* p, int from = 0) const;
		int FindString(const wchar_t* p, int from = 0) const;
		int ReverseFindChar(wchar_t ch, int from = -1) const;
		int Replace(wchar_t from, wchar_t to);
		int Replace(const wchar_t* from, const wchar_t* to);
		StringW SubStr(int from, int len) const;
		bool IsEmpty(void) const;
		void Clear(void);
		static StringW Format(const wchar_t* format, ...);
		static StringW Format(const wchar_t* format, va_list argptr);
		void Trim(void);
		void Trim(const wchar_t* p);
		void ToUpper(void);
		void ToLower(void);
		//转换函数
		int ToInt(void) const;
		uint ToUint(void) const;
		float ToFloat(void) const;
		double ToDouble(void) const;
		bool ToBool(void) const;
		//符号重载函数
		wchar_t operator[](int index) const;
		wchar_t & operator[](int index);
		bool operator==(const StringW & r) const;
		bool operator!=(const StringW & r) const;
		bool operator<(const StringW & r) const;
		bool operator>(const StringW & r) const;
		StringW operator+(const StringW & r) const;
		const StringW & operator=(const StringW & r) throw();
		const StringW & operator+=(const StringW & r) throw();
	private:
		void Initialize(const char* p, Charset charset = CHARSET_LOCALE) throw();
	private:
		wchar_t* buffer;
		int bufferSize;
	};

	inline StringW operator+(const wchar_t* l, const StringW & r)
	{
		return StringW(l) + r;
	}

	inline StringW operator+(const char* l, const StringW & r)
	{
		return StringW(l) + r;
	}

#ifdef UNICODE
	typedef StringW String;
#	ifndef TEXT
#		define TEXT(x) L##x
#	endif
#else
#	error Can not compile without UNICODE flag.
	//typedef StringA String
	//#	ifndef TEXT
	//#		define TEXT(x)
	//#	endif
#endif

	String BytesToMKBText(uint _bytes);
	String ByteToHex(uint8 _byte);
	String PointerToHex(void * _p);
}

#endif