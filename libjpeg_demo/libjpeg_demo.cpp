#include "private.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <conio.h>
#include "Decoder.h"
#include "Encoder.h"

using namespace e;

int DecodeImage(int argc, char* argv[])
{
	const char* infile = argv[1];
	const char* outfile = argv[2];

	FILE* fp = 0;
	fopen_s(&fp, infile, "rb");
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
			e::Bitmap bm(x, y, 24);

			for (int i = 0; i < y; i++)
			{
				uchar* p = bm.Get(0, i);
				memcpy(p, data + i * x * 3, sizeof(uchar)* x * 3);
			}

			bm.SwapChannel(0, 2);
			bm.Save(outfile);

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

int EncodeImage(int argc, char* argv[])
{
	const char* infile = argv[1];
	const char* outfile = argv[2];
	const int quality = atoi(argv[3]);

	e::Bitmap bm(infile);
	if (!bm.IsValid())
	{
		printf("input file invalid : %s\n", infile);
		return 0;
	}

	uchar* tmp = (uchar*)malloc(bm.Width() * bm.Height() * 3);
	assert(tmp);

	do 
	{
		Encoder encoder;
		uint32 size = encoder.EncodeImage(tmp, bm.bits, bm.Width(), bm.Height(), bm.biBitCount, quality);
		printf("encode image width = %d, height = %d, size = %d\n", bm.Width(), bm.Height(), size);

		FILE* fp = 0;
		fopen_s(&fp, outfile, "wb");

		if (fp == 0)
		{
			printf("open out file failed : %s\n", outfile);
			break;
		}

		fwrite(tmp, 1, size, fp);

		fclose(fp);

	} while (0);

	if (tmp) free(tmp);

	return 0;
}

int main(int argc, char* argv[])
{
// 	char c = _getch();
// 
// 	if (c == 'e')
// 	{
// 		EncodeImage();
// 	}
// 	else
// 	{
// 		DecodeImage();
// 	}

	if (1)
		EncodeImage(argc, argv);
	else
		DecodeImage(argc, argv);

//	system("pause");
	return 0;
}
