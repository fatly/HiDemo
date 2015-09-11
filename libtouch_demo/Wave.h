#pragma once
#include "Defines.h"

#pragma pack(push, 1)

struct PCM_HEADER {
	uint16 format; 			//2 bytes  0x0001          Format tag: 1 = PCM
	uint16 channels; 		//2 bytes  <channels>      Channels: 1 = mono, 2 = stereo
	uint32 sampleRate;		//4 bytes  <sample rate>   Samples per second: e.g., 44100
	uint32 byteRate;		//4 bytes  <bytes/second>  sample rate * block align
	uint16 blockAlign;		//2 bytes  <block align>   channels * bits/sample / 8
	uint16 bitsPerSample; 	//2 bytes  <bits/sample>   8 or 16
	uint16 extraBytes;		// for compatible with WAVEFORMATEX struct 
};

struct PCM{
	PCM_HEADER header;
	uint32 size;
	uint8* data;
	PCM(void);
	~PCM(void);
};

struct WAVE_HEADER{
	char	magic[4];
	uint32  length;
	char	type[4];
};

struct WAVE_CHUNK_HEADER{
	char	type[4];
	uint32	length;
};

struct WAVE_SAMPLE{
	unsigned short * left;
	unsigned short * right;
	unsigned short * data;
};

#pragma pack(pop)

class Wave
{
public:
	Wave(void);
	virtual ~Wave(void);
public:
	static bool Load(PCM & pcm, const char * filename);
	static bool Save(const char* filename, const PCM & pcm);
};

