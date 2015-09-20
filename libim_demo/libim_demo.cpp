#include <stdio.h>
#include <stdlib.h>
#include "Bitmap.h"
#include "Beautify.h"
#include "AutoPtr.h"
using namespace e;

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#else 
#pragma comment(lib, "libutils.lib")
#endif

int main(int argc, char* argv[])
{
	const char* fileName = "f:\\01.bmp";
	AutoPtr<Bitmap> src = new Bitmap(fileName);
	AutoPtr<Bitmap> dst = new Bitmap(*src);
	AutoPtr<Beautify> p = new Beautify();
	p->HighContrast(dst->bits, src->bits, src->Width(), src->Height(), src->biBitCount);

//	system("pause");
	return 0;
}