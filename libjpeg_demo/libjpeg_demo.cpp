#include "private.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "decoder.h"

using namespace e;

int main(int argc, char* argv[])
{
	char* file = "e:\\images\\texture\\tex0.jpg";
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
				memcpy(p, data + i * x * 3, sizeof(uchar) * x * 3);
			}

			bm.SwapChannel(0, 2);
			bm.Save("f:\\res0.bmp");

			free(data);
			printf("decode result: x = %d, y = %d, comp = %d, err=%s\n", 
				x, y, comp,decoder.lastError?decoder.lastError:"no");
		}
		else
		{
			printf("decode failed : %s\n", decoder.lastError);
		}
		
	} while (0);

	free(buffer);
	fclose(fp);

	system("pause");
	return 0;
}
