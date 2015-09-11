#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "aubio.h"
#include "Wave.h"
#include "Defines.h"

#pragma comment(lib, "libaubio.lib")

uint_t win_size = 1024;
uint_t hop_size = win_size / 4;
uint_t samplerate = 44100;
aubio_pitch_t* o = NULL;
fvec_t* in = 0, *out = 0;

void conv(fvec_t* dst, uint8* src, int samples)
{
	short* s = (short*)src;
	for (int i = 0; i < samples; i++, s+=2)
	{
		fvec_set_sample(dst, *s, i);
	}
}

void conv(uint8* dst, fvec_t* src, int samples)
{
	short* d = (short*)dst;
	for (int i = 0; i < samples; i++, d+=2)
	{
		*d = (short)fvec_get_sample(src, i);
	}
}

void do_pitch(uint8* dst, uint8* src, int samples, int channels)
{
	int n = samples / hop_size / channels;
	for (int i = 0; i < n; i++)
	{
		conv(in, src, hop_size);
		aubio_pitch_do(o, in, out);
		conv(dst, out, hop_size);
	
		src += 2;
		dst += 2;
		conv(in, src, hop_size);
		aubio_pitch_do(o, in, out);
		conv(dst, out, hop_size);

		src -= 2;
		dst -= 2;

		src += hop_size * 2 * channels;
		dst += hop_size * 2 * channels;
	}
}

int main(int argc, char* argv[])
{
	const char* fileName = "E:\\Musics\\KConvert\\test0.wav";
	PCM pcm;
	if (!Wave::Load(pcm, fileName))
	{
		printf("load pcm file failed!\n");
		return -1;
	}

	in = new_fvec(hop_size);
	out = new_fvec(hop_size);
	o = new_aubio_pitch("default", win_size, hop_size, samplerate);
	aubio_pitch_set_tolerance(o, 0.85);

	do_pitch(pcm.data, pcm.data, pcm.size / (pcm.header.bitsPerSample / 8), pcm.header.channels);

	del_aubio_pitch(o);
	del_fvec(in);
	del_fvec(out);
	aubio_cleanup();

	if (!Wave::Save("f:\\test.wav", pcm))
	{
		printf("save file failed!\n");
	}

	return 0;
}