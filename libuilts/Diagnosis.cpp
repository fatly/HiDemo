#include "private.h"
#include "Diagnosis.h"
#include "String.h"
#include "Thread.h"
#include <stdlib.h>

namespace e
{
	void ReportError(const String & msg)
	{
		WriteLog(TEXT("error"), msg);

		MessageBox(NULL, msg.c_str(), TEXT("Assert"), MB_OK | MB_ICONERROR);
	}

	void OnAssertFailed(const String & msg, const String & file, int line)
	{
		String text = file + TEXT("(") + String(line) + TEXT("): ") + msg;

		OutputDebugString(text.c_str());
		OutputDebugString(TEXT("\n"));

#if (defined(WIN32) || defined(WIN64)) && defined(_DEBUG)
		if (!IsDebuggerPresent())
		{
			ReportError(text);
		}
#endif
	}

	void DebugWrite(const String & msg)
	{
		OutputDebugString(msg.c_str());
	}

	void DebugWrite(const String & msg, const String & file, const int line)
	{
		String text = file + TEXT(":") + String(line) + TEXT(" -> ") + msg;

		OutputDebugString(text.c_str());
	}

	void DebugWriteLine(const String & msg)
	{
		OutputDebugString(msg.c_str());
		OutputDebugString(TEXT("\n"));
	}

	void DebugWriteLine(const String & msg, const String & file, const int line)
	{
		String text = file + TEXT(":") + String(line) + TEXT(" -> ") + msg;

		OutputDebugString(text.c_str());
	}

	Exception::Exception(const String & what, const char* file /* = 0 */, int line /* = -1 */) :
		_what(what),
		_file(file),
		_line(line)
	{

	}

	Exception::~Exception(void)
	{

	}

	String Exception::GetText(void)
	{
		return String(_file) + TEXT("(") + String(_line) + TEXT(") : ") + _what;
	}

	String GetOSErrorText(void)
	{
		LPVOID* pString = 0;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
			, 0
			, GetLastError()
			, LANG_USER_DEFAULT
			, LPTSTR(&pString)
			, sizeof(pString)
			, 0);
		String ret((const Char*)pString);
		LocalFree(pString);
		return ret;
	}

	bool WriteLog(const String & category, const String & text)
	{

		return true;
	}
}
	//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG

#undef malloc
#undef realloc
#undef calloc
#undef free

void* operator new(size_t sz)
{
	return e::debug_malloc(sz, 0, 0, e::MAT_NEW);
}

void* operator new(size_t sz, const char* file, int line)
{
	return e::debug_malloc(sz, file, line, e::MAT_NEW);
}

void* operator new[](size_t sz)
{
	return e::debug_malloc(sz, 0, 0, e::MAT_NEWARRAY);
}

void* operator new[](size_t sz, const char* file, int line)
{
	return e::debug_malloc(sz, file, line, e::MAT_NEWARRAY);
}

void operator delete(void* p)
{
	return e::debug_free(p, e::MAT_NEW);
}

void operator delete[](void* p)
{
	return e::debug_free(p, e::MAT_NEWARRAY);
}

namespace e
{
#define DEBUG_MEMORY_HEADER_COOKIE 0xABCDABCD

	struct DebugMemoryHeader
	{
		int order;
		const char* file;
		int line;
		MemoryAllocType type;
		size_t size;
		uint32 cookie0;

		void Dump(bool extrainfo);
	};

	typedef DebugMemoryHeader* DMHPtr;

#define get_debug_memory_body(p) (((char*)(p)) + sizeof(DebugMemoryHeader))

	void DebugMemoryHeader::Dump(bool extrainfo)
	{
		if (extrainfo)
		{
			DebugWriteLine(TEXT("[library] : details of this memory leaks"));
			DebugWriteLine(TEXT("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"));
		}

		String t, t1, t2;
		size_t sz1 = 32 < size ? 32 : size;
		char* body = get_debug_memory_body(this);
		t = PointerToHex(body);

		for (size_t i = 0; i < sz1; i++)
		{
			if (body[i] >= 32)
			{
				t1.Append((Char)body[i]);
			}
			else
			{
				t1.Append(TEXT("."));
			}
		}

		for (size_t i = 0; i < sz1; i++)
		{
			t2 += ByteToHex(body[i]);
		}

		if (file)
		{
			DebugWriteLine(String(file) + TEXT("(") + String(line) + TEXT(")"));
		}
		else
		{
			DebugWriteLine(TEXT("unknown allocated location"));
		}

		DebugWriteLine(TEXT(" { ") + String(order) + TEXT(" }") + TEXT("(") + String(line) + TEXT(" bytes) \"") + t1 + String("\""));
		DebugWriteLine(TEXT(" ") + t2);

		if (extrainfo)
		{
			DebugWriteLine(TEXT("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"));
		}
	}

	class DMHSet
	{
	public:
		struct Node{
			Node* parent;
			Node* left;
			Node* right;
			int priority;
			DMHPtr data;
		};

		DMHSet(void) : root(0)
		{

		}
		~DMHSet(void)
		{
			clear();
		}

		Node* begin(void)
		{
			Node* p = root;
			if (p)
			{
				while (p->left)
				{
					p = p->left;
				}
			}
			return p;
		}

		bool empty(void) const
		{
			return (root == 0);
		}

		Node* next(Node* node)
		{
			Node* p = node;
			ASSERT(p);

			if (p->right)
			{
				p = p->right;
				while (p->left)
				{
					p = p->left;
				}
			}
			else if (p->parent)
			{
				Node* p1 = 0;
				do 
				{
					p1 = p;
					p = p->parent;
				} while (p && p->right==p1);
			}
			else
			{
				p = 0;
			}

			return p;
		}

		void insert(DMHPtr & data)
		{
			Node* node = (Node*)malloc(sizeof(Node));
			memset(node, 0, sizeof(Node));
			node->priority = Random();
			node->data = data;

			if (root == 0)
			{
				root = node;
				return;
			}

			Node* p = root;

			while (true)
			{
				if (node->data < p->data)
				{
					if (p->left)
					{
						p = p->left;
					}
					else
					{
						node->parent = p;
						p->left = node;
						break;
					}
				}
				else
				{
					if (p->right)
					{
						p = p->right;
					}
					else
					{
						node->parent = p;
						p->right = node;
						break;
					}
				}
			}

			adjust(node);
		}

		void clear(void)
		{
			for (auto it = begin(); it != 0; it = erase(it));
		}

		Node* erase(Node* node)
		{
			Node* p = node;
			while (p->left && p->right)
			{
				Node* pp = p->parent;
				Node* p1 = p->left;
				Node* p2 = p->right;
				Node* &pm = (pp) ? (pp->left == p ? pp->left : pp->right) : root;

				if (p->left->priority < p->right->priority)
				{
					pm = p2;
					p2->parent = pp;
					p->right = p2->left;

					if (p->right)
					{
						p->right->parent = p;
					}

					p2->left = p;
					p->parent = p2;
				}
				else
				{
					pm = p1;
					p1->parent = pp;
					p->left = p1->right;

					if (p->left)
					{
						p->left->parent = p;
					}

					p1->right = p;
					p->parent = p1;
				}
			}

			{
				Node* pp = p->parent;
				Node* &pm = (pp) ? (pp->left == p ? pp->left : pp->right) : root;

				if (p->right)
				{
					pm = p->right;
					p->right->parent = pp;
					free(p);
					return pm;
				}
				else
				{
					pm = p->left;
					if (p->left)
					{
						p->left->parent = pp;
					}

					free(p);
					return pp;
				}
			}
		}

		void erase(DMHPtr & data)
		{
			auto it = find(data);
			if (it != 0)
			{
				erase(it);
			}
		}

		Node* find(DMHPtr & data)
		{
			Node* p = root;

			while (p)
			{
				if (data == p->data)
				{
					return p;
				}

				if (data < p->data)
				{
					p = p->left;
				}
				else
				{
					p = p->right;
				}
			}

			return 0;
		}
	private:
		Node* root;

		static int Random(void)
		{
			static uint randomSeed = 1;
			randomSeed = (randomSeed * 1103515245 + 12345) % 32678;
			return randomSeed;
		}

		void adjust(Node* node)
		{
			Node* p = node;
			while (p->parent)
			{
				if (p->parent->priority < p->priority)
				{
					//×ó×ÓÊ÷£¬ÓÒÐý
					if (p->parent->left == p)
					{
						p->parent->left = p->right;

						if (p->right)
						{
							p->right->parent = p->parent;
						}
						p->right = p->parent;
						p->parent= p->right->parent;
						p->right->parent = p;

						if (p->parent)
						{
							if (p->parent->left == p->right)
							{
								p->parent->left = p;
							}
							else
							{
								p->parent->right = p;
							}
						}
						else
						{
							root = p;
							break;
						}
					}
					else
					{
						//ÓÒ×ÓÊ÷£¬×óÐý
						p->parent->right = p->left;

						if (p->left)
						{
							p->left->parent = p->parent;
						}

						p->left = p->parent;
						p->parent = p->left->parent;
						p->left->parent = p;

						if (p->parent)
						{
							if (p->parent->right == p->left)
							{
								p->parent->right = p;
							}
							else
							{
								p->parent->left = p;
							}
						}
						else
						{
							root = p;
							break;
						}
					}
				}
				else
				{
					p = p->parent;
				}
			}
		}
	};

	static Mutex & debug_memory_mutex(void)
	{
		static Mutex g_memory_mutex;
		return g_memory_mutex;
	}

	static int g_debug_memory_break_at_alloc = -1;
	static int g_debug_memory_block_order = 0;
	static DMHSet* g_debug_memory_heap = 0;

	void set_debug_memory_break_at_alloc(int order)
	{
		ScopeLock lock(debug_memory_mutex());
		g_debug_memory_break_at_alloc = order;
	}

	static String get_dmat_allocated_by(MemoryAllocType type)
	{
		switch (type)
		{
		case MAT_C:
			return TEXT("malloc,calloc or recalloc");
		case MAT_NEW:
			return TEXT("\" operator new \"");
		case MAT_NEWARRAY:
			return TEXT("\" operator new[] \"");
		default:
			return TEXT("\" unknown memory alloc \"");
		}
	}

	static String get_dmat_delete_by(MemoryAllocType type)
	{
		switch (type)
		{
		case MAT_C:
			return TEXT("free");
		case MAT_NEW:
			return TEXT("\" delete \"");
		case MAT_NEWARRAY:
			return TEXT("\" delete[] \"");
		default:
			return TEXT("\" unknown memory delete \"");
		}
	}

	static void debug_memory_heap_close(void);

	static void debug_memory_heap_open(void)
	{
		ScopeLock lock(debug_memory_mutex());
		if (g_debug_memory_heap) return;

		g_debug_memory_heap = (DMHSet*)malloc(sizeof(DMHSet));
		new (g_debug_memory_heap)DMHSet();

		DebugWrite(TEXT("[library] : debug memory pool initialized\n"));
		atexit(&debug_memory_heap_close);
	}

	static void debug_memory_heap_close(void)
	{
		ASSERT(g_debug_memory_heap != 0);

		if (g_debug_memory_heap->empty())
		{
			DebugWriteLine(TEXT("[library] : no memory leaks"));
		}
		else
		{
			DebugWriteLine(TEXT("[library] : dump memory leaks"));
			auto it = g_debug_memory_heap->begin();
			for (; it != 0; it = g_debug_memory_heap->erase(it))
			{
				DMHPtr p = it->data;
				p->Dump(false);
				free(p);
			}

			DebugWriteLine(TEXT("[library] : end of dump memory leaks"));
		}

		g_debug_memory_heap->~DMHSet();
		free(g_debug_memory_heap);
		g_debug_memory_heap = 0;
	}

	static void debug_memory_heap_insert(DMHPtr p)
	{
		ScopeLock lock(debug_memory_mutex());

		g_debug_memory_block_order++;
		p->order = g_debug_memory_block_order;

		g_debug_memory_heap->insert(p);

		if (p->order == g_debug_memory_break_at_alloc)
		{
			DEBUG_BREAK;
		}
	}

	static DMHPtr debug_memory_heap_checkout(void* p)
	{
		ScopeLock lock(debug_memory_mutex());

		if (p == 0) return 0;

#ifdef _MSC_VER
		ASSERT(p != (void*)0xcdcdcdcd);
		ASSERT(p != (void*)0xfeeefeee);
#endif

		DMHPtr h = (DMHPtr)(((char*)p) - sizeof(DebugMemoryHeader));

		ASSERT((h)->cookie0 == DEBUG_MEMORY_HEADER_COOKIE);
		ASSERT(*((uint32*)((char*)(h)+sizeof(DebugMemoryHeader) + (h)->size)) == DEBUG_MEMORY_HEADER_COOKIE);

		DMHSet::Node* node = 0;
		if (g_debug_memory_heap && (node = g_debug_memory_heap->find(h)) != 0)
		{
			g_debug_memory_heap->erase(node);
			return h;
		}
		else
		{
			ASSERT(0);
			return 0;
		}
	}

	void* debug_malloc(size_t sz, const char* file, const int line, MemoryAllocType type)
	{
		if (g_debug_memory_heap == 0)
		{
			debug_memory_heap_open();
		}

		size_t allocBytes = (sz == 0) ? 1 : sz;
		size_t totalBytes = allocBytes + sizeof(DebugMemoryHeader) + sizeof(uint32);
		char* p = (char*)malloc(totalBytes);
		DebugMemoryHeader* header = (DebugMemoryHeader*)p;
		header->order = 0;
		header->file = file;
		header->line = line;
		header->size = sz;
		header->type = type;
		header->cookie0 = DEBUG_MEMORY_HEADER_COOKIE;
		*((uint32*)(p + sizeof(DebugMemoryHeader) + header->size)) = DEBUG_MEMORY_HEADER_COOKIE;

		g_debug_memory_heap->insert(header);

		return p + sizeof(DebugMemoryHeader);
	}

	void* debug_realloc(void* p, size_t sz, const char* file, const int line)
	{
		if (g_debug_memory_heap == 0)
		{
			debug_memory_heap_open();
		}

		if (p == 0)
		{
			return debug_malloc(sz, file, line, MAT_C);
		}

		DebugMemoryHeader* header = debug_memory_heap_checkout(p);
		if (header == 0)
		{
			OnAssertFailed(TEXT("[library] debug_realloc : re-alloc a block which was not allocated by by debug memory function."), file, line);
			DEBUG_BREAK;
			return realloc(p, sz);
		}

		if (header->type != MAT_C)
		{
			header->Dump(true);
			OnAssertFailed(TEXT("[library] debug_realloc : re-alloc a block which was allocate by ") + get_dmat_allocated_by(header->type), file, line);
			DEBUG_BREAK;
		}

		if (sz == 0)
		{
			return debug_malloc(sz, file, line, MAT_C);
		}

		void* ret = debug_malloc(sz, file, line, MAT_C);
		sz = header->size < sz ? header->size : sz;
		memcpy(ret, p, sz);
		free(header);
		return ret;
	}

	void* debug_calloc(size_t num, size_t sz, const char* file, const int line)
	{
		void* p = debug_malloc(num * sz, file, line, MAT_C);
		memset(p, 0, num * sz);
		return p;
	}

	void debug_free(void* p, MemoryAllocType type)
	{
		if (g_debug_memory_heap == 0)
		{
			debug_memory_heap_open();
		}

		if (type != MAT_C && type != MAT_NEW && type != MAT_NEWARRAY)
		{
			OnAssertFailed(TEXT("[library] debug_free() : Invalid params, type = ") + String((int)type), __FILE__, __LINE__);
			DEBUG_BREAK;
		}

		if (p == 0)
		{
			return;
		}

		DebugMemoryHeader* header = debug_memory_heap_checkout(p);
		if (header == 0)
		{
			OnAssertFailed(TEXT("[library] debug_free():")
				+ get_dmat_delete_by(type)
				+ TEXT("a block which was allocated by ")
				+ get_dmat_allocated_by(header->type),
				__FILE__, __LINE__);
			header->Dump(true);
			DEBUG_BREAK;
		}

		free(header);
	}

	void debug_change_location(void* p, const char* file, const int line)
	{
		ASSERT(g_debug_memory_heap != 0);
		ASSERT(p != 0);
		DebugMemoryHeader* header = debug_memory_heap_checkout(p);
		if (header == 0)
		{
			ASSERT(0);
		}
		else
		{
			header->file = file;
			header->line = line;
			debug_memory_heap_insert(header);
		}
	}

#endif //end ifdef _DEBUG

}
