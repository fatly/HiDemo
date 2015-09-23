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

typedef uint8 pixel_t;
//typedef float pixel_t;

void test_xbitmap(void)
{
	const char* fileName = "f:\\temp\\01.bmp";
	AutoPtr<Beautify> bfy = new Beautify();

	XBitmap<pixel_t>* src = new XBitmap<pixel_t>(fileName);
	XBitmap<pixel_t>* dst = new XBitmap<pixel_t>(*src);

	bfy->HighPass(dst->Ptr(0), src->Ptr(0), src->Width(), src->Height(), src->Channels());
	XBitmap<pixel_t>* gc = static_cast<XBitmap<pixel_t>*>(dst->Clone(0));
	gc->Save("f:\\temp\\02.bmp");

	for (int i = 0; i < 3; i++)
		bfy->CalcMatte(gc->Ptr(0), gc->Ptr(0), gc->Width(), gc->Height(), gc->Channels());
	gc->Save("f:\\temp\\03.bmp");

	bfy->AdjustSample(dst->Ptr(0), src->Ptr(0), src->Width(), src->Height(), src->Channels());
	dst->Save("f:\\temp\\04.bmp");

	bfy->Smooth(dst->Ptr(0), src->Ptr(0), gc->Ptr(0), src->Width(), src->Height(), src->Channels());
	dst->Save("f:\\temp\\05.bmp");

	delete src;
	delete dst;
	delete gc;
}



int main(int argc, char* argv[])
{
	//test_xbitmap();
	
	system("pause");
	return 0;
}