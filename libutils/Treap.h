#ifndef __E_TREAP_H__
#define __E_TREAP_H__

#include "Defines.h"

#undef malloc
#undef free

namespace e
{
	template<class T> class Treap
	{
	public:
		struct Node{
			Node* parent;
			Node* left;
			Node* right;
			int priority;
			T data;
		};

		Treap(void) : root(0)
		{

		}
		~Treap(void)
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
				} while (p && p->right == p1);
			}
			else
			{
				p = 0;
			}

			return p;
		}

		void insert(T & data)
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

		void erase(T & data)
		{
			auto it = find(data);
			if (it != 0)
			{
				erase(it);
			}
		}

		Node* find(T & data)
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
						p->parent = p->right->parent;
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
}

#endif