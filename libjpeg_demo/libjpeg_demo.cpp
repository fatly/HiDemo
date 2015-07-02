#include "private.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <conio.h>
#include "decoder.h"
#include "Encoder.h"

using namespace e;

int DecodeImage(void)
{
	//char* file = "e:\\images\\texture\\tex2.jpg";
	char* file = "f:\\res.jpg";

	FILE* fp = 0;
	fopen_s(&fp, file, "rb");
	if (fp == 0)
	{
		printf("open file fail\n");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	uchar* buffer = (uchar*)malloc(size * sizeof(uchar));
	assert(buffer);

	int bytes = fread(buffer, 1, size, fp);

	do
	{
		Decoder decoder;
		int x = 0, y = 0, comp = 0;
		uchar* data = decoder.LoadJpegImage(buffer, size, &x, &y, &comp, 0);

		if (data)
		{
			Bitmap bm(x, y, 24);

			for (int i = 0; i < y; i++)
			{
				uchar* p = bm.Get(0, i);
				memcpy(p, data + i * x * 3, sizeof(uchar)* x * 3);
			}

			bm.SwapChannel(0, 2);
			bm.Save("f:\\res.bmp");

			free(data);
			printf("decode result: x = %d, y = %d, comp = %d, err=%s\n",
				x, y, comp, decoder.lastError ? decoder.lastError : "no");
		}
		else
		{
			printf("decode failed : %s\n", decoder.lastError);
		}

	} while (0);

	free(buffer);
	fclose(fp);

	return 0;
}

int EncodeImage(void)
{
	const char* file = "f:\\a.bmp";

	e::Bitmap bm(file);

	uchar* tmp = (uchar*)malloc(bm.Width() * bm.Height() * 3);

	do 
	{
		Encoder encoder;
		uint32 size = encoder.EncodeImage(tmp, bm.bits, bm.Width(), bm.Height(), bm.biBitCount, 85);
		printf("encode image width = %d, height = %d, size = %d\n", bm.Width(), bm.Height(), size);

		const char* outfile = "f:\\res.jpg";
		FILE* fp = 0;
		fopen_s(&fp, outfile, "w");
		if (fp == 0) break;

		fwrite(tmp, 1, size, fp);

		fclose(fp);

	} while (0);

	free(tmp);

	return 0;
}

int main(int argc, char* argv[])
{
	char c = _getch();

	if (c == 'e')
	{
		EncodeImage();
	}
	else
	{
		DecodeImage();
	}

// 	if (1)
// 		EncodeImage();
// 	else
// 		DecodeImage();

	system("pause");
	return 0;
}
