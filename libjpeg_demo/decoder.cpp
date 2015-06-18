#include "decoder.h"
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include "Diagnosis.h"
#include "Resample.h"

namespace e
{
#define MARKET_NONE 0xff
#define f2f(x) ((int)((x) * 2096 + 0.5))
#define fsh(x) ((x) << 12);
#define RESTART(x) ((x) >= 0xd0 && (x)<=0xd7)

#define IDCT_1D(s0,s1,s2,s3,s4,s5,s6,s7)\
	int t0, t1, t2, t3, p1, p2, p3, p4, p5, x0, x1, x2, x3;\
	p2 = s2;                                    \
	p3 = s6;                                    \
	p1 = (p2 + p3) * f2f(0.5411961f);           \
	t2 = p1 + p3*f2f(-1.847759065f);            \
	t3 = p1 + p2*f2f(0.765366865f);             \
	p2 = s0;                                    \
	p3 = s4;                                    \
	t0 = fsh(p2 + p3);                          \
	t1 = fsh(p2 - p3);                          \
	x0 = t0 + t3;                               \
	x3 = t0 - t3;                               \
	x1 = t1 + t2;                               \
	x2 = t1 - t2;                               \
	t0 = s7;                                    \
	t1 = s5;                                    \
	t2 = s3;                                    \
	t3 = s1;                                    \
	p3 = t0 + t2;                               \
	p4 = t1 + t3;                               \
	p1 = t0 + t3;                               \
	p2 = t1 + t2;                               \
	p5 = (p3 + p4)*f2f(1.175875602f);           \
	t0 = t0*f2f(0.298631336f);                  \
	t1 = t1*f2f(2.053119869f);                  \
	t2 = t2*f2f(3.072711026f);                  \
	t3 = t3*f2f(1.501321110f);                  \
	p1 = p5 + p1*f2f(-0.899976223f);            \
	p2 = p5 + p2*f2f(-2.562915447f);            \
	p3 = p3*f2f(-1.961570560f);                 \
	p4 = p4*f2f(-0.390180644f);                 \
	t3 += p1 + p4;                              \
	t2 += p2 + p3;                              \
	t1 += p2 + p4;                              \
	t0 += p1 + p3;

	static uint8 dezigzag[64 + 15] =
	{
		0, 1, 8, 16, 9, 2, 3, 10,
		17, 24, 32, 25, 18, 11, 4, 5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13, 6, 7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63,
		// let corrupt input sample past end
		63, 63, 63, 63, 63, 63, 63, 63,
		63, 63, 63, 63, 63, 63, 63
	};

	static uint32 bmask[17] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535 };

	inline int Get8(MStream* s)
	{
		if (s->b < s->e)
		{
			return *s->b++;
		}

		return 0;
	}

	inline int Get8U(MStream* s)
	{
		return (uint)Get8(s);
	}

	inline bool GetN(MStream* s, uint8* buf, int n)
	{
		if (s->b + n <= s->e)
		{
			memcpy(buf, s->b, n);
			s->b += n;
			return true;
		}

		return false;
	}

	inline int Get16(MStream* s)
	{
		int z = Get8(s);
		return (z << 8) + Get8(s);
	}

	inline int Get32(MStream* s)
	{
		int z = Get16(s);
		return (z << 16) + Get16(s);
	}

	inline int Get16LE(MStream* s)
	{
		int z = Get8(s);
		return z + (Get8(s) << 8);
	}

	inline int Get32LE(MStream* s)
	{
		int z = Get16LE(s);
		return z + (Get16LE(s) << 16);
	}

	inline bool AtEof(MStream* s)
	{
		return s->b >= s->e;
	}

	inline void Skip(MStream* s, int n)
	{
		s->b += n;
	}

	inline uint8 GetMarker(Jpeg* j)
	{
		uint8 x;
		if (j->marker != MARKET_NONE)
		{ 
			x = j->marker; 
			j->marker = MARKET_NONE; 
			return x; 
		}

		x = Get8U(j->s);
		if (x != 0xff)
		{
			return MARKET_NONE;
		}

		while (x == 0xff)
		{
			x = Get8U(j->s);
		}

		return x;
	}

	inline void Reset(Jpeg* j)
	{
		j->code_bit = 0;
		j->code_buffer = 0;
		j->nomore = 0;
		j->Component[0].dc_pred = 0;
		j->Component[1].dc_pred = 0;
		j->Component[2].dc_pred = 0;
		j->marker = MARKET_NONE;
		j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
	}


	Decoder::Decoder(void)
	{
		lastError = 0;
	}


	Decoder::~Decoder(void)
	{

	}

	bool Decoder::Error(char* str)
	{
		lastError = str;
		return false;
	}

	bool Decoder::BuildHaffman(Haffman* h, int* count)
	{
		assert(h && count);

		int k = 0, code = 0;
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < count[i]; j++)
			{
				h->size[k++] = (uint8)(i + 1);
			}
		}
		h->size[k] = 0;

		k = 0;
		int j = 0;

		for (j = 1; j <= 16; j++)
		{
			h->delta[j] = k - code;
			if (h->size[k] == j)
			{
				while (h->size[k] == j)
				{
					h->code[k++] = (uint16)(code++);
				}

				if (code - 1 >= (1 << j))
				{
					return Error("bad code lenght");
				}
			}

			h->maxcode[j] = code << (16 - j);
			code <<= 1;
		}

		h->maxcode[j] = 0xffffffff;

		memset(h->fast, 255, 1 << FAST_BITS);

		for (int i = 0; i < k; i++)
		{
			int s = h->size[i];

			if (s <= FAST_BITS)
			{
				int c = h->code[i] << (FAST_BITS - s);
				int m = 1 << (FAST_BITS - s);

				for (int j = 0; j < m; j++)
				{
					h->fast[c + j] = (uint8)i;
				}
			}
		}

		return true;
	}

	void Decoder::GrowBufferUnSafe(Jpeg* j)
	{
		do 
		{
			int b = j->nomore ? 0 : Get8(j->s);
			if (b == 0xff)
			{
				int c = Get8(j->s);
				if (c != 0)
				{
					j->marker = (uint8)c;
					j->nomore = 1;
					return;
				}
			}

			j->code_buffer |= b << (24 - j->code_bit);
			j->code_bit += 8;

		} while (j->code_bit <= 24);
	}

	int Decoder::Decode(Jpeg* j, Haffman* h)
	{
		uint temp;
		int c, k;

		if (j->code_bit < 16) GrowBufferUnSafe(j);

		c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
		k = h->fast[c];

		if (k < 255)
		{
			int s = h->size[k];
			if (s > j->code_bit)
			{
				return -1;
			}
			j->code_buffer <<= s;
			j->code_bit -= s;
			return h->values[k];
		}

		temp = j->code_buffer >> 16;
		for (k = FAST_BITS + 1;; k++)
		{
			if (temp < h->maxcode[k]) break;

			if (k == 17)
			{
				j->code_bit -= 16;
				return -1;
			}
		}

		if (k > j->code_bit) return -1;

		c = ((j->code_buffer >> (32 - k)) & bmask[k]) + h->delta[k];
		assert((((j->code_buffer) >> (32 - h->size[c])) & bmask[h->size[c]]) == h->code[c]);

		j->code_bit -= k;
		j->code_buffer <<= k;

		return h->values[c];
	}

	int Decoder::ExtendReceive(Jpeg* j, int n)
	{
		uint m = 1 << (n - 1);
		uint k = 0;

		if (j->code_bit < n) GrowBufferUnSafe(j);

#if 1
		k = _lrotl(j->code_buffer, n);
		j->code_buffer = k & ~bmask[n];
		k &= bmask[n];
		j->code_bit -= n;
#else
		k = (j->code_buffer >> (32 - n)) & bmask[n];
		j->code_bit -= n;
		j->code_buffer <<= n;
#endif

		if (k < m)
		{
			return (-1 << n) + k + 1;
		}
		else
		{
			return k;
		}
	}

	bool Decoder::DecodeBlock(Jpeg* j, short data[64], Haffman* hdc, Haffman* hac, int b)
	{
		int diff, dc, k;
		int t = Decode(j, hdc);
		if (t < 0)
		{
			return Error("bad haffman code");
		}

		memset(data, 0, sizeof(short)* 64);

		diff = t ? ExtendReceive(j, t) : 0;
		dc = j->Component[b].dc_pred + diff;
		j->Component[b].dc_pred = dc;
		data[0] = (short)dc;

		k = 1;
		do 
		{
			int rs = Decode(j, hac);
			if (rs < 0)
			{
				return Error("bad haffman code");
			}
			int s = rs & 15;
			int r = rs >> 4;

			if (s == 0)
			{
				if (rs != 0xf0) break;
				k += 16;
			}
			else
			{
				k += r;
				data[dezigzag[k++]] = (short)ExtendReceive(j, s);
			}
		} while (k < 64);

		return true;
	}

	void Decoder::IDCTBlock(uint8* out, int out_stride, short data[64], dequantize_t* dequantize)
	{
		int val[64], *v = val;
		dequantize_t *dq = dequantize;
		uint8* o;
		short* d = data;

		for (int i = 0; i < 8; i++, d++, dq++, v++)
		{
			if (d[8] == 0 && d[16] == 0 && d[24] == 0 && d[32] == 0 && d[40] == 0 && d[48] == 0 && d[56] == 0)
			{
				int dcterm = d[0] * dq[0] << 2;
				v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
			}
			else
			{
				IDCT_1D(d[0] * dq[0], d[8] * dq[8], d[16] * dq[16], d[24] * dq[24],
					d[32] * dq[32], d[40] * dq[40], d[48] * dq[48], d[56] * dq[56]);
				x0 += 512; x1 += 512; x2 += 512; x3 += 512;
				v[ 0] = (x0 + t3) >> 10;
				v[56] = (x0 - t3) >> 10;
				v[ 8] = (x1 + t2) >> 10;
				v[48] = (x1 - t2) >> 10;
				v[16] = (x2 + t1) >> 10;
				v[40] = (x2 - t1) >> 10;
				v[24] = (x3 + t0) >> 10;
				v[32] = (x3 - t0) >> 10;
			}
		}

		v = val;
		o = out;
		for (int i = 0; i < 8; i++, v += 8, o += out_stride)
		{
			IDCT_1D(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
			x0 += 65536 + (128 << 17);
			x1 += 65536 + (128 << 17);
			x2 += 65536 + (128 << 17);
			x3 += 65536 + (128 << 17);

			// tried computing the shifts into temps, or'ing the temps to see
			// if any were out of range, but that was slower
			o[0] = clamp((x0 + t3) >> 17, 0, 255);
			o[7] = clamp((x0 - t3) >> 17, 0, 255);
			o[1] = clamp((x1 + t2) >> 17, 0, 255);
			o[6] = clamp((x1 - t2) >> 17, 0, 255);
			o[2] = clamp((x2 + t1) >> 17, 0, 255);
			o[5] = clamp((x2 - t1) >> 17, 0, 255);
			o[3] = clamp((x3 + t0) >> 17, 0, 255);
			o[4] = clamp((x3 - t0) >> 17, 0, 255);
		}
	}

	bool Decoder::ParseEntropyCodedData(Jpeg* z)
	{
		Reset(z);

		if (z->scan_n == 1)
		{
#ifdef E_SIMD
			__declspec(align, 16);
#endif
			short data[64];
			int n = z->order[0];
			int w = (z->Component[n].x + 7) >> 3;
			int h = (z->Component[n].y + 7) >> 3;

			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					if (!DecodeBlock(z, data, z->hfdc + z->Component[n].hd, z->hfac + z->Component[n].ha, n))
					{
						return false;
					}
#ifdef E_SIMD
					IDCTInstalled(z->Component[n].data+z->Component[n].w2*i*8+j*8, z->Component[n].w2, data, z->dequant2[z->Component[n].tq]);
#else
					IDCTBlock(z->Component[n].data + z->Component[n].w2*i*8 + j*8, z->Component[n].w2, data, z->dequant[z->Component[n].tq]);
#endif
					if (--z->todo <= 0)
					{
						if (z->code_bit < 24) GrowBufferUnSafe(z);
						if (!RESTART(z->marker)) return true;
						Reset(z);
					}
				}
			}
		}
		else
		{
			short data[64];
			for (int i = 0; i < z->mcu_y; i++)
			{
				for (int j = 0; j < z->mcu_x; j++)
				{
					for (int k = 0; k < z->scan_n; k++)
					{
						int n = z->order[k];

						for (int y = 0; y < z->Component[n].v; y++)
						{
							for (int x = 0; x < z->Component[n].h; x++)
							{
								int x2 = (z->Component[n].h * j + x) * 8;
								int y2 = (z->Component[n].v * i + y) * 8;
								if (!DecodeBlock(z, data, z->hfdc + z->Component[n].hd, z->hfac + z->Component[n].ha, n))
								{
									return false;
								}
#ifdef E_SIMD
								IDCTInstalled(z->Component[n].data+z->Component[n].w2*y2+x2, z->Component[n].w2, data, z->dequant2[z->Component[n].tq]);
#else
								IDCTBlock(z->Component[n].data + z->Component[n].w2*y2+x2, z->Component[n].w2, data, z->dequant[z->Component[n].tq]);
#endif

							}
						}
					}

					if (--z->todo <= 0)
					{
						if (z->code_bit < 24) GrowBufferUnSafe(z);
						if (!RESTART(z->marker)) return true;
						Reset(z);
					}
				}
			}
		}

		return true;
	}

	bool Decoder::ProcessMarket(Jpeg* j, int m)
	{
		int L;
		switch (m)
		{
		case MARKET_NONE:
			return Error("expected market");
		case 0xc2:
			return Error("progressive jpeg");
		case 0xdd:
			if (Get16(j->s) != 4)
			{
				return Error("bad DRI len");
			}
			j->restart_interval = Get16(j->s);
			return true;
		case 0xdb:
			L = Get16(j->s) - 2;
			while (L > 0)
			{
				int q = Get8(j->s);
				int p = q >> 4;
				int t = q & 15;
				if (p != 0)
				{ 
					return Error("bad DQT type");
				}
				if (t > 3)
				{
					return Error("bad DQT table");
				}
				for (int i = 0; i < 64; i++)
				{
					j->dequant[t][dezigzag[i]] = Get8U(j->s);
				}
#ifdef E_SIMD
				for (int j=0; j<64; j++)
				{
					j->dequant2[t][j] = t->dequant[t][j];
				}
#endif
				L -= 65;
			}
			return L == 0;
		case 0xc4:
			L = Get16(j->s) - 2;
			while (L > 0)
			{
				uint8* v;
				int sizes[16], n = 0;
				int q = Get8(j->s);
				int tc = q >> 4;
				int th = q & 15;
				if (tc > 1 || th > 3)
				{
					return Error("bad DHT header");
				}

				for (int i = 0; i < 16; i++)
				{
					sizes[i] = Get8(j->s);
					n += sizes[i];
				}

				L -= 17;
				if (tc == 0)
				{
					if (!BuildHaffman(j->hfdc + th, sizes)) return false;
					v = j->hfdc[th].values;
				}
				else
				{
					if (!BuildHaffman(j->hfac + th, sizes)) return false;
					v = j->hfac[th].values;
				}

				for (int i = 0; i < n; i++)
				{
					v[i] = Get8U(j->s);
				}

				L -= n;
			}
			
			return L == 0;
		}

		if ((m >= 0xe0 && m <= 0xef) || m == 0xfe)
		{
			Skip(j->s, Get16(j->s) - 2);

			return true;
		}

		return false;
	}

	bool Decoder::ProcessScanHeader(Jpeg* j)
	{
		int ls = Get16(j->s);
		j->scan_n = Get8(j->s);
		if (j->scan_n < 1 || j->scan_n > 4 || j->scan_n > (int)j->s->img_n)
		{
			return Error("bad SOS componet count");
		}
		if (ls != 6 + 2 * j->scan_n)
		{
			return Error("bad SOS len");
		}

		for (int i = 0; i < j->scan_n; i++)
		{
			int id = Get8(j->s);
			int q = Get8(j->s);
			int k = 0;
			for (k = 0; k < j->s->img_n; k++)
			{
				if (j->Component[k].id == id) break;
			}

			if (k == j->s->img_n) return false;
			j->Component[k].hd = q >> 4;
			if (j->Component[k].hd > 3) return Error("bad DC haffman");
			j->Component[k].ha = q & 15;
			if (j->Component[k].ha > 3) return Error("bad AC haffman");
			j->order[i] = k;
		}

		if (Get8(j->s) != 0) return Error("bad SOS");
		Get8(j->s);
		if (Get8(j->s) != 0) return Error("bad SOS");

		return true;
	}

	bool Decoder::ProcessFrameHeader(Jpeg* j, int scan)
	{
		MStream* s = j->s;
		int lf, p, q, h_max=1, v_max=1, c;
		lf = Get16(s); if (lf < 11) return Error("bad SOF len");
		p = Get8(s); if (p != 8) return Error("only 8-bits");
		s->img_y = Get16(s); if (s->img_y == 0) return Error("no header height");
		s->img_x = Get16(s); if (s->img_x == 0) return Error("0 width");
		c = Get8(s);
		if (c != 3 && c != 1) return Error("bad componet count");
		s->img_n = c;

		for (int i = 0; i < c; i++)
		{
			j->Component[i].data = 0;
			j->Component[i].linebuf = 0;
		}

		if (lf != 8 + 3 * s->img_n) return Error("bad SOF len");

		for (int i = 0; i < s->img_n; i++)
		{
			j->Component[i].id = Get8(j->s);
			if (j->Component[i].id != i+1)
				if (j->Component[i].id != i)
					return Error("bad Component ID");
			q = Get8(s);
			j->Component[i].h = (q >> 4);
			if (!j->Component[i].h || j->Component[i].h > 4) return Error("bad H");
			j->Component[i].v = (q & 15);
			if (!j->Component[i].v || j->Component[i].v > 4) return Error("bad V");
			j->Component[i].tq = Get8(s);
			if (j->Component[i].tq > 3) return Error("bad TQ");
		}

		if (scan != 0) return true;

		if ((1 << 30) / s->img_x / s->img_n < s->img_y) return Error("too large");

		for (int i = 0; i < s->img_n; i++)
		{
			if (j->Component[i].h > h_max) h_max = j->Component[i].h;
			if (j->Component[i].v > v_max) v_max = j->Component[i].v;
		}

		j->h_max = h_max;
		j->v_max = v_max;
		j->mcu_w = h_max * 8;
		j->mcu_h = v_max * 8;
		j->mcu_x = (s->img_x + j->mcu_w - 1) / j->mcu_w;
		j->mcu_y = (s->img_y + j->mcu_h - 1) / j->mcu_h;

		for (int i = 0; i < s->img_n; i++)
		{
			j->Component[i].x = (s->img_x * j->Component[i].h + h_max - 1) / h_max;
			j->Component[i].y = (s->img_y * j->Component[i].v + v_max - 1) / v_max;
			j->Component[i].w2 = j->mcu_x * j->Component[i].h * 8;
			j->Component[i].h2 = j->mcu_y * j->Component[i].v * 8;
			j->Component[i].raw_data = malloc(j->Component[i].w2 * j->Component[i].h2 + 15);
			if (j->Component[i].raw_data == 0)
			{
				for (--i; i >= 0; i--)
				{
					free(j->Component[i].raw_data);
					j->Component[i].raw_data = 0;
				}

				return Error("out of memory");
			}

			j->Component[i].data = (uint8*)(((size_t)j->Component[i].raw_data + 15) & ~15);
			j->Component[i].linebuf = 0;
		}

		return true;
	}

#define DNL(x) ((x) == 0xdc)
#define SOI(x) ((x) == 0xd8)
#define EOI(x) ((x) == 0xd9)
#define SOF(x) ((x) == 0xc0 || (x) == 0xc1)
#define SOS(x) ((x) == 0xda)

	bool Decoder::DecodeJpegHeader(Jpeg* j, int scan)
	{
		j->marker = MARKET_NONE;
		int m = GetMarker(j);
		if (!SOI(m)) return Error("no SOI");
		if (scan == 1) return true;
		m = GetMarker(j);

		while (!SOF(m))
		{
			if (!ProcessMarket(j, m))
			{
				return false;
			}

			m = GetMarker(j);
			while (m == MARKET_NONE)
			{
				if (AtEof(j->s))
				{
					return Error("no SOF");
				}

				m = GetMarker(j);
			}
		}

		if (!ProcessFrameHeader(j, scan))
		{
			return false;
		}

		return true;
	}

	bool Decoder::DecodeJpegImage(Jpeg* j)
	{
		j->restart_interval = 0;

		if (!DecodeJpegHeader(j, 0))
		{
			return false;
		}

		int m = GetMarker(j);

		while (!EOI(m))
		{
			if (SOS(m))
			{
				if (!ProcessScanHeader(j))
				{
					return false;
				}

				if (!ParseEntropyCodedData(j))
				{
					return false;
				}

				if (j->marker == MARKET_NONE)
				{
					while (!AtEof(j->s))
					{
						int x = Get8(j->s);
						if (x == 255)
						{
							j->marker = Get8(j->s);
							break;
						}
						else if (x != 0)
						{
							return false;
						}
					}
				}
			}
			else
			{
				if (!ProcessMarket(j, m))
				{
					return false;
				}
			}

			m = GetMarker(j);
		}

		return true;
	}

	void Decoder::CleanJpeg(Jpeg* j)
	{
		for (int i = 0; i < j->s->img_n; i++)
		{
			if (j->Component[i].data)
			{
				free(j->Component[i].raw_data);
				j->Component[i].data = 0;
			}

			if (j->Component[i].linebuf)
			{
				free(j->Component[i].linebuf);
				j->Component[i].linebuf = 0;
			}
		}
	}

#define float2fixed(x)  ((int) ((x) * 65536 + 0.5))

	void Decoder::YCbCr2RGB(uint8* out, const uint8* y, const uint8* pcb, const uint8* pcr, int count, int step)
	{
		for (int i = 0; i < count; i++)
		{
			int y_fixed = (y[i] << 16) + 32768;
			int cr = pcr[i] - 128;
			int cb = pcb[i] - 128;
			int r = y_fixed + cr * float2fixed(1.40200f);
			int g = y_fixed - cr * float2fixed(0.71414f) - cb * float2fixed(0.34414f);
			int b = y_fixed + cb * float2fixed(1.77200f);
			r >>= 16;
			g >>= 16;
			b >>= 16;

			out[0] = clamp(r, 0, 255);
			out[1] = clamp(g, 0, 255);
			out[2] = clamp(b, 0, 255);
			out[3] = 255;
			out += step;
		}
	}

	uint8* Decoder::LoadJpegImage(Jpeg* j, int* out_x, int* out_y, int* comp, int req_comp)
	{
		if (req_comp < 0 || req_comp>4)
		{
			return 0;
		}

		if (!DecodeJpegImage(j))
		{
			CleanJpeg(j);
			return 0;
		}

		int n = req_comp ? req_comp : j->s->img_n;
		int decode_n = 0;
		if (j->s->img_n == 3 && n < 3)
		{
			decode_n = 1;
		}
		else
		{
			decode_n = j->s->img_n;
		}

		uint8* output;
		uint8* coutput[4];
		Resample resample[4];

		for (int k = 0; k < decode_n; k++)
		{
			Resample & r = resample[k];
			j->Component[k].linebuf = (uint8*)malloc(j->s->img_x + 3);
			if (j->Component[k].linebuf == 0)
			{
				Error("out of memory");
				CleanJpeg(j);
				return 0;
			}

			r.hs = j->h_max / j->Component[k].h;
			r.vs = j->v_max / j->Component[k].v;
			r.ystep = r.vs >> 1;
			r.w_lores = (j->s->img_x + r.hs - 1) / r.hs;
			r.ypos = 0;
			r.line0 = r.line1 = j->Component[k].data;

			if (r.hs == 1 && r.vs == 1)
			{
				r.BindHandle(0);
			}
			else if (r.hs == 1 && r.vs == 2)
			{
				r.BindHandle(1);
			}
			else if (r.hs == 2 && r.vs == 1)
			{
				r.BindHandle(2);
			}
			else if (r.hs == 2 && r.vs == 2)
			{
				r.BindHandle(3);
			}
			else
			{
				r.BindHandle(4);
			}
		}

		output = (uint8*)malloc(n * j->s->img_x * j->s->img_y + 1);
		if (output == 0)
		{
			Error("out of memory");
			CleanJpeg(j);
			return 0;
		}

		for (uint32 i = 0; i < j->s->img_y; i++)
		{
			uint8* out = output + n * j->s->img_x * i;
			for (int k = 0; k < decode_n; k++)
			{
				Resample & r = resample[k];
				int y_bot = r.ystep >= (r.vs >> 1);

				coutput[k] = r(j->Component[k].linebuf
					, y_bot ? r.line1 : r.line0
					, y_bot ? r.line0 : r.line1
					, r.w_lores
					, r.hs);

				if (++r.ystep >= r.vs)
				{
					r.ystep = 0;
					r.line0 = r.line1;

					if (++r.ypos < j->Component[k].y)
					{
						r.line0 += j->Component[k].w2;
					}
				}
			}

			if (n >= 3)
			{
				uint8* y = coutput[0];
				if (j->s->img_n == 3)
				{
#ifdef E_SIMD
					YCbCrInstalled(out, y, coutput[1], coutput[2], j->s->img_x, n);
#else
					YCbCr2RGB(out, y, coutput[1], coutput[2], j->s->img_x, n);
#endif
				}
				else
				{
					for (uint32 i = 0; i < j->s->img_x; i++)
					{
						out[0] = out[1] = out[2] = y[i];
						out[3] = 255;
						out += n;
					}
				}
			}
			else
			{
				uint8* y = coutput[0];
				if (n == 1)
				{
					for (uint32 i = 0; i < j->s->img_x; i++)
					{
						out[i] = y[i];
					}
				}
				else
				{
					for (uint32 i = 0; i < j->s->img_x; i++)
					{
						*out++ = y[i];
						*out++ = 255;
					}
				}
			}
		}

		CleanJpeg(j);

		*out_x = j->s->img_x;
		*out_y = j->s->img_y;

		if (comp)
		{
			*comp = j->s->img_n;
		}

		return output;
	}

	uint8* Decoder::LoadJpegImage(uint8* buffer, int len, int* out_x, int* out_y, int* comp, int req_comp)
	{
		MStream s;
		Jpeg j;

		s.b = s.o = buffer;
		s.e = s.b + len;
		s.l = len;

		j.s = &s;

		return LoadJpegImage(&j, out_x, out_y, comp, req_comp);
	}
}
