#ifndef __CORE_JPEG_ENCODER_H__
#define __CORE_JPEG_ENCODER_H__
#include "Defines.h"
#include "Bitmap.h"

namespace e
{
	class HuffmanTable
	{
	public:
		uint codes[256];
		uint8 code_sizes[256];
		uint8 bits[17];
		uint8 values[256];
		uint32 counts[256];
		void Optimize(int tableLenght);
		void Compute(void);
		void Dump(const char* file);
	};

	class HuffmanDCAC
	{
	public:
		int quantization_table[64];
		HuffmanTable dc, ac;
	};

	class Component
	{
	public:
		uint8 h_samples;
		uint8 v_samples;
		int last_dc_value;
	};

	enum subsample_e{Y_ONLY = 0, H1V1 = 1, H2V1 = 2, H2V2 = 3};

	class Encoder
	{
	public:
		Encoder(void);
		virtual ~Encoder(void);
	public:
		int EncodeImage(uint8* output
			, uint8* input
			, int width
			, int height
			, int bitCount
			, int quality);
	private:
		void Initialize(int width, int height, int bitCount, bool nochroma);
		void Pretreatment(uint8* input, int width, int height, int bitCount);
		void InitQuantTable(int quality, bool nochroma);
		void ComputeQuantTable(int32* dst, int16* src, int quality);
		void SubSample(void);
		int16 GetPixel(int x, int y, int channel);
		int16 BlendQuad(int x, int y, int channel);
		int16 BlendDual(int x, int y, int channel);
		int16* GetDQ(int x, int y, int channel);
		void LoadBlock(int16* dst, int x, int y, int channel);
		void LoadBlock(double* dst, int x, int y, int channel);
		void ResetLastDC(void);
		void CodeMCURow(int y, bool write);
		void CodeBlock(int16* data, HuffmanDCAC* huff, Component* comp, bool write);
		void ComputeHuffmanTables(void);
		void DCT(double* data);
		void DCT(int16* data);
		void QuantizePixels(int16* dst, int16* src, int32* quants);
		void QuantizePixels(int16* dst, double* src, int32* quants);
		void Compress(void);
		void PutBits(uint bits, uint len);
		void PutSignalBits(int num, uint len);
		uint BitCount(int value);
		void Cleanup(void);

		void EmitByte(uint8 value);
		void EmitWord(uint16 value);
		void EmitMarker(int value);
		void EmitJFIFAPP0(void);
		void EmitDQT(void);
		void EmitSOF(void);
		void EmitDHT(uint8* bits, uint8* values, int index, bool ac_flag);
		void EmitDHTs(void);
		void EmitSOS(void);
		void EmitStartMarkers(void);
		void EmitEndMarker(void);

		void DumpData(const char* file, int channel);
		bool LoadData(const char* file, int channel);
	private:
		int w;
		int h;
		int w2[3];
		int h2[3];
		int compCount;
		int16* data[6];
		int mcu_w;
		int mcu_h;
		int quality;
		int subsampling;
		Component comp[3];
		HuffmanDCAC huffman[2];
		uint32 bit_buffer;
		uint32 bits_in;
		uint8* output_buffer;
		uint8* original_buffer;
		uint32 output_size;
	};
}

#endif