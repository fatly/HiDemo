#include <stdio.h>
#include "libutils.h"
using namespace e;

#pragma comment(lib, "libutils.lib")

int main(int argc, char* argv[])
{
	String s = "hello world";

	printf("%s\n", s.c_str());

	printf("end test\n");

	getchar();
	return 0;
}