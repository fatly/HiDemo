#ifndef __LIBPITCH_SHIFT_TDSTRETCH_H__
#define __LIBPITCH_SHIFT_TDSTRETCH_H__
#include "Define.h"
#include "SampleBuffer.h"

#define USE_AUTO_SEQUENCE_LEN		0
#define USE_AUTO_SEEKWINDOW_LEN		0
#define DEFAULT_SEQUENCE_MS			USE_AUTO_SEQUENCE_LEN
#define DEFAULT_SEEKWINDOW_MS		USE_AUTO_SEEKWINDOW_LEN
#define DEFAULT_OVERLAP_MS			8

namespace e
{
	//time-scaler
	class TimeScale : public FIFOAdapter
	{
	public:
		TimeScale(void);
		virtual ~TimeScale(void);
		static TimeScale* GetInstance(void);
	public:
		SamplePipe* GetInput(void) { return inputBuffer; }
		SamplePipe* GetOutput(void) { return outputBuffer; }
		void AcceptOverlapLength(int overlapLength);
		void SetTempo(float tempo);
		void SetChannels(int channels);
		void EnableQuickSeek(bool enable);
		bool IsQuickSeekEnable(void) const;
		void SetParameters(int sampleRate, int sequenceMS = -1, int seekWindowMS = -1, int overlapMS = -1);
		void GetParameters(int *sampleRate, int *sequenceMS, int *seekWindowMS, int *overlapMS) const;
		int GetInputRequestSamples(void) const { return (int)(nominalSkip + 0.5); }
		int GetOutputBatchSize(void) const { return seekWindowLength - overlapLength;}
		void ClearTempBuffer(void);
		void ClearInputBuffer(void);
		virtual void PutSamples(const sample_t* samples, uint count) override;
		virtual void Clear(void) override;
	protected:
		void SetOverlapLength(int length);
		void CalcOverlapLength(int overlapMS);
		void CalcSequenceParamters(void);
		void ClearCrossCorrState(void);
		double CalcCrossCorr(const sample_t* mixing, const sample_t* compare, double &norm) const;
		double CalcCrossCorrAccumulate(const sample_t* mixing, const sample_t* compare, double &norm) const;
		int SeekBestOverlapPositionFull(const sample_t* refPos);
		int SeekBestOverlapPositionQuick(const sample_t* refPos);
		int SeekBestOverlapPosition(const sample_t* refPos);
		void OverlapStereo(sample_t* dst, const sample_t* src) const;
		void OverlapMono(sample_t* dst, const sample_t* src) const;
		void Overlap(sample_t* dst, const sample_t* src, uint pos) const;
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