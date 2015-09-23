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
 	const char* fileName = "f:\\temp\\01.bmp";
// 	AutoPtr<Bitmap> src = new Bitmap(fileName);
// 	AutoPtr<Bitmap> dst = new Bitmap(*src);
// 	AutoPtr<Beautify> bfy = new Beautify();
// 
// 	bfy->HighPass(dst->bits
// 		, src->bits
// 		, src->Width()
// 		, src->Height()
// 		, src->biBitCount);
// 
// 	AutoPtr<Bitmap> gc = dst->Clone(0);
// 	gc->Save("f:\\temp\\02.bmp");
// 
// 	for (int i = 0; i < 3; i++)
// 		bfy->CalcMatte(gc->bits, gc->bits, gc->Width(), gc->Height(), gc->biBitCount);
// 	gc->Save("f:\\temp\\03.bmp");
// 
// 	bfy->AdjustSample(dst->bits, src->bits, src->Width(), src->Height(), src->biBitCount);
// 	dst->Save("f:\\temp\\04.bmp");
// 
// 	bfy->Smooth(dst->bits, src->bits, gc->bits, src->Width(), src->Height(), src->biBitCount);
// 	dst->Save("f:\\temp\\05.bmp");

	XBitmap<float> im(fileName);
	printf("w=%d,h=%d,c=%d\n", im.Width(), im.Height(), im.Channels());
	Convert::Normalize(&im);
	for (int i = 0; i < 30; i++)
	{
		float* p = im.Ptr(0, i);
		for (int j = 0; j < 30; j++)
		{
			printf("%f ", p[j]);
		}

		printf("\n");
	}

	im.Save("f:\\temp\\tmp.bmp");

	printf("\n");
	system("pause");
	return 0;
}