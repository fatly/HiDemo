#ifndef __CORE_LIST_H__
#define __CORE_LIST_H__

#include <new>
#include "Diagnosis.h"

namespace e
{
	template <typename T> class List
	{
		struct Node
		{
			Node * prev;
			Node * next;
			T data;
		};
		
		Node * head;
		Node * tail;
	public:
		class iterator
		{
			friend class List;
			Node * p;
		public:
			iterator(Node * node)
			{
				this->p = node;
			}
			
			bool operator==(const iterator & r) const
			{
				return this->p = r.p;
			}
			
			bool operator!=(const iterator & r) const
			{
				return this->p != r.p;
			}

			iterator & operator++(void)
			{
				p = p->next;
				return * this;			
			}

			iterator & operator++(int x)
			{
				p = p->next;
				return * this;				
			}

			T & operator*(void)
			{
				return p->data;
			}

			T * operator->(void)
			{
				return &p->data;
			}
		};
		
		iterator begin(void)
		{
			return iterator(head);
		}

		iterator end(void)
		{
			return iterator(0);
		}

		List(void)
			: head(0)
			, tail(0)
		{
			
		}
		
		virtual ~List(void)
		{
			clear();
		}

		void push_back(const T & data)
		{
			Node * p = (Node*)malloc(sizeof(Node));
			E_ASSERT(p);
			new(&p->data) T(data);

			p->prev = tail;
			p->next = 0;

			if (tail)
			{
				tail->next = p;
				tail = p;
			}
			else
			{
				tail = p;
				head = p;
			}
		}

		bool empty(void)
		{
			return head == 0;
		}

		int size(void)
		{
			int count = 0;
			Node * p = head;
			while (p)
			{
				count++;
				p = p->next;
			}
			return count;
		}

		iterator erase(const iterator & it)
		{
			Node * p = it.p;
			Node * next = p->next;
			
			if (p->prev)
			{
				p->prev->next = p->next;
			}
			else
			{
				head = p->next;
				if (head == 0)
				{
					tail = 0;
				}
			}
			
			if (p->next)
			{
				p->next->prev = p->prev;
			}
			else
			{
				tail = p->prev;
				if (tail == 0)
				{
					head = 0;
				}
			}
				
			p->data.~T();
			free(p);
			
			return iterator(next);
		}

		void clear(void)
		{
			Node * p;
			Node * p1;
			p = head;
			while (p)
			{
				p1 = p;
				p = p->next;
				free(p1);
			}
			head = 0;
			tail = 0;
		}

		T & front(void)
		{
			return head->data;
		}

		T & back(void)
		{
			return tail->data;
		}
	};
}

#endif
