#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Treap.h"
#include <time.h>


class A
{
public:
	A(int* ap)
	{
		p = ap;
		printf("p = %x\n", p);
	}

protected:
	int* p;
};

class B : public A
{
public:
	B(void)
		: A(s)
	{
		s = new int;
	}

	void Print(void)
	{
		printf("s = %x\n", s);
	}
private:
	int* s;
};


int main(int argc, char* argv[])
{
// 	srand((unsigned)time(0));
// 
// 	Treap<int> treap;
// 
// 	int count = 100000;
// 	int* data = new int[count];
// 	for (int i = 0; i < count; i++)
// 	{
// 		int v = rand();
// 		treap.insert(v);
// 		data[i] = v;
// 	}
// 
// 	for (int i = 0; i < count; i++)
// 	{
// 		if (!treap.find(data[i]))
// 		{
// 			printf("cannot find data : %d\n", data[i]);
// 		}
// 	}
// 
// 	for (int i = 0; i < count; i++)
// 	{
// 		treap.erase(data[i]);
// 	}
// 
// 	if (!treap.empty())
// 	{
// 		printf("treap error\n");
// 	}
// 	else
// 	{
// 		printf("treap healthy\n");
// 	}
// 
// 	delete[] data;

	B b;
	b.Print();

	system("pause");
	return 0;
}