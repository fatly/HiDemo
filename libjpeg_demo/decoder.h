#ifndef __CORE_DECODER_H__
#define __CORE_DECODER_H__
#include "BaseClass.h"

namespace e
{
#ifdef STBI_SIMD
	typedef unsigned short dequantize_t;
#else
	typedef uint8 dequantize_t;
#endif

	class Decoder
	{
	public:
		Decoder(void);
		virtual ~Decoder(void);
	public:
		uint8* LoadJpegImage(uint8* buffer, int len, int* out_x, int* out_y, int* comp, int req_comp);
	private:
		bool BuildHaffman(Haffman* h, int* count);
		void GrowBufferUnSafe(Jpeg* j);
		int  Decode(Jpeg* j, Haffman* h);
		int  ExtendReceive(Jpeg* j, int n);
		bool DecodeBlock(Jpeg* j, short data[64], Haffman* hdc, Haffman* hac, int b);
		void IDCTBlock(uint8* out, int out_stride, short data[64], dequantize_t* dequantize);
		bool ParseEntropyCodedData(Jpeg* z);
		bool ProcessMarket(Jpeg* j, int m);
		bool ProcessScanHeader(Jpeg* j);
		bool ProcessFrameHeader(Jpeg* j, int scan);
		bool DecodeJpegHeader(Jpeg* j, int scan);
		bool DecodeJpegImage(Jpeg* j);
		void CleanJpeg(Jpeg* j);
		void YCbCr2RGB(uint8* out, const uint8* y, const uint8* pcb, const uint8* pcr, int count, int step);
		uint8* LoadJpegImage(Jpeg* j, int* out_x, int* out_y, int* comp, int req_comp);
		bool Error(char* str);
	public:
		char* lastError;
	};
}

#endif