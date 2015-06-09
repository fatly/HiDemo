#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "public.h"
using namespace e;

#pragma comment(lib, "libuilts.lib")

int main(int argc, char* argv[])
{
	int count = 10000;
	char** ptrs = (char**)malloc(count);

	for (int i = 0; i < count; i++)
	{
		ptrs[i] = (char*)malloc(1024);
	}

	for (int i = 0; i < count; i++)
	{
		free(ptrs[i]);
	}

	free(ptrs);

	system("pause");
	return 0;
}