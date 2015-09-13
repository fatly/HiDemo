#include "TDStretch.h"
#include "SampleBuffer.h"
#include <assert.h>

namespace e
{
	static const short g_scan_offsets[5][24] = {
		{ 124, 186, 248, 310, 372, 434, 496, 558, 620, 682, 744, 806, 868, 930, 992, 1054, 1116, 1178, 1240, 1302, 1364, 1426, 1488, 0 },
		{ -100, -75, -50, -25, 25, 50, 75, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ -20, -15, -10, -5, 5, 10, 15, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ -4, -3, -2, -1, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 121, 114, 97, 114, 98, 105, 108, 32, 104, 99, 117, 111, 116, 100, 110, 117, 111, 115, 0, 0, 0, 0, 0, 0 }
	};

	TDStretch::TDStretch()
	{
		isQuickSeek = false;
		channels = 2;
		tempBuffer[0] = 0;
		tempBuffer[1] = 0;
		overlapLength = 0;
		isAutoSeqSetting = true;
		isAutoSeekSetting = true;
		skipFract = 0;
		tempo = 1.0f;

		inputBuffer = new SampleBuffer();
		assert(inputBuffer);
		outputBuffer = new SampleBuffer();
		assert(outputBuffer);
		//set output buffer of base class
		FIFOAdapter::SetOutput(outputBuffer);

		SetParamters(44100, DEFAULT_SEQUENCE_MS, DEFAULT_SEEKWINDOW_MS, DEFAULT_OVERLAP_MS);
		SetTempo(1.0f);

		Clear();
	}


	TDStretch::~TDStretch()
	{
		if (tempBuffer[1])
		{
			free(tempBuffer[1]);
			tempBuffer[1] = 0;
		}

		if (inputBuffer)
		{
			delete inputBuffer;
			inputBuffer = 0;
		}

		if (outputBuffer)
		{
			delete outputBuffer;
			outputBuffer = 0;
		}
	}

	void TDStretch::SetParamters(int sampleRate, int sequenceMS, int seekWindowMS, int overlapMS)
	{
		if (sampleRate > 0) this->sampleRate = sampleRate;
		if (overlapMS > 0) this->overlapMS = overlapMS;

		if (sequenceMS > 0)
		{
			this->sequenceMS = sequenceMS;
			isAutoSeqSetting = false;
		}
		else if (sequenceMS == 0)
		{
			isAutoSeqSetting = true;
		}

		if (seekWindowMS > 0)
		{
			this->seekWindowMS = seekWindowMS;
			isAutoSeekSetting = false;
		}
		else if (seekWindowMS == 0)
		{
			isAutoSeekSetting = false;
		}

		CalcSeqParamters();
		CalcOverlapLength(overlapMS);
		SetTempo(tempo);
	}

	void TDStretch::GetParamters(int *sampleRate, int *sequenceMS, int *seekWindowMS, int *overlapMS) const
	{
		if (sampleRate)
		{
			*sampleRate = this->sampleRate;
		}

		if (sequenceMS)
		{
			*sequenceMS = isAutoSeqSetting ? (USE_AUTO_SEQUENCE_LEN) : this->sequenceMS;
		}

		if (seekWindowMS)
		{
			*seekWindowMS = isAutoSeekSetting ? (USE_AUTO_SEEKWINDOW_LEN) : this->seekWindowMS;
		}

		if (overlapMS)
		{
			*overlapMS = this->overlapMS;
		}
	}

	void TDStretch::OverlapMono(sample_t* dst, const sample_t* src) const
	{
		sample_t a = (sample_t)0;
		sample_t b = (sample_t)overlapLength;

		for (int i = 0; i < overlapLength; i++)
		{
			dst[i] = (src[i] * a + tempBuffer[0][i] * b) / overlapLength;
			a += 1;
			b -= 1;
		}
	}

	void TDStretch::ClearTempBuffer(void)
	{
		memset(tempBuffer[0], 0, sizeof(sample_t)* overlapLength * channels);
	}

	void TDStretch::ClearInputBuffer(void)
	{
		assert(inputBuffer);
		inputBuffer->Clear();
		ClearTempBuffer();
	}

	void TDStretch::Clear(void)
	{
		assert(outputBuffer);
		outputBuffer->Clear();
		ClearInputBuffer();
	}

	void TDStretch::EnableQuickSeek(bool enable)
	{
		isQuickSeek = enable;
	}

	bool TDStretch::IsQuickSeekEnable(void) const
	{
		return isQuickSeek;
	}

	int TDStretch::SeekBestOverlapPosition(const sample_t* refPos)
	{
		if (isQuickSeek)
		{
			return SeekBestOverlapPositionQuick(refPos);
		}
		else
		{
			return SeekBestOverlapPositionFull(refPos);
		}
	}

	inline void TDStretch::Overlap(sample_t* dst, const sample_t* src, uint pos) const
	{
		if (channels == 1)
		{
			OverlapMono(dst, src + pos);
		}
		else if (channels == 2)
		{
			OverlapStereo(dst, src + pos);
		}
		else
		{
			assert(0);
		}
	}

	int TDStretch::SeekBestOverlapPositionFull(const sample_t* refPos)
	{
		int bestOffset = 0;
		double bestCorr = FLT_MIN, corr = 0;
		double norm = 0;

		bestCorr = CalcCrossCorr(refPos, tempBuffer[0], norm);

		for (int i = 1; i < seekLength; i++)
		{
			corr = CalcCrossCorrAccumulate(refPos + i * channels, tempBuffer[0], norm);

			double temp = (double)(2.0*i - seekLength) / (double)seekLength;
			corr = ((corr + 0.1) * (1.0 - 0.25 * temp * temp));

			if (corr > bestCorr)
			{
				bestCorr = corr;
				bestOffset = i;
			}
		}

		ClearCrossCorrState();
		return bestOffset;
	}

	int TDStretch::SeekBestOverlapPositionQuick(const sample_t* refPos)
	{
		int bestOffset = 0, corrOffset = 0, tempOffset = 0;
		double corr = 0, bestCorr = FLT_MIN, norm = 0;

		bestOffset = g_scan_offsets[0][0];

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; g_scan_offsets[i][j]; j++)
			{
				tempOffset = corrOffset + g_scan_offsets[i][j];
				if (tempOffset >= seekLength) break;

				corr = CalcCrossCorr(refPos + tempOffset * channels, tempBuffer[0], norm);
				double temp = (double)(2 * tempOffset - seekLength) / (double)seekLength;
				corr = ((corr + 0.1) * (1.0 - 0.25 * temp * temp));

				if (corr > bestCorr)
				{
					bestCorr = corr;
					bestOffset = tempOffset;
				}
			}

			corrOffset = bestOffset;
		}

		ClearCrossCorrState();
		return bestOffset;
	}

	void TDStretch::ClearCrossCorrState(void)
	{
		// TODO : 
	}

	void TDStretch::CalcSeqParamters(void)
	{
		// Adjust tempo param according to tempo, so that variating processing sequence length is used
		// at varius tempo settings, between the given low...top limits
#define AUTOSEQ_TEMPO_LOW   0.5     // auto setting low tempo range (-50%)
#define AUTOSEQ_TEMPO_TOP   2.0     // auto setting top tempo range (+100%)

		// sequence-ms setting values at above low & top tempo
#define AUTOSEQ_AT_MIN      125.0
#define AUTOSEQ_AT_MAX      50.0
#define AUTOSEQ_K           ((AUTOSEQ_AT_MAX - AUTOSEQ_AT_MIN) / (AUTOSEQ_TEMPO_TOP - AUTOSEQ_TEMPO_LOW))
#define AUTOSEQ_C           (AUTOSEQ_AT_MIN - (AUTOSEQ_K) * (AUTOSEQ_TEMPO_LOW))

		// seek-window-ms setting values at above low & top tempo
#define AUTOSEEK_AT_MIN     25.0
#define AUTOSEEK_AT_MAX     15.0
#define AUTOSEEK_K          ((AUTOSEEK_AT_MAX - AUTOSEEK_AT_MIN) / (AUTOSEQ_TEMPO_TOP - AUTOSEQ_TEMPO_LOW))
#define AUTOSEEK_C          (AUTOSEEK_AT_MIN - (AUTOSEEK_K) * (AUTOSEQ_TEMPO_LOW))

#define CHECK_LIMITS(x, mi, ma) (((x) < (mi)) ? (mi) : (((x) > (ma)) ? (ma) : (x)))

		double sequence = 0, seek = 0;
		if (isAutoSeqSetting)
		{
			sequence = AUTOSEQ_C + AUTOSEQ_K * tempo;
			sequence = CHECK_LIMITS(sequence, AUTOSEQ_AT_MIN, AUTOSEQ_AT_MAX);
			sequenceMS = (int)(sequence + 0.5);
		}

		if (isAutoSeekSetting)
		{
			seek = AUTOSEEK_C + AUTOSEEK_K * tempo;
			seek = CHECK_LIMITS(seek, AUTOSEEK_AT_MIN, AUTOSEEK_AT_MAX);
			seekWindowMS = (int)(seek + 0.5);
		}

		seekWindowLength = (sampleRate * sequenceMS) / 1000;
		if (seekWindowLength < 2 * overlapLength)
		{
			seekWindowLength = 2 * overlapLength;
		}

		seekLength = (sampleRate * seekWindowMS) / 1000;
	}

	void TDStretch::SetTempo(float tempo)
	{
		this->tempo = tempo;
		CalcSeqParamters();
		nominalSkip = tempo * (seekWindowLength - overlapLength);
		int intskip = (int)(nominalSkip + 0.5);
		requestSamples = max(intskip + overlapLength, seekWindowLength) + seekLength;
	}

	void TDStretch::SetChannels(int channels)
	{
		assert(channels > 0);
		if (this->channels == channels) return;

		this->channels = channels;
		assert(inputBuffer);
		assert(outputBuffer);
		inputBuffer->SetChannels(channels);
		outputBuffer->SetChannels(channels);

		overlapLength = 0;
		SetParamters(sampleRate);
	}

	void TDStretch::ProcessSamples(void)
	{
		int skip = 0, offset = 0, temp = 0;

		while ((int)inputBuffer->GetSampleCount() >= requestSamples)
		{
			offset = SeekBestOverlapPosition(inputBuffer->Begin());
			Overlap(outputBuffer->End((uint)overlapLength), inputBuffer->Begin(), (uint)offset);
			outputBuffer->PutSamples((uint)overlapLength);
			temp = (seekWindowLength - 2 * overlapLength);

			if ((int)inputBuffer->GetSampleCount() < (offset + temp + overlapLength * 2))
			{
				continue;
			}

			outputBuffer->PutSamples(inputBuffer->Begin() + (offset + overlapLength) * channels, (uint)temp);
			assert((offset + temp + overlapLength * 2) <= (int)inputBuffer->GetSampleCount());
			memcpy(tempBuffer[0], inputBuffer->Begin() + (offset + temp + overlapLength) * channels
				, sizeof(sample_t)* overlapLength * channels);

			skipFract += nominalSkip;
			skip = (int)skipFract;
			skipFract -= skip;
			inputBuffer->GetSamples((uint)skip);
		}
	}

	void TDStretch::PutSamples(const sample_t* samples, uint count)
	{
		assert(inputBuffer);
		inputBuffer->PutSamples(samples, count);
		ProcessSamples();
	}

	void TDStretch::AcceptOverlapLength(int overlapLength)
	{
		assert(overlapLength > 0);
		int length = this->overlapLength;
		this->overlapLength = overlapLength;

		if (overlapLength > length)
		{
			int totalBytes = (overlapLength * channels + 16 / sizeof(sample_t)) * sizeof(sample_t);
			tempBuffer[1] = (sample_t*)realloc(tempBuffer[0], totalBytes);
			assert(tempBuffer[1]);
			if (tempBuffer[1] == 0) E_THROW("TDStretch realloc failed!!!");
			tempBuffer[0] = (sample_t*)ALIGN_POINTER_16(tempBuffer[1]);
			ClearTempBuffer();
		}
	}

	TDStretch* TDStretch::GetInstance(void)
	{
		return new TDStretch();
	}
//INTEGER_SAMPLES
#ifdef INTEGER_SAMPLES
	void TDStretch::OverlapStereo(sample_t* dst, const sample_t* src) const
	{
		int j = 0;
		for (int i = 0; i < overlapLength; i++)
		{
			j = 2 * i;
			short temp = (short)(overlapLength - i);
			dst[j + 0] = (src[j+0] * i + tempBuffer[0][j+0] * temp) / overlapLength;
			dst[j + 1] = (src[j+1] * i + tempBuffer[0][j+1] * temp) / overlapLength;
		}
	}

	static inline int closest_2_power(double value)
	{
		return (int)(log(value) / log(2.0) + 0.5);
	}

	void TDStretch::CalcOverlapLength(int overlapMS)
	{
		assert(overlapMS > 0);
		overlapDividerBits = closest_2_power((sampleRate * overlapMS) / 1000.0) - 1;
		if (overlapDividerBits > 9) overlapDividerBits = 9;
		if (overlapDividerBits < 3) overlapDividerBits = 3;
		int overlap = (int)pow(2.0, (int)overlapDividerBits + 1);

		AcceptOverlapLength(overlap);

		slopingDivider = (overlap * overlap - 1) / 3;
	}

	double TDStretch::CalcCrossCorr(const sample_t* mixingPos, const sample_t* compare, double &norm) const
	{
		long corr = 0, lnorm = 0;
		for (int i=0; i<overlapLength; i+=4)
		{
			corr += (mixingPos[i + 0] * compare[i + 0] + mixingPos[i + 1] * compare[i + 1]) >> overlapDividerBits;
			corr += (mixingPos[i + 2] * compare[i + 2] + mixingPos[i + 3] * compare[i + 3]) >> overlapDividerBits;
			lnorm += (mixingPos[i + 0] * mixingPos[i + 0] + mixingPos[i + 1] * mixingPos[i + 1]) >> overlapDividerBits;
			lnorm += (mixingPos[i + 2] * mixingPos[i + 2] + mixingPos[i + 3] * mixingPos[i + 3]) >>overlapDividerBits;
		}

		norm = (double)lnorm;
		return (double)corr / sqrt((norm < 1e-9) ? 1.0 : norm);
	}

	double TDStretch::CalcCrossCorrAccumulate(const sample_t* mixingPos, const sample_t* compare, double &norm) const
	{
		int i = 0, j = 0;
		long corr = 0, lnorm = 0;
		for (i = 1; i <= channels; i++)
		{
			lnorm -= (mixingPos[-i] * mixingPos[-i]) >> overlapDividerBits;
		}

		for (i=0; i<channels * overlapLength; i+=4)
		{
			corr += (mixingPos[i + 0] * compare[i + 0] + mixingPos[i + 1] * compare[i + 1]) >> overlapDividerBits;
			corr += (mixingPos[i + 2] * compare[i + 2] + mixingPos[i + 3] * compare[i + 3]) >> overlapDividerBits;
		}

		for (j = 0; j < channels; j++)
		{
			i--;
			lnorm += (mixingPos[i] * mixingPos[i]) >> overlapDividerBits;
		}

		norm = (double)lnorm;
		return (double)corr / sqrt((norm < 1e-9) ? 1.0 : norm);
	}
#else //FLOAT_SAMPLES
	void TDStretch::OverlapStereo(sample_t* dst, const sample_t* src) const
	{
		float f1 = 0.0f, f2 = 1.0f;
		float scale = 1.0f / (float)overlapLength;

		for (int i = 0; i<2 * (int)overlapLength; i+=2)
		{
			dst[i+0] = src[i+0] * f1 + tempBuffer[0][i+0] * f2;
			dst[i+1] = src[i+1] * f1 + tempBuffer[0][i+1] * f2;

			f1 += scale;
			f2 -= scale;
		}
	}

	void TDStretch::CalcOverlapLength(int overlapMS)
	{
		assert(overlapMS > 0);
		int overlap = (sampleRate * overlapMS) / 1000;
		if (overlap < 16) overlap = 16;
		overlap -= overlap % 8;
		AcceptOverlapLength(overlap);
	}

	double TDStretch::CalcCrossCorr(const sample_t* mixingPos, const sample_t* compare, double &norm) const
	{
		norm = 0;
		double corr = 0; 
		for (int i=0; i<overlapLength * channels; i+=4)
		{
			corr += (mixingPos[i+0] * compare[i+0] + mixingPos[i+1] * compare[i+1]);
			norm += (mixingPos[i+0] * mixingPos[i+0] + mixingPos[i+1] * mixingPos[i+1]);

			corr += (mixingPos[i+2] * compare[i+2] + mixingPos[i+3] * compare[i+3]);
			norm += (mixingPos[i+2] * mixingPos[i+2] + mixingPos[i+3] * mixingPos[i+3]);
		}

		return corr / sqrt((norm<1e-9)?1.0:norm);
	}

	double TDStretch::CalcCrossCorrAccumulate(const sample_t* mixingPos, const sample_t* compare, double &norm) const 
	{
		int i = 0;
		double corr = 0;
		// cancel first normalizer tap from previous round
		for (i = 1; i <= channels; i++)
		{
			norm -= mixingPos[-i] * mixingPos[-i];
		}

		// Same routine for stereo and mono. For Stereo, unroll by factor of 2.
		// For mono it's same routine yet unrollsd by factor of 4.
		for (i = 0; i < channels * overlapLength; i += 4)
		{
			corr += mixingPos[i + 0] * compare[i + 0]
				+ mixingPos[i + 1] * compare[i + 1]
				+ mixingPos[i + 2] * compare[i + 2]
				+ mixingPos[i + 3] * compare[i + 3];
		}

		// update normalizer with last samples of this round
		for (int j = 0; j < channels; j++)
		{
			i--;
			norm += mixingPos[i] * mixingPos[i];
		}

		return corr / sqrt((norm < 1e-9 ? 1.0 : norm));
	}
#endif

}