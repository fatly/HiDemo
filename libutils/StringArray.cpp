#include "private.h"
#include "StringArray.h"

namespace e
{
	StringArray Split(const String & src, const Char* ch, int from, int len)
	{
		StringArray ret;
		int length = src.Length();

		if (len != -1 && from + len < length)
		{
			length = from + len;
		}

		int pos = from;

		while (pos < length)
		{
			int n = src.FindChar(ch, pos);
			if (n == -1)
			{
				StringW s = src.SubStr(pos, -1);
				if (!s.IsEmpty())
				{
					ret.push_back(s);
					break;
				}
			}
			else
			{
				StringW s = src.SubStr(pos, n - pos);
				if (!s.IsEmpty())
				{
					ret.push_back(s);
				}

				pos = n + 1;
			}
		}

		return ret;
	}
}