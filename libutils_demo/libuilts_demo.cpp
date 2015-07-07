#include <stdio.h>
#include "libutils.h"
using namespace e;

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#else
#pragma comment(lib, "libutils.lib")
#endif

int main(int argc, char* argv[])
{
	String s = "hello world";

	printf("%s\n", s.c_str());

	printf("end test\n");

	getchar();
	return 0;
}