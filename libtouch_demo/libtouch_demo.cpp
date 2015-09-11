#include <stdio.h>
#include <stdlib.h>
#include "Wave.h"
#include "WavePlayer.h"
#include "SoundTouch.h"

#ifdef _DEBUG
#pragma comment(lib, "SoundTouchD.lib")
#else
#pragma comment(lib, "SoundTouch.lib")
#endif

#pragma comment(lib, "Winmm.lib.")

using namespace soundtouch;

int main(int argc, char* argv[])
{
	const char* fileName = "E:\\Musics\\KConvert\\test0.wav";
	PCM pcm;
	if (!Wave::Load(pcm, fileName))
	{
		printf("load pcm file failed!\n");
		return -1;
	}

	int count = pcm.size / (pcm.header.bitsPerSample / 8);
	int samples = count / pcm.header.channels;
	SoundTouch *p = new SoundTouch();
	p->setChannels(pcm.header.channels);
	p->setSampleRate(pcm.header.sampleRate);
	p->setPitch(1.3f);
	//p->setPitchSemiTones(5);
	p->putSamples((SAMPLETYPE*)pcm.data, samples);
	int done = p->numSamples();
	int undo = p->numUnprocessedSamples();

	int n = 0, offset = 0;
	short* src = (short*)pcm.data;
	do{
		n = p->receiveSamples(src, samples - offset);
		offset += n;
		src += n * pcm.header.channels;
	} while (n != 0);

	if (!Wave::Save("f:\\test.wav", pcm))
	{
		printf("save file failed!\n");
	}

	delete p;
	return 0;
}