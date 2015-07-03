#include "private.h"
#include "Encoder.h"
#include <assert.h>
#include <stdio.h>

namespace e
{
	static uint8 zag_table[] = {
// 		0, 1, 5, 6, 14, 15, 27, 28,
// 		2, 4, 7, 13, 16, 26, 29, 42,
// 		3, 8, 12, 17, 25, 30, 41, 43,
// 		9, 11, 18, 24, 31, 40, 44, 53,
// 		10, 19, 23, 32, 39, 45, 52, 54,
// 		20, 22, 33, 38, 46, 51, 55, 60,
// 		21, 34, 37, 47, 50, 56, 59, 61,
// 		35, 36, 48, 49, 57, 58, 62, 63
		0, 1, 8, 16, 9, 2, 3, 10,
		17, 24, 32, 25, 18, 11, 4, 5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13, 6, 7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63
	};

	static int16 luminance_quant_table[] = {
		16, 11, 10, 16, 24, 40, 51, 61,
		12, 12, 14, 19, 26, 58, 60, 55,
		14, 13, 16, 24, 40, 57, 69, 56,
		14, 17, 22, 29, 51, 87, 80, 62,
		18, 22, 37, 56, 68, 109, 103, 77,
		24, 35, 55, 64, 81, 104, 113, 92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103, 99
	};

	static int16 chrominance_quant_table[] = {
		17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99
	};

	// Various JPEG enums and tables.
	enum { 
		M_SOF0 = 0xC0, 
		M_DHT = 0xC4, 
		M_SOI = 0xD8, 
		M_EOI = 0xD9, 
		M_SOS = 0xDA, 
		M_DQT = 0xDB, 
		M_APP0 = 0xE0 
	};

	enum { 
		DC_LUM_CODES = 12, 
		AC_LUM_CODES = 256, 
		DC_CHROMA_CODES = 12, 
		AC_CHROMA_CODES = 256, 
		MAX_HUFF_SYMBOLS = 257, 
		MAX_HUFF_CODESIZE = 32 
	};

	inline void BGR2YCC(uint8 b, uint8 g, uint8 r,  int8 & Y, int8 & Cb, int8 & Cr)
	{
		Y = (0.299 * r) + (0.587 * g) + (0.114 * b) - 128;
		Cb = -(0.168736 * r) - (0.331264 * g) + (0.5 * b);
		Cr = (0.5 * r) - (0.418688 * g) - (0.081312 * b);
	}

	struct SymbolFreq{ uint key, index; };

	SymbolFreq* RadixSortSyms(int count, SymbolFreq* sym0, SymbolFreq* sym1)
	{
		const uint maxPasses = 4;
		uint32 hist[256 * maxPasses] = { 0 };
		for (int i = 0; i < count; i++)
		{
			uint freq = sym0[i].key;
			hist[freq & 0xff]++;
			hist[256+((freq >> 8) & 0xff)]++;
			hist[256*2+((freq >> 16) & 0xff)]++;
			hist[256*3+((freq >> 24) & 0xff)]++;
		}

		uint totalPasses = maxPasses;
		SymbolFreq* p0 = sym0, *p1 = sym1;

		while ((totalPasses > 1) && (count == hist[(totalPasses - 1) * 256]))
		{
			totalPasses--;
		}

		for (uint pass_shift = 0, pass = 0; pass < totalPasses; pass++, pass_shift += 8)
		{
			uint offset = 0;
			uint offsets[256] = { 0 };
			const uint32* p = &hist[pass << 8];

			for (int i = 0; i < 256; i++)
			{
				offsets[i] = offset;
				offset += p[i];
			}

			for (int i = 0; i < count; i++)
			{
				p1[offsets[(p0[i].key >> pass_shift) & 0xff]++] = p0[i];
			}

			SymbolFreq* temp = p0;
			p0 = p1;
			p1 = temp;
		}

		return p0;
	}

	void CalculateMinimumRedundancy(SymbolFreq* A, int n)
	{
		if (n == 0)
		{
			return;
		}
		else if (n == 1)
		{
			A[0].key = 1; 
			return;
		}

		int root, leaf, next, avb1, used, depth;

		root = 0;
		leaf = 2;
		A[0].key += A[1].key;

		for (next = 1; next < n - 1; next++)
		{
			if (leaf >= n || A[root].key < A[leaf].key)
			{
				A[next].key = A[root].key;
				A[root].key = next;
				root++;
			}
			else
			{
				A[next].key = A[leaf].key;
				leaf++;
			}

			if (leaf >= n || (root < next && A[root].key < A[leaf].key))
			{
				A[next].key += A[root].key;
				A[root].key = next;
				root++;
			}
			else
			{
				A[next].key += A[leaf].key;
				leaf++;
			}
		}

		A[n - 2].key = 0;
		for (next = n - 3; next >= 0; next--)
		{
			A[next].key = A[A[next].key].key + 1;
		}

		avb1 = 1;
		used = depth = 0;
		root = n - 2;
		next = n - 1;

		while (avb1 > 0)
		{
			while (root >= 0 && (int)A[root].key == depth)
			{
				used++;
				root--;
			}

			while (avb1 > used)
			{
				A[next].key = depth;
				next--;
				avb1--;
			}

			avb1 = 2 * used; depth++; used = 0;
		}
	}

	static void HuffmanEnforceMaxCodeSize(int* codesCount, int codeListLen, int maxCodeSize)
	{
		if (codeListLen <= 1) return;

		for (int i = maxCodeSize + 1; i <= MAX_HUFF_CODESIZE; i++)
		{
			codesCount[maxCodeSize] += codesCount[i];
		}

		uint32 total = 0;
		for (int i = maxCodeSize; i > 0; i--)
		{
			total += (((uint32)codesCount[i]) << (maxCodeSize - i));
		}

		while (total != (1UL << maxCodeSize))
		{
			codesCount[maxCodeSize]--;

			for (int i = maxCodeSize - 1; i > 0; i--)
			{
				if (codesCount[i])
				{
					codesCount[i]--;
					codesCount[i + 1] += 2;
					break;
				}
			}
			
			total--;
		}
	}

	void HuffmanTable::Optimize(int tableLenght)
	{
		SymbolFreq sym0[MAX_HUFF_SYMBOLS], sym1[MAX_HUFF_SYMBOLS];
		sym0[0].key = 1; sym0[0].index = 0;
		int usedCount = 1;

		for (int i = 0; i < tableLenght; i++)
		{
			if (counts[i])
			{
				sym0[usedCount].key = counts[i];
				sym0[usedCount].index = i + 1;
				usedCount++;
			}
		}

		SymbolFreq* syms = RadixSortSyms(usedCount, sym0, sym1);
		CalculateMinimumRedundancy(syms, usedCount);

		int codesCount[MAX_HUFF_CODESIZE + 1] = { 0 };
		for (int i = 0; i < usedCount; i++)
		{
			codesCount[syms[i].key]++;
		}

		const uint CODE_SIZE_LIMIT = 16;
		HuffmanEnforceMaxCodeSize(codesCount, usedCount, CODE_SIZE_LIMIT);

		memset(bits, 0, sizeof(bits));
		for (int i = 1; i <= (int)CODE_SIZE_LIMIT; i++)
		{
			bits[i] = static_cast<uint8>(codesCount[i]);
		}

		for (int i = CODE_SIZE_LIMIT; i >= 1; i--)
		{
			if (bits[i])
			{
				bits[i]--;
				break;
			}
		}

		for (int i = usedCount - 1; i >= 1; i--)
		{
			values[usedCount - 1 - i] = static_cast<uint8>(syms[i].index - 1);
		}
	}

	void HuffmanTable::Compute(void)
	{
		int last_p, si;
		uint8 huffSize[257];
		uint huffCode[257];
		uint code;

		int p = 0; 
		for (char l = 1; l <= 16; l++)
		{
			for (int i = 1; i <= bits[l]; i++)
			{
				huffSize[p++] = l;
			}
		}

		huffSize[p] = 0;
		last_p = p;

		code = 0;
		si = huffSize[0];
		p = 0;

		while (huffSize[p])
		{
			while (huffSize[p] == si)
			{
				huffCode[p++] = code++;
			}

			code <<= 1;
			si++;
		}

		memset(codes, 0, sizeof(codes[0]) * 256);
		memset(code_sizes, 0, sizeof(code_sizes[0]) * 256);

		for (p = 0; p < last_p; p++)
		{
			codes[values[p]] = huffCode[p];
			code_sizes[values[p]] = huffSize[p];
		}
	}

	void HuffmanTable::Dump(const char* file)
	{
		FILE* fp = 0;
		fopen_s(&fp, file, "w");

		if (fp)
		{
			for (int i = 0; i < 256; i++)
			{
				fprintf(fp, "%d ", codes[i]);
			}

			fprintf(fp, "\r\n");

			for (int i = 0; i < 256; i++)
			{
				fprintf(fp, "%d ", code_sizes[i]);
			}

			fprintf(fp, "\r\n");

			for (int i = 0; i < 17; i++)
			{
				fprintf(fp, "%d ", bits[i]);
			}

			fprintf(fp, "\r\n");

			for (int i = 0; i < 256; i++)
			{
				fprintf(fp, "%d ", values[i]);
			}

			fprintf(fp, "\r\n");

			for (int i = 0; i < 256; i++)
			{
				fprintf(fp, "%d ", counts[i]);
			}

			fprintf(fp, "\r\n");

			fclose(fp);
		}
	}

	Encoder::Encoder()
	{
		w = h = 0;
		w2[0] = w2[1] = w2[2] = 0;
		h2[0] = h2[1] = h2[2] = 0;
		data[0] = data[1] = data[3] = 0;
		compCount = 0;
		quality = 85;
		subsampling = -1;
		output_buffer = 0;
	}

	Encoder::~Encoder()
	{
		Cleanup();
	}

	void Encoder::Initialize(int width, int height, int bitCount, bool nochroma)
	{
		switch (subsampling)
		{
		case Y_ONLY:
			comp[0].h_samples = 1;
			comp[0].v_samples = 1;
			mcu_w = 8;
			mcu_h = 8;
			break;
		case H1V1:
			comp[0].h_samples = 1;
			comp[0].v_samples = 1;
			comp[1].h_samples = 1;
			comp[1].v_samples = 1;
			comp[2].h_samples = 1;
			comp[2].v_samples = 1;
			mcu_w = 8;
			mcu_h = 8;
			break;
		case H2V1:
			comp[0].h_samples = 2;
			comp[0].v_samples = 1;
			comp[1].h_samples = 1;
			comp[1].v_samples = 1;
			comp[2].h_samples = 1;
			comp[2].v_samples = 1;
			mcu_w = 16;
			mcu_h = 8;
			break;
		case H2V2:
			comp[0].h_samples = 2;
			comp[0].v_samples = 2;
			comp[1].h_samples = 1;
			comp[1].v_samples = 1;
			comp[2].h_samples = 1;
			comp[2].v_samples = 1;
			mcu_w = 16;
			mcu_h = 16;
			break;
		default:
			assert(0);
			break;
		}

		w = width;
		h = height;
		w2[0] = w2[1] = w2[2] = (w + mcu_w - 1) & (~(mcu_w - 1));
		h2[0] = h2[1] = h2[2] = (h + mcu_h - 1) & (~(mcu_h - 1));

		for (int i = 0; i < compCount; i++)
		{
			data[i] = (int16*)malloc(w2[i] * h2[i] * sizeof(int16));
			assert(data[i] != 0);
		}

		memset(&huffman, 0, sizeof(huffman));

		InitQuantTable(quality, nochroma);

		ResetLastDC();
	}

	void Encoder::InitQuantTable(int quality, bool nochroma)
	{
		ComputeQuantTable(huffman[0].quantization_table, luminance_quant_table, quality);
		ComputeQuantTable(huffman[1].quantization_table, nochroma ? luminance_quant_table : chrominance_quant_table, quality);
	}

	void Encoder::ComputeQuantTable(int32* dst, int16* src, int quality)
	{
		int q;

		if (quality < 50)
		{
			q = 5000 / quality;
		}
		else
		{
			q = 200 - quality * 2;
		}

		for (int i = 0; i < 64; i++)
		{
			int32 j = (src[i] * q + 50) / 100;
			dst[i] = MIN(MAX(j, 1), 1024 / 3);
		}

		if (dst[0] > 8) dst[0] = (dst[0] + 24) / 4;
		if (dst[1] > 24) dst[1] = (dst[1] + 24) / 2;
		if (dst[2] > 24) dst[2] = (dst[2] + 24) / 2;
	}

	void Encoder::ResetLastDC(void)
	{
		bit_buffer = 0;
		bits_in = 0;
		comp[0].last_dc_value = 0;
		comp[1].last_dc_value = 0;
		comp[2].last_dc_value = 0;
	}

	void Encoder::Pretreatment(uint8* input, int width, int height, int bitCount)
	{
		//BGR2YCC
		int bytesPerPixel = bitCount / 8;
		int lineBytes = WIDTHBYTES(width*bitCount);

		for (int y = 0; y < height; y++)
		{
			uint8* p = input + lineBytes * y;
			
			for (int x = 0; x < width; x++)
			{
				int8 Y, Cb, Cr;

				BGR2YCC(*(p + 0), *(p + 1), *(p + 2), Y, Cb, Cr);

				data[0][w2[0] * y + x] = Y;
				data[1][w2[1] * y + x] = Cb;
				data[2][w2[2] * y + x] = Cr;

				p += bytesPerPixel;
			}

			for (int x = width; x < w2[0]; x++)
			{
				data[0][w2[0] * y + x] = data[0][w2[0] * y + width - 1];
				data[1][w2[1] * y + x] = data[1][w2[1] * y + width - 1];
				data[2][w2[2] * y + x] = data[2][w2[2] * y + width - 1];
			}
		}

		for (int y = height; y < h2[0]; y++)
		{
			for (int x = 0; x < w2[0]; x++)
			{
				data[0][w2[0] * y + x] = data[0][w2[0] * (y - 1) + x];
				data[1][w2[1] * y + x] = data[1][w2[1] * (y - 1) + x];
				data[2][w2[2] * y + x] = data[2][w2[2] * (y - 1) + x];
			}
		}

		SubSample();

		// overflow white and black, making distortions overflow as well,
		// so distortions (ringing) will be clamped by the decoder
		if (huffman[0].quantization_table[0] > 2)
		{
			for (int c = 0; c < compCount; c++)
			{
				for (int y = 0; y < h2[c]; y++)
				{
					for (int x = 0; x < w2[c]; x++)
					{
						int16 px = data[c][w2[c] * y + x];

						if (px <= -128)
							px -= huffman[0].quantization_table[0];
						else if (px >= 128)
							px += huffman[0].quantization_table[0];

						data[c][w2[c] * y + x] = px;
					}
				}
			}
		}

		Dump("f:\\dump1.txt", 0);
	}

	inline int16 Encoder::GetPixel(int x, int y, int channel)
	{
		return data[channel][w2[channel] * y + x];
	}

	inline int16 Encoder::BlendQuad(int x, int y, int channel)
	{
		int a = 129 - abs(GetPixel(x, y, 0));
		int b = 129 - abs(GetPixel(x + 1, y, 0));
		int c = 129 - abs(GetPixel(x, y + 1, 0));
		int d = 129 - abs(GetPixel(x + 1, y + 1, 0));

		return (GetPixel(x, y, channel) * a
			+ GetPixel(x + 1, y, channel) * b
			+ GetPixel(x, y + 1, channel) * c
			+ GetPixel(x + 1, y + 1, channel) * d) / ((a + b + c + d) == 0 ? 1 : (a + b + c + d));
	}

	inline int16 Encoder::BlendDual(int x, int y, int channel)
	{
		int16 a = 129 - abs(GetPixel(x, y, 0));
		int16 b = 129 - abs(GetPixel(x + 1, y, 0));
		return (GetPixel(x, y, channel)*a + GetPixel(x + 1, y, channel) * b) / (a + b);
	}

	void Encoder::SubSample(void)
	{
		if (comp[0].h_samples == 2)
		{
			if (comp[0].v_samples == 2)
			{
				for (int c = 1; c < compCount; c++)
				{
					for (int y = 0; y < h2[c]; y+=2)
					{
						for (int x = 0; x < w2[c]; x+=2)
						{
							data[c][w2[c] / 4 * y + x / 2] = BlendQuad(x, y, c);
						}
					}

					w2[c] /= 2;
					h2[c] /= 2;
				}
			}
			else
			{
				for (int c = 1; c < compCount; c++)
				{
					for (int y = 0; y < h2[c]; y++)
					{
						for (int x = 0; x < w2[c]; x+=2)
						{
							data[c][w2[c] / 2 * y + x / 2] = BlendDual(x, y, c);
						}
					}

					w2[c] /= 2;
				}
			}
		}
	}

	void Encoder::Compress(void)
	{
		for (int c = 0; c < compCount; c++)
		{
			int16 sample[64];
			for (int y = 0; y < h2[c]; y += 8)
			{
				for (int x = 0; x < w2[c]; x += 8)
				{
					LoadBlock(sample, x, y, c);
					QuantizePixels(GetDQ(x, y, c), sample, huffman[c>0].quantization_table);
				}
			}
		}

		Dump("f:\\dump1.txt", 0);

		for (int y = 0; y < h; y += mcu_h)
		{
			CodeMCURow(y, false);
		}		

		ComputeHuffmanTables();
		ResetLastDC();

		huffman[0].dc.Dump("f:\\huff_dump1.txt");

		EmitStartMarkers();
		for (int y = 0; y < h; y += mcu_h)
		{
			CodeMCURow(y, true);
		}
		EmitEndMarker();
	}

	inline int16* Encoder::GetDQ(int x, int y, int channel)
	{
		return &data[channel][64 * (y / 8 * w2[channel] / 8 + x / 8)];
	}

	void Encoder::LoadBlock(int16* dst, int x, int y, int channel)
	{
		for (int i = 0; i < 8; i++, dst+=8)
		{
			dst[0] = GetPixel(x + 0, y + i, channel);
			dst[1] = GetPixel(x + 1, y + i, channel);
			dst[2] = GetPixel(x + 2, y + i, channel);
			dst[3] = GetPixel(x + 3, y + i, channel);
			dst[4] = GetPixel(x + 4, y + i, channel);
			dst[5] = GetPixel(x + 5, y + i, channel);
			dst[6] = GetPixel(x + 6, y + i, channel);
			dst[7] = GetPixel(x + 7, y + i, channel);
		}
	}

	static inline int16 RoundToZero(int16 j, const int32 quant)
	{
		if (j < 0)
		{
			int16 temp = -j + (quant >> 1);
			return (temp < quant) ? 0 : static_cast<int16>(-(temp / quant));
		}
		else
		{
			int16 temp = j + (quant >> 1);
			return (temp < quant) ? 0 : static_cast<int16>(temp / quant);
		}
	}

	void Encoder::QuantizePixels(int16* dst, int16* src, int32* quants)
	{
		DCT(src);

		for (int i = 0; i < 64; i++)
		{
			dst[i] = RoundToZero(src[zag_table[i]], quants[i]);
		}
	}

	void Encoder::DCT(int16* data)
	{
		int32 x0, x1, x2, x3, x4, x5, x6, x7, x8;

		//all values are shifted left by 10 and rounded off to nearest integer
		static uint16 c1 = 1420; // cos PI/16 * root(2);
		static uint16 c2 = 1338; // cos PI/8 * root(2);
		static uint16 c3 = 1204; // cos 3PI/16 * root(2);
		static uint16 c5 = 805;  // cos 5PI/16 * root(2);
		static uint16 c6 = 554;  // cos 3PI/8 * root(2);
		static uint16 c7 = 283;  // cos 7PI/16 * root(2);
		static uint16 s1 = 3;
		static uint16 s2 = 10;
		static uint16 s3 = 13;

		for (int i = 8; i > 0; i--)
		{
			x8 = data[0] + data[7];
			x0 = data[0] - data[7];
			x7 = data[1] + data[6];
			x1 = data[1] - data[6];
			x6 = data[2] + data[5];
			x2 = data[2] - data[5];
			x5 = data[3] + data[4];
			x3 = data[3] - data[4];
			x4 = x8 + x5;
			x8 -= x5;
			x5 = x7 + x6;
			x7 -= x6;

			data[0] = (int16)(x4 + x5);
			data[4] = (int16)(x4 - x5);
			data[2] = (int16)((x8 * c2 + x7 * c6) >> s2);
			data[6] = (int16)((x8 * c6 - x7 * c2) >> s2);
			data[7] = (int16)((x0 * c7 - x1 * c5 + x2 * c3 - x3 * c1) >> s2);
			data[5] = (int16)((x0 * c5 - x1 * c1 + x2 * c7 + x3 * c3) >> s2);
			data[3] = (int16)((x0 * c3 - x1 * c7 - x2 * c1 - x3 * c5) >> s2);
			data[1] = (int16)((x0 * c1 + x1 * c3 + x2 * c5 + x3 * c7) >> s2);
			data += 8;
		}

		data -= 64;
		for (int i = 8; i > 0; i--)
		{
			x8 = data[0] + data[56];
			x0 = data[0] - data[56];
			x7 = data[8] + data[48];
			x1 = data[8] - data[48];
			x6 = data[16] + data[40];
			x2 = data[16] - data[40];
			x5 = data[24] + data[32];
			x3 = data[24] - data[32];
			x4 = x8 + x5;
			x8 -= x5;
			x5 = x7 + x6;
			x7 -= x6;

			data[0] = (int16)((x4 + x5) >> s1);
			data[32] = (int16)((x4 - x5) >> s1);
			data[16] = (int16)((x8 * c2 + x7 * c6) >> s3);
			data[48] = (int16)((x8 * c6 - x7 * c2) >> s3);
			data[56] = (int16)((x0 * c7 - x1 * c5 + x2 * c3 - x3 * c1) >> s3);
			data[40] = (int16)((x0 * c5 - x1 * c1 + x2 * c7 + x3 * c3) >> s3);
			data[24] = (int16)((x0 * c3 - x1 * c7 - x2 * c1 - x3 * c5) >> s3);
			data[8] = (int16)((x0 * c1 + x1 * c3 + x2 * c5 + x3 * c7) >> s3);

			data++;
		}
	}

	void Encoder::CodeMCURow(int y, bool write)
	{
		if (compCount == 1)
		{
			for (int x = 0; x < w; x += mcu_w)
			{
				CodeBlock(GetDQ(x, y, 0), &huffman[0], &comp[0], write);
			}
		}
		else if (comp[0].h_samples == 1 && comp[0].v_samples == 1)
		{
			for (int x = 0; x < w; x += mcu_w)
			{
				CodeBlock(GetDQ(x, y, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x, y, 1), &huffman[1], &comp[1], write);
				CodeBlock(GetDQ(x, y, 2), &huffman[1], &comp[1], write);
			}
		}
		else if (comp[0].h_samples == 2 && comp[0].v_samples == 1)
		{
			for (int x = 0; x < w; x += mcu_w)
			{
				CodeBlock(GetDQ(x + 0, y, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x + 8, y, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x / 2, y, 1), &huffman[1], &comp[1], write);
				CodeBlock(GetDQ(x / 2, y, 2), &huffman[1], &comp[2], write);
			}
		}
		else if (comp[0].h_samples == 2 && comp[0].v_samples == 2)
		{
			for (int x = 0; x < w; x += mcu_w)
			{
				CodeBlock(GetDQ(x + 0, y + 0, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x + 8, y + 0, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x + 0, y + 8, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x + 8, y + 8, 0), &huffman[0], &comp[0], write);
				CodeBlock(GetDQ(x / 2, y / 2, 1), &huffman[1], &comp[1], write);
				CodeBlock(GetDQ(x / 2, y / 2, 2), &huffman[1], &comp[2], write);
			}
		}
	}

	void Encoder::CodeBlock(int16* data, HuffmanDCAC* huff, Component* comp, bool write)
	{
		const int dc_delta = data[0] - comp->last_dc_value;
		comp->last_dc_value = data[0];

		const uint nbits = BitCount(dc_delta);

		if (write)
		{
			PutBits(huff->dc.codes[nbits], huff->dc.code_sizes[nbits]);
			PutSignalBits(dc_delta, nbits);
		}
		else
		{
			huff->dc.counts[nbits]++;
		}

		int run_len = 0;
		for (int i = 1; i < 64; i++)
		{
			const int16 ac_val = data[i];
			if (ac_val == 0)
			{
				run_len++;
			}
			else
			{
				while (run_len >= 16)
				{
					if (write)
						PutBits(huff->ac.codes[0xF0], huff->ac.code_sizes[0xF0]);
					else
						huff->ac.counts[0xF0]++;

					run_len -= 16;
				}

				const uint nbits = BitCount(ac_val);
				const int code = (run_len << 4) + nbits;

				if (write)
				{
					PutBits(huff->ac.codes[code], huff->ac.code_sizes[code]);
					PutSignalBits(ac_val, nbits);
				}
				else
				{
					huff->ac.counts[code]++;
				}

				run_len = 0;
			}
		}

		if (run_len)
		{
			if (write)
				PutBits(huff->ac.codes[0], huff->ac.code_sizes[0]);
			else
				huff->ac.counts[0]++;
		}
	}

	inline void Encoder::PutBits(uint bits, int len)
	{
		bit_buffer |= ((uint32)(bits << (24 - (bits_in += len))));

		while (bits_in >= 8)
		{
			uint8 c;
#define PUT_BYTE(c) {*output_buffer++=(c);}
			PUT_BYTE(c = (uint8)((bit_buffer >> 16) & 0xff));
			if (c == 0xff)
				PUT_BYTE(0x00);

			bit_buffer <<= 8;
			bits_in -= 8;
		}
	}

	inline void Encoder::PutSignalBits(int num, int len)
	{
		if (num < 0) num--;

		PutBits(num & ((1 << len) - 1), len);
	}

	inline uint Encoder::BitCount(int value)
	{
		if (value < 0) value = -value;

		uint count = 0;

		while (value)
		{
			count++;
			value >>= 1;
		}

		return count;
	}

	void Encoder::ComputeHuffmanTables(void)
	{
		huffman[0].dc.Optimize(DC_LUM_CODES);
		huffman[0].dc.Compute();

		huffman[0].ac.Optimize(AC_LUM_CODES);
		huffman[0].ac.Compute();

		if (compCount > 1)
		{
			huffman[1].dc.Optimize(DC_CHROMA_CODES);
			huffman[1].dc.Compute();

			huffman[1].ac.Optimize(AC_CHROMA_CODES);
			huffman[1].ac.Compute();
		}
	}

	void Encoder::Cleanup(void)
	{
		w = h = 0;
		for (int i = 0; i < 3; i++)
		{
			w2[i] = h2[i] = 0;

			if (data[i])
			{
				free(data[i]);
				data[i] = 0;
			}
		}

		output_buffer = 0;
	}

	inline void Encoder::EmitByte(uint8 value)
	{
		*output_buffer++ = value;
	}

	inline void Encoder::EmitWord(uint16 value)
	{
		EmitByte((uint8)(value >> 8) & 0xff);
		EmitByte((uint8)(value & 0xff));
	}

	inline void Encoder::EmitMarker(int value)
	{
		EmitByte(uint8(0xff));
		EmitByte(uint8(value));
	}

	inline void Encoder::EmitJFIFAPP0(void)
	{
		EmitMarker(M_APP0);
		EmitWord(2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1);
		EmitByte(0x4a); EmitByte(0x46); EmitByte(0x49); EmitByte(0x46);
		EmitByte(0);
		EmitByte(1);
		EmitByte(1);
		EmitByte(0);
		EmitWord(1);
		EmitWord(1);
		EmitByte(0);
		EmitByte(0);
	}

	inline void Encoder::EmitDQT(void)
	{
		for (int c = 0; c < ((compCount == 3) ? 2 : 1); c++)
		{
			EmitMarker(M_DQT);
			EmitWord(64 + 1 + 2);
			EmitByte(static_cast<uint8>(c));
			for (int i = 0; i < 64; i++)
			{
				EmitByte(static_cast<uint8>(huffman[c].quantization_table[i]));
			}
		}
	}

	inline void Encoder::EmitSOF(void)
	{
		EmitMarker(M_SOF0); //baseline
		EmitWord(3 * compCount + 2 + 5 + 1);
		EmitByte(8); //precision
		EmitWord(h);
		EmitWord(w);
		EmitByte(compCount);

		for (int i = 0; i < compCount; i++)
		{
			EmitByte(static_cast<uint8>(i + 1)); // component ID
			EmitByte((comp[i].h_samples << 4) + comp[i].v_samples);//h and v sampling;
			EmitByte(i > 0);// quantization table num
		}
	}

	inline void Encoder::EmitDHT(uint8* bits, uint8* values, int index, bool ac_flag)
	{
		EmitMarker(M_DHT);
		int lenght = 0;
		for (int i = 1; i <= 16; i++)
		{
			lenght += bits[i];
		}

		EmitWord(lenght + 2 + 1 + 16);
		EmitByte(static_cast<uint8>((ac_flag << 4) + index));

		for (int i = 1; i <= 16; i++)
		{
			EmitByte(bits[i]);
		}

		for (int i = 0; i < lenght; i++)
		{
			EmitByte(values[i]);
		}
	}

	inline void Encoder::EmitDHTs(void)
	{
		EmitDHT(huffman[0].dc.bits, huffman[0].dc.values, 0, false);
		EmitDHT(huffman[0].ac.bits, huffman[0].ac.values, 0, true);

		if (compCount == 3)
		{
			EmitDHT(huffman[1].dc.bits, huffman[1].dc.values, 1, false);
			EmitDHT(huffman[1].ac.bits, huffman[1].ac.values, 1, true);
		}
	}

	inline void Encoder::EmitSOS(void)
	{
		EmitMarker(M_SOS);
		EmitWord(2 * compCount + 2 + 1 + 3);
		EmitByte(compCount);

		for (int i = 0; i < compCount; i++)
		{
			EmitByte(static_cast<uint8>(i + 1));

			if (i == 0)
				EmitByte((0 << 4) + 0);
			else
				EmitByte((1 << 4) + 1);
		}
		//spectral selection
		EmitByte(0);
		EmitByte(63);
		EmitByte(0);
	}

	inline void Encoder::EmitStartMarkers(void)
	{
		EmitMarker(M_SOI);
		EmitJFIFAPP0();
		EmitDQT();
		EmitSOF();
		EmitDHTs();
		EmitSOS();
	}

	inline void Encoder::EmitEndMarker(void)
	{
		PutBits(0x7f, 7);
		EmitMarker(M_EOI);
	}

	void Encoder::Dump(const char* file, int channel)
	{
		FILE *fp = 0;
		fopen_s(&fp, file, "w");

		if (fp)
		{
			for (int y = 0; y < h2[channel]; y++)
			{
				for (int x = 0; x < w2[channel]; x++)
				{
					int value = data[channel][w2[channel] * y + x];
					fprintf(fp, "%d ", value);
				}
				fprintf(fp, "\r\n");
			}

			fclose(fp);
		}
	}

	int Encoder::EncodeImage(uint8* output, uint8* input, int width, int height, int bitCount, int quality)
	{
		int bpp = bitCount / 8;

		if (bpp == 1)
			subsampling = Y_ONLY;
		else
			subsampling = H2V2;

		this->compCount = bpp>1?3:1;
		this->quality = quality;
		this->output_buffer = output;

		bool nochroma = false;
		if (bpp == 1) nochroma = true;

		Initialize(width, height, bitCount, nochroma);

		Pretreatment(input, width, height, bitCount);

		Compress();

		int size = output_buffer - output;

		Cleanup();

		return size;
	}
}


