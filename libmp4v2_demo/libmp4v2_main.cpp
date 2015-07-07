#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <string>
#include "mp4v2/mp4v2.h"
using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "libmp4v2d.lib")
#else
#pragma comment(lib, "libmp4v2.lib")
#endif

typedef unsigned char uchar;

inline int IsStartCode(const char* buffer)
{
	if (buffer[0] == 0x00 && buffer[1] == 0x00)
	{
		if (buffer[2] == 0x01) return 3;

		if (buffer[2] == 0x00 && buffer[3] == 0x01) return 4;
	}

	return 0;
}

inline int Find264Nal(string & s, int &pos, int &len)
{
	size_t size = s.length();

	for (size_t i = 0; i < size; i++)
	{
		int ret = IsStartCode(s.c_str() + i);
		if (ret > 0)
		{
			pos = i + ret;
			break;
		}
	}

	for (size_t i = pos + 1; i < size; i++)
	{
		if (IsStartCode(s.c_str() + i))
		{
			len = i - pos;
			return 1;
		}
	}

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		printf("usage : app [input] [width] [height] [output]\n");
		return 0;
	}

	const char* input = argv[1];
	const char* output = argv[4];
	const int width = atoi(argv[2]);
	const int height = atoi(argv[3]);

	FILE *fin = 0;
	fopen_s(&fin, input, "rb");

	if (fin == 0)
	{
		printf("open h264 file failed\n");
		return 0;
	}

	MP4FileHandle hFile = MP4Create(output);

	if ( hFile==0)
	{
		printf("open mp4 file failed\n");
		return 0;
	}

	const int profileLevel = 1;
	const int timeScale = 90000;
	const int frameRate = 25;
	MP4TrackId videoId = 0;

	MP4SetTimeScale(hFile, timeScale);
	MP4SetVideoProfileLevel(hFile, profileLevel);

	int frameCount = 0;
	const int bufferSize = width * height * 4;
	char* buffer = (char*)malloc(bufferSize);

	string s;
	int pos = 0, len = 0;
	const uchar* p = 0;
	while (!feof(fin))
	{
		int bytes = fread(buffer, 1, bufferSize, fin);
		s.append(buffer, bytes);

		while (Find264Nal(s, pos, len))
		{
			p = (uchar*)(s.c_str() + pos);

			int type = *p & 0x1f;

			switch (type)
			{
			case 6: //sei

				break;
			case 7: //sps
				videoId = MP4AddH264VideoTrack(hFile
					, timeScale
					, timeScale / frameRate
					, width
					, height
					, p[1]
					, p[2]
					, p[3]
					, 3);
				MP4AddH264SequenceParameterSet(hFile, videoId, p, len);
				break;
			case 8: //pps
				MP4AddH264PictureParameterSet(hFile, videoId, p, len);
				break;
			case 1:
			case 5:
				{
					uchar* data = (uchar*)malloc(len + 4);
					data[0] = (len & 0xff000000) >> 24;
					data[1] = (len & 0x00ff0000) >> 16;
					data[2] = (len & 0x0000ff00) >> 8;
					data[3] = (len & 0x000000ff);
					memcpy(data + 4, p, len);
					MP4WriteSample(hFile, videoId, data, len + 4, MP4_INVALID_DURATION, 0, 1);
					frameCount++;
					free(data);
				}
				break;
			default:
				//assert(0);
				break;
			}

			s = s.substr(pos + len);
		}
	}

	if (!s.empty())
	{
		int ret = IsStartCode(s.c_str());

		if (ret > 0)
		{
			p = (uchar*)(s.c_str() + ret);
			len = s.length() - ret;
			uchar* data = (uchar*)malloc(len + 4);
			data[0] = (len & 0xff000000) >> 24;
			data[1] = (len & 0x00ff0000) >> 16;
			data[2] = (len & 0x0000ff00) >> 8;
			data[3] = (len & 0x000000ff);
			memcpy(data + 4, p, len);
			MP4WriteSample(hFile, videoId, data, len + 4, MP4_INVALID_DURATION, 0, 1);
			frameCount++;
			free(data);
		}
	}

	free(buffer);

	MP4Close(hFile);
	fclose(fin);

	printf("read video frame = %d\n", frameCount);

	system("pause");
	return 0;
}