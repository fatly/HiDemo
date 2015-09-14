#include <stdio.h>
#include <stdlib.h>
#include "Wave.h"
#include "WavePlayer.h"

//#define USE_SOUNDTOUCH

#ifdef USE_SOUNDTOUCH
#include "SoundTouch.h"
using namespace soundtouch;
#pragma comment(lib, "libsoundtouch.lib")
#else
#include "libpitchshift.h"
#pragma comment(lib, "libpitchshift.lib")
using namespace e;
#endif
#pragma comment(lib, "Winmm.lib.")


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
#ifdef USE_SOUNDTOUCH
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
#else
	SoundPitch* p = new SoundPitch();
	p->SetChannels(pcm.header.channels);
	p->SetSampleRate(pcm.header.sampleRate);
	p->SetPitch(1.3f);
	
	p->PutSamples((sample_t*)pcm.data, samples);
	int done = p->GetSampleCount();
	int undo = p->UnProcessSamples();
	int n = 0, offset = 0;
	sample_t* src = (sample_t*)pcm.data;
	do 
	{
		n = p->GetSamples(src, samples - offset);
		offset += n;
		src += n * pcm.header.channels;
	} while (n != 0);
#endif
	if (!Wave::Save("f:\\test0.wav", pcm))
	{
		printf("save file failed!\n");
	}

	delete p;
	return 0;
}