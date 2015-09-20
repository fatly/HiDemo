#include <stdio.h>
#include <stdlib.h>
#include "libutils.h"
#include "libim.h"
using namespace e;

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#pragma comment(lib, "libimd.lib")
#else 
#pragma comment(lib, "libutils.lib")
#pragma comment(lib, "libim.lib")
#endif

int main(int argc, char* argv[])
{
	const char* fileName = "f:\\01.bmp";
	AutoPtr<Bitmap> src = new Bitmap(fileName);
	AutoPtr<Bitmap> dst = new Bitmap(*src);
	AutoPtr<Beautify> bfy = new Beautify();

	bfy->HighContrast(dst->bits
		, src->bits
		, src->Width()
		, src->Height()
		, src->biBitCount);

//	system("pause");
	return 0;
}