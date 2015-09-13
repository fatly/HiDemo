#ifndef __LIBPITCH_SHIFT_TDSTRETCH_H__
#define __LIBPITCH_SHIFT_TDSTRETCH_H__
#include "Define.h"
#include "SampleBuffer.h"

#define USE_AUTO_SEQUENCE_LEN	0
#define USE_AUTO_SEEKWINDOW_LEN 0
#define DEFAULT_SEQUENCE_MS		USE_AUTO_SEQUENCE_LEN
#define DEFAULT_SEEKWINDOW_MS	USE_AUTO_SEEKWINDOW_LEN
#define DEFAULT_OVERLAP_MS		8

namespace e
{
	//time-stretch
	class TDStretch : public FIFOAdapter
	{
	public:
		TDStretch(void);
		virtual ~TDStretch(void);
	public:
		static TDStretch* GetInstance(void);
		SamplePipe* GetInput(void) { return inputBuffer; }
		SamplePipe* GetOutput(void) { return outputBuffer; }
		void AcceptOverlapLength(int overlapLength);
		void SetTempo(float tempo);
		void ClearInputBuffer(void);
		void SetChannels(int channels);
		void EnableQuickSeek(bool enable);
		bool IsQuickSeekEnable(void) const;
		void SetParamters(int sampleRate, int sequenceMS = -1, int seekWindowMS = -1, int overlapMS = -1);
		void GetParamters(int *sampleRate, int *sequenceMS, int *seekWindowMS, int *overlapMS) const;
		int GetInputRequestSamples(void) const { return (int)(nominalSkip + 0.5); }
		int GetOutputBatchSize(void) const { return seekWindowLength - overlapLength;}
		virtual void PutSamples(const sample_t* samples, uint count);
		virtual void Clear(void);
	protected:
		void SetOverlapLength(int length);
		void CalcOverlapLength(int overlapMS);
		virtual void ClearCrossCorrState(void);
		virtual double CalcCrossCorr(const sample_t* mixingPos, const sample_t* compare, double &norm) const;
		virtual double CalcCrossCorrAccumulate(const sample_t* mixingPos, const sample_t* compare, double &norm) const;
		virtual int SeekBestOverlapPositionFull(const sample_t* refPos);
		virtual int SeekBestOverlapPositionQuick(const sample_t* refPos);
		virtual int SeekBestOverlapPosition(const sample_t* refPos);
		virtual void OverlapStereo(sample_t* dst, const sample_t* src) const;
		virtual void OverlapMono(sample_t* dst, const sample_t* src) const;
		virtual void Overlap(sample_t* dst, const sample_t* src, uint pos) const;
		virtual void ClearTempBuffer(void);
		void CalcSeqParamters(void);
		void ProcessSamples(void);
	protected:
		int channels;
		int requestSamples;
		float tempo;
		sample_t* tempBuffer[2];
		int overlapLength;
		int seekLength;
		int seekWindowLength;
		int overlapDividerBits;
		int slopingDivider;
		float nominalSkip;
		float skipFract;
		SampleBuffer* inputBuffer;
		SampleBuffer* outputBuffer;
		bool isQuickSeek;

		int sampleRate;
		int sequenceMS;
		int overlapMS;
		int seekWindowMS;
		bool isAutoSeqSetting;
		bool isAutoSeekSetting;
	};
}


#endif