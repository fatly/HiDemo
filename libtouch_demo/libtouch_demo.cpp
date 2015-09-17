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
#include "Config.h"
#include "Define.h"
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

	uint count = pcm.size / (pcm.header.bitsPerSample / 8);
	uint samples = count / pcm.header.channels;
	uint channels = pcm.header.channels;
	uint sampleRate = pcm.header.sampleRate;
#ifdef USE_SOUNDTOUCH
	SoundTouch *p = new SoundTouch();
	p->setChannels(channels);
	p->setSampleRate(sampleRate);
	p->setPitch(1.3f);
	p->setSetting(SETTING_SEQUENCE_MS, 40);
	p->setSetting(SETTING_SEEKWINDOW_MS, 15);
	p->setSetting(SETTING_OVERLAP_MS, 8);
	//p->setPitchSemiTones(5);
	SAMPLETYPE* src = (SAMPLETYPE*)pcm.data;
	p->putSamples(src, samples);
	int done = p->numSamples();
	int undo = p->numUnprocessedSamples();
	int n = p->receiveSamples(src, done);
	src += done * channels;
	p->flush();
	done = p->numSamples();
#else
	SoundPitch* p = new SoundPitch();
	p->SetChannels(channels);
	p->SetSampleRate(sampleRate);
	p->SetPitch(1.2f);
	p->SetSetting(SETTING_SEQUENCE_MS, 40);
	p->SetSetting(SETTING_SEEKWINDOW_MS, 15);
	p->SetSetting(SETTING_OVERLAP_MS, 8);
	//add samples to buffer and process
	sample_t* src = (sample_t*)pcm.data;
	p->PutSamples(src, samples);
	uint done = p->GetSampleCount();
	p->FetchSamples(src, done);
	src += done * channels;

	p->Flush();
	uint remain = samples - done;
	done = p->GetSampleCount();
	p->FetchSamples(src, min(remain, done));
#endif
	if (!Wave::Save("f:\\test0.wav", pcm))
	{
		printf("save file failed!\n");
	}

	delete p;
	return 0;
}