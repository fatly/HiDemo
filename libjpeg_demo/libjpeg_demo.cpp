#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "decoder.h"
#include "libutils.h"

using namespace e;

#pragma comment(lib, "libutils.lib")

extern "C"
{
	extern uchar *stbi_load_from_memory(uchar const *buffer, int len, int *x, int *y, \
		int *comp, int req_comp);
	extern void stbi_image_free(void *retval_from_stbi_load);
};

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

			bm.Save("f:\\res0.bmp");

			free(data);
			printf("decode result: x = %d, y = %d, comp = %d\n", x, y, comp);
		}
		else
		{
			printf("decode failed : %s\n", decoder.lastError);
		}
		
	} while (0);

// 	do 
// 	{
// 		int x = 0, y = 0, comp = 0, req_comp = 0;
// 		uchar* data = stbi_load_from_memory(buffer, size, &x, &y, &comp, req_comp);
// 
// 		if (data)
// 		{
// 			Bitmap bm(x, y, 24);
// 
// 			for (int i = 0; i < y; i++)
// 			{
// 				uchar* p = bm.Get(0, i);
// 				memcpy(p, data + i * x * 3, sizeof(uchar)* x * 3);
// 			}
// 
// 			bm.Save("f:\\res1.bmp");
// 
// 			free(data);
// 			printf("decode result: x = %d, y = %d, comp = %d\n", x, y, comp);
// 		}
// 		else
// 		{
// 			printf("decode failed : %s\n", "stbi");
// 		}
// 
// 	} while (0);

	free(buffer);
	fclose(fp);

	system("pause");
	return 0;
}
