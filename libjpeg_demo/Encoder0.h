#ifndef __CORE_JPEG_ENCODER0_H__
#define __CORE_JPEG_ENCODER0_H__
#include "BaseClass.h"

#define BLOCK_SIZE 64

namespace e
{
	class Encoder0
	{
	public:
		Encoder0();
		virtual ~Encoder0();

		uint32 EncodeImage(uint8* output
			, uint8* input
			, uint32 qualityFactor
			, uint32 format
			, uint32 width
			, uint32 height);

	private:
		uint8* WriteMarkers(uint8* out, uint32 format, uint32 width, uint32 height);
		void ReadYUV422(JpegEncoderStruct* jes, uint8* input);
		void RGB2YUV422(uint8* output, uint8* input, int width, int height);
		void Initialize(JpegEncoderStruct* jes, uint32 format, uint32 width, uint32 height);
		void InitializeQuantizationTables(uint32 qualityFactor);
		uint8* CloseBitStream(uint8* output);
		void DCT(int16* data);
		void Quantization(int16* const data, uint16* const quant_table);
		uint8* Haffman(JpegEncoderStruct* jes, uint16 component, uint8* output);
		uint8* EncodeMCU(JpegEncoderStruct* jes, uint32 format, uint8* output);
	private:
		uint8 LQT[BLOCK_SIZE];
		uint8 CQT[BLOCK_SIZE];
		uint16 ILQT[BLOCK_SIZE];
		uint16 ICQT[BLOCK_SIZE];
		int16 Y1[BLOCK_SIZE];
		int16 Y2[BLOCK_SIZE];
		int16 Y3[BLOCK_SIZE];
		int16 Y4[BLOCK_SIZE];
		int16 CB[BLOCK_SIZE];
		int16 CR[BLOCK_SIZE];
		int16 temp[BLOCK_SIZE];
		int32 lcode;
		uint16 bitindex;

		typedef void (Encoder0::*ReadFormatHandle)(JpegEncoderStruct*, uint8*);

		ReadFormatHandle fnReadFormat;
	};
}

#endif