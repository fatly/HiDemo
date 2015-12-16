#ifndef __HI_LIBAUDIO_CONSTS_H__
#define __HI_LIBAUDIO_CONSTS_H__

namespace e
{
	const double TWOPI = 6.283185308;
	const int kMinFrequency = 0;
	const int kMaxFrequency = 20000;
	const int kDefaultFrequency = 440;				// A-440
	const int kMinAmplitude = 0;
	const int kMaxAmplitude = 100;
	const int kDefaultChannels = 2;
	const int KDefaultBitsPerSample = 16;
	const int kDefaultSamplePerSec = 44100;
	//未定的播放时长值,直播模式用这个值
	const int kUncertainValue = -1;
	const int kUncertainDuration = kUncertainValue;

	const int BITS_PER_BYTE = 8;
	const int kMaxBufferSize = 8 * 1024;     // Size of each allocated buffer, 8KB
	const int kMaxSampleSize = 4 * 1024;
	const int kDefaultTimeout = 5 * 1000;		//5s
}

#endif