#ifndef __CORE_BASECLASS_H__
#define __CORE_BASECLASS_H__
#include "Defines.h"

namespace e
{
	struct MStream
	{
		size_t l;//len of buffer
		uint8* b;//buffer
		uint8* o;//start of buffer
		uint8* e;//end of buffer
		uint32 img_x, img_y;
		int img_n, img_out_n;
	};

#define FAST_BITS 9
	struct Haffman
	{
		uint8 fast[1 << FAST_BITS];
		uint16 code[256];
		uint8 values[256];
		uint8 size[257];
		uint maxcode[18];
		int delta[17];
	};

	struct Jpeg
	{
#ifdef E_SIMD
		ushort dequant2[4][64];
#endif
		MStream* s;
		Haffman hfdc[4];
		Haffman hfac[4];
		uint8 dequant[4][64];

		int h_max, v_max;
		int mcu_x, mcu_y;
		int mcu_w, mcu_h;

		struct
		{
			int id;
			int h, v;
			int tq;
			int hd, ha;
			int dc_pred;
			int x, y, w2, h2;
			uint8* data;
			void* raw_data;
			uint8* linebuf;
		}Component[4];

		uint32 code_buffer;
		int code_bit;
		uint8 marker;
		int nomore;

		int scan_n, order[4];
		int restart_interval, todo;
	};
}

#endif