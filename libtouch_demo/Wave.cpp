#include "Wave.h"
#include <stdlib.h>
#include <stdio.h>
#include <exception>

PCM::PCM(void)
{
	header.extraBytes = 0;
	data = 0;
	size = 0;
}

PCM::~PCM(void)
{
	if (data != 0)
	{
		free(data);
		data = 0;		
	}

	size = 0;
}


Wave::Wave()
{

}

Wave::~Wave()
{

}

inline bool read(void * buffer, size_t len, FILE * fp) throw()
{
	return fread(buffer, 1, len, fp) == len;
}

inline bool write(void * buffer, size_t len, FILE * fp) throw()
{
	return fwrite(buffer, 1, len, fp) == len;
}

bool Wave::Load(PCM & pcm, const char* filename)
{
	if (filename == 0) return false;

	FILE * fp = 0;
	if (fopen_s(&fp, filename, "rb") != 0 || fp == 0)
	{
		return false;
	}

	bool result = false;

	do
	{
		WAVE_HEADER wf;
		if (!read(&wf.magic[0], sizeof(wf.magic), fp))
		{
			break;
		}
		if (wf.magic[0] != 'R' || wf.magic[1] != 'I' || wf.magic[2] != 'F' || wf.magic[3] != 'F')
		{
			break;
		}
		if (!read(&wf.length, sizeof(wf.length), fp))
		{
			break;
		}
		if (!read(&wf.type[0], sizeof(wf.type), fp))
		{
			break;
		}
		if (wf.type[0] != 'W' || wf.type[1] != 'A' || wf.type[2] != 'V' || wf.type[3] != 'E')
		{
			break;
		}

		WAVE_CHUNK_HEADER chunk;
		while (read(&chunk.type[0], sizeof(chunk.type), fp) && read(&chunk.length, sizeof(chunk.length), fp))
		{
			//读头部PCM头部信息
			if (chunk.type[0] == 'f' && chunk.type[1] == 'm' && chunk.type[2] == 't')
			{
				if (!read(&pcm.header.format, sizeof(pcm.header.format), fp) ||
					!read(&pcm.header.channels, sizeof(pcm.header.channels), fp) ||
					!read(&pcm.header.sampleRate, sizeof(pcm.header.sampleRate), fp) ||
					!read(&pcm.header.byteRate, sizeof(pcm.header.byteRate), fp) ||
					!read(&pcm.header.blockAlign, sizeof(pcm.header.blockAlign), fp) ||
					!read(&pcm.header.bitsPerSample, sizeof(pcm.header.bitsPerSample), fp))
				{
					goto clear;
				}

				int skip = chunk.length - 16;//如果length==16,则没有extraBytes字段，length==18,有extraBytes字段
				if (skip > 0)
				{
					if (fseek(fp, skip, SEEK_CUR) != 0)
					{
						goto clear;
					}
				}
				//检查是否是PCM数据，
				if (pcm.header.format != 1 || (pcm.header.channels != 1) && (pcm.header.channels != 2))
				{
					pcm.~PCM();
					goto clear;
				}
			}
			else if (chunk.type[0] == 'd' && chunk.type[1] == 'a' && chunk.type[2] == 't' && chunk.type[3] == 'a')
			{
				pcm.size = chunk.length;
				pcm.data = (uint8*)malloc(pcm.size);
				
				if (!read(pcm.data, pcm.size, fp))
				{
					pcm.~PCM();
					goto clear;
				}
			}
			else
			{
				if (fseek(fp, chunk.length, SEEK_CUR) != 0)
				{
					goto clear;
				}
			}
		}

		result = true;

	} while (0);

clear:
	fclose(fp);
	return result;
}

bool Wave::Save(const char* filename, const PCM & pcm)
{
	if (filename == 0) return false;

	FILE * fp = 0;
	if (fopen_s(&fp, filename, "wb") != 0 || fp == 0)
	{
		return false;
	}

	bool result = false;

	do
	{
		WAVE_HEADER wf;
		wf.magic[0] = 'R';
		wf.magic[1] = 'I';
		wf.magic[2] = 'F';
		wf.magic[3] = 'F';

		wf.type[0] = 'W';
		wf.type[1] = 'A';
		wf.type[2] = 'V';
		wf.type[3] = 'E';

		wf.length = pcm.size + 36;

		if (!write(&wf.magic[0], sizeof(wf.magic), fp))
		{
			break;
		}
		if (!write(&wf.length, sizeof(wf.length), fp))
		{
			break;
		}
		if (!write(&wf.type[0], sizeof(wf.type), fp))
		{
			break;
		}

		WAVE_CHUNK_HEADER chunk;
		chunk.type[0] = 'f';
		chunk.type[1] = 'm';
		chunk.type[2] = 't';
		chunk.type[3] = 32; //一定是32(0x20)
		chunk.length = 16;

		if (!write(&chunk.type[0], sizeof(chunk.type), fp))
		{
			break;
		}
		if (!write(&chunk.length, sizeof(chunk.length), fp))
		{
			break;
		}

		if (!write((void*)(&pcm.header.format), sizeof(pcm.header.format), fp) ||
			!write((void*)(&pcm.header.channels), sizeof(pcm.header.channels), fp) ||
			!write((void*)(&pcm.header.sampleRate), sizeof(pcm.header.sampleRate), fp) ||
			!write((void*)(&pcm.header.byteRate), sizeof(pcm.header.byteRate), fp) ||
			!write((void*)(&pcm.header.blockAlign), sizeof(pcm.header.blockAlign), fp) ||
			!write((void*)(&pcm.header.bitsPerSample), sizeof(pcm.header.bitsPerSample), fp))
		{
			break;
		}

		chunk.type[0] = 'd';
		chunk.type[1] = 'a';
		chunk.type[2] = 't';
		chunk.type[3] = 'a';
		chunk.length = pcm.size;

		if (!write(&chunk.type[0], sizeof(chunk.type), fp))
		{
			break;
		}
		if (!write(&chunk.length, sizeof(chunk.length), fp))
		{
			break;
		}
		if (!write((void*)pcm.data, pcm.size, fp))
		{
			break;
		}
		result = true;

	} while (0);

	fclose(fp);

	return result;
}
