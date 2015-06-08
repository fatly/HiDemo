#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "x265.h"
#pragma comment(lib, "libx265.lib")

#ifdef __cplusplus
};
#endif


int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		printf("usage: app [input file] [width] [height] [output file]\n");
		return 0;
	}

	const char* input = argv[1];
	const char* output = argv[4];
	const int width = atoi(argv[2]);
	const int height = atoi(argv[3]);
	const int fps = 25;

	FILE *fin = 0, *fout = 0;
	fopen_s(&fin, input, "rb");
	fopen_s(&fout, output, "wb");

	if (fin == NULL || fout == NULL)
	{
		printf("open file failed\n");
		return 0;
	}

	int bufferSize = width * height * 3 / 2;
	char* buffer = (char*)malloc(bufferSize);
	assert(buffer);

	x265_param param;
	//x265_param_default(&param);
	x265_param_default_preset(&param, x265_preset_names[4], x265_tune_names[2]);
	//设置x265参数
	param.bRepeatHeaders = 0;
	param.sourceWidth = width;
	param.sourceHeight = height;
	param.fpsNum = fps;
	param.fpsDenom = 1;
	param.levelIdc = 40;
	param.keyframeMin = fps;
	param.keyframeMax = fps * 2;
	param.bIntraInBFrames = 0;
	param.rc.cuTree = 0;
	param.rc.bitrate = 500;//码率会影响Profile Level的
	param.rc.rateControlMode = X265_RC_ABR;
	param.rc.vbvBufferSize = param.rc.bitrate;
	param.rc.vbvMaxBitrate = param.rc.bitrate;
	param.rc.vbvBufferInit = 0.7;
	param.rc.qCompress = 0.5f;
	param.rc.rfConstant = 8.5f;
	param.rc.rfConstantMax = 20;
	param.rc.rateTolerance = 0.1;
	param.rc.aqStrength = 0.5f;
	param.rc.aqMode = X265_AQ_AUTO_VARIANCE;
	param.logLevel = X265_LOG_INFO;

	x265_encoder* h265 = x265_encoder_open(&param);
	if (h265 == NULL)
	{
		printf("open x265 failed\n");
		return 0;
	}

	x265_picture picture;
	x265_picture_init(&param, &picture);
	picture.planes[0] = buffer;
	picture.planes[1] = buffer + width * height;
	picture.planes[2] = buffer + width * height * 5 / 4;
	picture.stride[0] = width;
	picture.stride[1] = width / 2;
	picture.stride[2] = width / 2;
	picture.colorSpace = X265_CSP_I420;
	picture.pts = 0;

	int result = 0;
	int frameCount = 0;
	x265_nal* p_nal = NULL;
	uint32_t i_nal = 0;
	//encoder header
	x265_encoder_headers(h265, &p_nal, &i_nal);
	for (uint32_t i = 0; i < i_nal; i++)
	{
		printf("header nal type -> %d\n", p_nal[i].type);
		fwrite(p_nal[i].payload, 1, p_nal[i].sizeBytes, fout);
	}

	while (!feof(fin))
	{
		int bytes = fread(buffer, 1, bufferSize, fin);

		if (bytes == bufferSize)
		{
			if (frameCount++ % 8 == 0)
			{
				picture.sliceType = X265_TYPE_IDR;
			}
			else
			{
				picture.sliceType = X265_TYPE_AUTO;
			}

			result = x265_encoder_encode(h265, &p_nal, &i_nal, &picture, NULL);
			for (uint32_t i = 0; i < i_nal; i++)
			{
				printf("frame nal type -> %d\n", p_nal[i].type);
				fwrite(p_nal[i].payload, 1, p_nal[i].sizeBytes, fout);
			}
		}

		picture.pts++;
	}

	while (result = x265_encoder_encode(h265, &p_nal, &i_nal, NULL, NULL))
	{
		for (uint32_t i = 0; i < i_nal; i++)
		{
			printf("frame nal type -> %d\n", p_nal[i].type);
			fwrite(p_nal[i].payload, 1, p_nal[i].sizeBytes, fout);
		}
	}

	x265_encoder_close(h265);

	fclose(fin);
	fclose(fout);
	free(buffer);

	system("pause");
	return 0;
}