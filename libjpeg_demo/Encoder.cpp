#include "private.h"
#include "Encoder.h"
#include <assert.h>

#define FOUR_ZERO_ZERO	0
#define FOUR_TWO_TWO	2

namespace e
{
	static uint8 zigzag_table[] = {
		0, 1, 5, 6, 14, 15, 27, 28, 
		2, 4, 7, 13, 16, 26, 29, 42, 
		3, 8, 12, 17, 25, 30, 41, 43, 
		9, 11, 18, 24, 31, 40, 44, 53, 
		10, 19, 23, 32, 39, 45, 52, 54, 
		20, 22, 33, 38, 46, 51, 55, 60, 
		21, 34, 37, 47, 50, 56, 59, 61, 
		35, 36, 48, 49, 57, 58, 62, 63
	};

	static uint16 marketdata[] = {
		// dht
		0xFFC4, 0x01A2, 0x00,
		// luminance dc (2 - 16) + 1
		0x0105, 0x0101, 0x00101, 0x0101, 0x0000, 0x00000, 00000, 00000,
		// luminance dc (2 - 12) + 1
		0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B01,
		// chrominance dc (1 - 16)
		0x0003, 0x0101, 0x0101, 0x0101, 0x0101, 0x0100, 0x0000, 0x0000,
		// chrominance dc (1 - 12)
		0x0001, 0x00203, 0x0405, 0x0607, 0x0809, 0x00A0B,
		// luminance ac 1 + (1 - 15)
		0x1000, 0x0201, 0x0303, 0x0204, 0x0305, 0x0504, 0x0400, 0x0001,
		// luminance ac 1 + (1 - 162) + 1
		0x7D01, 0x0203, 0x0004, 0x1105, 0x1221, 0x3141, 0x0613, 0x5161, 0x0722,
		0x7114, 0x3281, 0x91A1, 0x0823, 0x42B1, 0xC115, 0x52D1, 0xF024, 0x3362,
		0x7282, 0x090A, 0x1617, 0x1819, 0x1A25, 0x2627, 0x2829, 0x2A34, 0x3536,
		0x3738, 0x393A, 0x4344, 0x4546, 0x4748, 0x494A, 0x5354, 0x5556, 0x5758,
		0x595A, 0x6364, 0x6566, 0x6768, 0x696A, 0x7374, 0x7576, 0x7778, 0x797A,
		0x8384, 0x8586, 0x8788, 0x898A, 0x9293, 0x9495, 0x9697, 0x9899, 0x9AA2,
		0xA3A4, 0xA5A6, 0xA7A8, 0xA9AA, 0xB2B3, 0xB4B5, 0xB6B7, 0xB8B9, 0xBAC2,
		0xC3C4, 0xC5C6, 0xC7C8, 0xC9CA, 0xD2D3, 0xD4D5, 0xD6D7, 0xD8D9, 0xDAE1,
		0xE2E3, 0xE4E5, 0xE6E7, 0xE8E9, 0xEAF1, 0xF2F3, 0xF4F5, 0xF6F7, 0xF8F9,
		0xFA11,
		// chrominance ac (1 - 16)
		0x0002, 0x0102, 0x0404, 0x0304, 0x0705, 0x0404, 0x0001, 0x0277,
		// chrominance ac (1 - 162)
		0x0001, 0x0203, 0x1104, 0x0521, 0x3106, 0x1241, 0x5107, 0x6171, 0x1322,
		0x3281, 0x0814, 0x4291, 0xA1B1, 0xC109, 0x2333, 0x52F0, 0x1562, 0x72D1,
		0x0A16, 0x2434, 0xE125, 0xF117, 0x1819, 0x1A26, 0x2728, 0x292A, 0x3536,
		0x3738, 0x393A, 0x4344, 0x4546, 0x4748, 0x494A, 0x5354, 0x5556, 0x5758,
		0x595A, 0x6364, 0x6566, 0x6768, 0x696A, 0x7374, 0x7576, 0x7778, 0x797A,
		0x8283, 0x8485, 0x8687, 0x8889, 0x8A92, 0x9394, 0x9596, 0x9798, 0x999A,
		0xA2A3, 0xA4A5, 0xA6A7, 0xA8A9, 0xAAB2, 0xB3B4, 0xB5B6, 0xB7B8, 0xB9BA,
		0xC2C3, 0xC4C5, 0xC6C7, 0xC8C9, 0xCAD2, 0xD3D4, 0xD5D6, 0xD7D8, 0xD9DA,
		0xE2E3, 0xE4E5, 0xE6E7, 0xE8E9, 0xEAF2, 0xF3F4, 0xF5F6, 0xF7F8, 0xF9FA
	};

	static uint8 bitsize[] = {
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};

	static uint16 luminance_dc_code_table[] = {
		0x0000, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006,
		0x000E, 0x001E, 0x003E, 0x007E, 0x00FE, 0x01FE
	};

	static uint16 luminance_dc_size_table[] = {
		0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
		0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009
	};

	static uint16 chrominance_dc_code_table[] = {
		0x0000, 0x0001, 0x0002, 0x0006, 0x000E, 0x001E, 
		0x003E, 0x007E, 0x00FE, 0x01FE, 0x03FE, 0x07FE
	};

	static uint16 chrominance_dc_size_table[] = {
		0x0002, 0x0002, 0x0002, 0x0003, 0x0004, 0x0005,
		0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B
	};

	static uint16 luminance_ac_code_table[] = {
		0x000A, 0x0000, 0x0001, 0x0004, 0x000B, 0x001A, 0x0078, 0x00F8, 0x03F6,
		0xFF82, 0xFF83, 0x000C, 0x001B, 0x0079, 0x01F6, 0x07F6, 0xFF84, 0xFF85,
		0xFF86, 0xFF87, 0xFF88, 0x001C, 0x00F9, 0x03F7, 0x0FF4, 0xFF89, 0xFF8A,
		0xFF8b, 0xFF8C, 0xFF8D, 0xFF8E, 0x003A, 0x01F7, 0x0FF5, 0xFF8F, 0xFF90,
		0xFF91, 0xFF92, 0xFF93, 0xFF94, 0xFF95, 0x003B, 0x03F8, 0xFF96, 0xFF97,
		0xFF98, 0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D, 0x007A, 0x07F7, 0xFF9E,
		0xFF9F, 0xFFA0, 0xFFA1, 0xFFA2, 0xFFA3, 0xFFA4, 0xFFA5, 0x007B, 0x0FF6,
		0xFFA6, 0xFFA7, 0xFFA8, 0xFFA9, 0xFFAA, 0xFFAB, 0xFFAC, 0xFFAD, 0x00FA,
		0x0FF7, 0xFFAE, 0xFFAF, 0xFFB0, 0xFFB1, 0xFFB2, 0xFFB3, 0xFFB4, 0xFFB5,
		0x01F8, 0x7FC0, 0xFFB6, 0xFFB7, 0xFFB8, 0xFFB9, 0xFFBA, 0xFFBB, 0xFFBC,
		0xFFBD, 0x01F9, 0xFFBE, 0xFFBF, 0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3, 0xFFC4,
		0xFFC5, 0xFFC6, 0x01FA, 0xFFC7, 0xFFC8, 0xFFC9, 0xFFCA, 0xFFCB, 0xFFCC,
		0xFFCD, 0xFFCE, 0xFFCF, 0x03F9, 0xFFD0, 0xFFD1, 0xFFD2, 0xFFD3, 0xFFD4,
		0xFFD5, 0xFFD6, 0xFFD7, 0xFFD8, 0x03FA, 0xFFD9, 0xFFDA, 0xFFDB, 0xFFDC,
		0xFFDD, 0xFFDE, 0xFFDF, 0xFFE0, 0xFFE1, 0x07F8, 0xFFE2, 0xFFE3, 0xFFE4,
		0xFFE5, 0xFFE6, 0xFFE7, 0xFFE8, 0xFFE9, 0xFFEA, 0xFFEB, 0xFFEC, 0xFFED,
		0xFFEE, 0xFFEF, 0xFFF0, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4, 0xFFF5, 0xFFF6,
		0xFFF7, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE, 0x07F9
	};

	static uint16 luminance_ac_size_table[] = {
		0x0004, 0x0002, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000A,
		0x0010, 0x0010, 0x0004, 0x0005, 0x0007, 0x0009, 0x000B, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0005, 0x0008, 0x000A, 0x000C, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0006, 0x0009, 0x000C, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0006, 0x000A, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0007, 0x000B, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0007, 0x000C,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0008,
		0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0009, 0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x000A, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x000B, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x000B
	};

	static uint16 chrominance_ac_code_table[] = {
		0x0000, 0x0001, 0x0004, 0x000A, 0x0018, 0x0019, 0x0038, 0x0078, 0x01F4,
		0x03F6, 0x0FF4, 0x000B, 0x0039, 0x00F6, 0x01F5, 0x07F6, 0x0FF5, 0xFF88,
		0xFF89, 0xFF8A, 0xFF8B, 0x001A, 0x00F7, 0x03F7, 0x0FF6, 0x7FC2, 0xFF8C,
		0xFF8D, 0xFF8E, 0xFF8F, 0xFF90, 0x001B, 0x00F8, 0x03F8, 0x0FF7, 0xFF91,
		0xFF92, 0xFF93, 0xFF94, 0xFF95, 0xFF96, 0x003A, 0x01F6, 0xFF97, 0xFF98,
		0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D, 0xFF9E, 0x003B, 0x03F9, 0xFF9F,
		0xFFA0, 0xFFA1, 0xFFA2, 0xFFA3, 0xFFA4, 0xFFA5, 0xFFA6, 0x0079, 0x07F7,
		0xFFA7, 0xFFA8, 0xFFA9, 0xFFAA, 0xFFAB, 0xFFAC, 0xFFAD, 0xFFAE, 0x007A,
		0x07F8, 0xFFAF, 0xFFB0, 0xFFB1, 0xFFB2, 0xFFB3, 0xFFB4, 0xFFB5, 0xFFB6,
		0x00F9, 0xFFB7, 0xFFB8, 0xFFB9, 0xFFBA, 0xFFBB, 0xFFBC, 0xFFBD, 0xFFBE,
		0xFFBF, 0x01F7, 0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3, 0xFFC4, 0xFFC5, 0xFFC6,
		0xFFC7, 0xFFC8, 0x01F8, 0xFFC9, 0xFFCA, 0xFFCB, 0xFFCC, 0xFFCD, 0xFFCE,
		0xFFCF, 0xFFD0, 0xFFD1, 0x01F9, 0xFFD2, 0xFFD3, 0xFFD4, 0xFFD5, 0xFFD6,
		0xFFD7, 0xFFD8, 0xFFD9, 0xFFDA, 0x01FA, 0xFFDB, 0xFFDC, 0xFFDD, 0xFFDE,
		0xFFDF, 0xFFE0, 0xFFE1, 0xFFE2, 0xFFE3, 0x07F9, 0xFFE4, 0xFFE5, 0xFFE6,
		0xFFE7, 0xFFE8, 0xFFE9, 0xFFEA, 0xFFEb, 0xFFEC, 0x3FE0, 0xFFED, 0xFFEE,
		0xFFEF, 0xFFF0, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4, 0xFFF5, 0x7FC3, 0xFFF6,
		0xFFF7, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE, 0x03FA
	};

	static uint16 chrominance_ac_size_table[] = {
		0x0002, 0x0002, 0x0003, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007, 0x0009,
		0x000A, 0x000C, 0x0004, 0x0006, 0x0008, 0x0009, 0x000B, 0x000C, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0005, 0x0008, 0x000A, 0x000C, 0x000F, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0005, 0x0008, 0x000A, 0x000C, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0006, 0x0009, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0006, 0x000A, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0007, 0x000B,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0007,
		0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0008, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x000B, 0x0010, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x000E, 0x0010, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x000F, 0x0010,
		0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x000A
	};

	uint16 DSP_Division(uint32 numer, uint32 denom)
	{
		denom <<= 15;
		for (int i = 16; i > 0; i--)
		{
			if (numer > denom)
			{
				numer -= denom;
				numer <<= 1;
				numer++;
			}
			else
			{
				numer <<= 1;
			}
		}

		return (uint16)(numer);
	}

	Encoder::Encoder()
	{
		fnReadFormat = 0;
		lcode = 0;
		bitindex = 0;
	}

	Encoder::~Encoder()
	{

	}

	uint8* Encoder::WriteMarkers(uint8* out, uint32 format, uint32 width, uint32 height)
	{
		uint16 header_lenght;
		uint8 number_of_components;

		//Start of image market
		*out++ = 0xff;
		*out++ = 0xd8;

		//Quantization table marker
		*out++ = 0xff;
		*out++ = 0xdb;

		//Quantization table lenght
		*out++ = 0x00;
		*out++ = 0x84;

		//PQ,TQ
		*out++ = 0x00;

		//LQT table
		for (int i = 0; i < 64; i++)
		{
			*out++ = LQT[i];
		}

		//PQ,TQ
		*out++ = 0x01;

		//CQT table
		for (int i = 0; i < 64; i++)
		{
			*out++ = CQT[i];
		}

		//haffman table (DHT)
		for (int i = 0; i < 210; i++)
		{
			*out++ = (uint8)(marketdata[i] >> 8);
			*out++ = (uint8)(marketdata[i]);
		}

		//FOUR_ZERO_ZERO
		number_of_components = 3;

		//frame header(SOF)

		//Start of frame marker
		*out++ = 0xff;
		*out++ = 0xc0;
		header_lenght = (uint16)(8 + 3 * number_of_components);

		//frame lenght
		*out++ = (uint8)(header_lenght >> 8);
		*out++ = (uint8)(header_lenght);

		//precision(P)
		*out++ = 0x08;

		//image height
		*out++ = (uint8)(height >> 8);
		*out++ = (uint8)(height);

		//image width
		*out++ = (uint8)(width >> 8);
		*out++ = (uint8)(width);

		//Nf
		*out++ = number_of_components;

		//FOUR_ZERO_ZERO
		*out++ = 0x01;
		*out++ = 0x21;
		*out++ = 0x00;
		*out++ = 0x02;
		*out++ = 0x11;
		*out++ = 0x01;
		*out++ = 0x03;
		*out++ = 0x11;
		*out++ = 0x01;

		//Scan header(SOS)

		//Start of scan marker
		*out++ = 0xff;
		*out++ = 0xda;
		header_lenght = (uint16)(6 + (number_of_components << 1));

		//scan header lenght
		*out++ = (uint8)(header_lenght >> 8);
		*out++ = (uint8)(header_lenght);

		//Ns
		*out++ = number_of_components;
		//FOUR_ZERO_ZERO
		*out++ = 0x01;
		*out++ = 0x00;
		*out++ = 0x02;
		*out++ = 0x11;
		*out++ = 0x03;
		*out++ = 0x11;

		*out++ = 0x00;
		*out++ = 0x3f;
		*out++ = 0x00;

		return out;
	}

	void Encoder::ReadYUV422(JpegEncoderStruct* jes, uint8* input)
	{
		uint16 y1_cols, y2_cols;
		int16* y1_ptr = Y1;
		int16* y2_ptr = Y2;
		int16* cb_ptr = CB;
		int16* cr_ptr = CR;
		uint16 rows = jes->rows;
		uint16 cols = jes->cols;
		uint16 incr = jes->incr;

		if (cols <= 8)
		{
			y1_cols = cols;
			y2_cols = 0;
		}
		else
		{
			y1_cols = 8;
			y2_cols = (uint16)(cols - 8);
		}

		for (int i = rows; i > 0; i--)
		{
			for (int j = y1_cols>>1; j > 0; j--)
			{
				*y1_ptr++ = *input++ - 128;
				*cb_ptr++ = *input++ - 128;
				*y1_ptr++ = *input++ - 128;
				*cr_ptr++ = *input++ - 128;
			}

			for (int j = y2_cols >> 1; j > 0; j--)
			{
				*y2_ptr++ = *input++ - 128;
				*cb_ptr++ = *input++ - 128;
				*y2_ptr++ = *input++ - 128;
				*cr_ptr++ = *input++ - 128;
			}

			if (cols <= 8)
			{
				for (int j = 8 - y1_cols; j > 0; j--)
				{
					*y1_ptr++ = *(y1_ptr - 1);
				}

				for (int j = 8 - y2_cols; j > 0; j--)
				{
					*y2_ptr++ = *(y1_ptr - 1);
				}
			}
			else
			{
				for (int j = 8 - y2_cols; j > 0; j--)
				{
					*y2_ptr++ = *(y2_ptr - 1);
				}
			}

			for (int j = (16 - cols) >> 1; j > 0; j--)
			{
				*cb_ptr++ = *(cb_ptr - 1);
				*cr_ptr++ = *(cr_ptr - 1);
			}

			input += incr;
		}

		for (int i = 8 - rows; i > 0; i--)
		{
			for (int j = 8; j > 0; j--)
			{
				*y1_ptr++ = *(y1_ptr - 8);
				*y2_ptr++ = *(y2_ptr - 8);
				*cb_ptr++ = *(cb_ptr - 8);
				*cr_ptr++ = *(cr_ptr - 8);
			}
		}
	}

	void Encoder::RGB2YUV422(uint8* output, uint8* input, int width, int height)
	{
		uint8 R, G, B, R1, G1, B1;
		int32 Y, Yp, Cb, Cr;
		uint8* inbuf = input;
		uint32 size = width * height / 2;

		for (uint32 i = size; i > 0; i--)
		{
			B = inbuf[0];
			G = inbuf[1];
			R = inbuf[2];
			B1 = inbuf[3];
			G1 = inbuf[4];
			R1 = inbuf[5];
			inbuf += 6;

			Y = clamp0255((77 * R + 150 * G + 29 * B) >> 8);
			Yp = clamp0255((77 * R1 + 150 * G1 + 29 * B) >> 8);
			Cb = clamp0255(((-43 * R - 85 * G + 128 * B) >> 8) + 128);
			Cr = clamp0255(((128 * R - 107 * G - 21 * B) >> 8) + 128);

			*input++ = (uint8)Y;
			*input++ = (uint8)Cb;
			*input++ = (uint8)Yp;
			*input++ = (uint8)Cr;
		}
	}

	void Encoder::Initialize(JpegEncoderStruct* jes, uint32 format, uint32 width, uint32 height)
	{
		uint16 mcu_width, mcu_height, bytes_per_pixel;

		lcode = 0;
		bitindex = 0;
		jes->mcu_width = mcu_width = 16;
		jes->mcu_height = mcu_height = 8;
		jes->hori_mcus = (uint16)((width + mcu_width - 1) >> 4);
		jes->vert_mcus = (uint16)((height + mcu_height - 1) >> 3);
		bytes_per_pixel = 2;
		fnReadFormat = &Encoder::ReadYUV422;

		jes->rows_in_bottom_mcus = (uint16)(height - (jes->vert_mcus - 1) * mcu_height);
		jes->cols_in_right_mcus = (uint16)(width - (jes->hori_mcus - 1) * mcu_width);
		jes->lenght_minus_mcu_width = (uint16)((width - mcu_width) * bytes_per_pixel);
		jes->lenght_minus_width = (uint16)((width - jes->cols_in_right_mcus) * bytes_per_pixel);
		jes->mcu_width_size = (uint16)(mcu_width * bytes_per_pixel);
		jes->offset = (uint16)((width * (mcu_height - 1) - (mcu_width - jes->cols_in_right_mcus)) * bytes_per_pixel);
		jes->ldc1 = 0;
		jes->ldc2 = 0;
		jes->ldc3 = 0;
	}

	void Encoder::InitializeQuantizationTables(uint32 qualityFactor)
	{
		uint8 luminance_quant_table[] = {
			16, 11, 10, 16, 24, 40, 51, 61,
			12, 12, 14, 19, 26, 58, 60, 55, 
			14, 13, 16, 24, 40, 57, 69, 56, 
			14, 17, 22, 29, 51, 87, 80, 62,
			18, 22, 37, 56, 68, 109, 103, 77,
			24, 35, 55, 64, 81, 104, 113, 92,
			49, 64, 78, 87, 103, 121, 120, 101,
			72, 92, 95, 98, 112, 100, 103, 99
		};

		uint8 chrominance_quant_table[] = {
			17, 18, 24, 47, 99, 99, 99, 99,
			18, 21, 26, 66, 99, 99, 99, 99,
			24, 26, 56, 99, 99, 99, 99, 99,
			47, 66, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99,
			99, 99, 99, 99, 99, 99, 99, 99
		};

		for (int i = 0; i < 64; i++)
		{
			uint16 index = zigzag_table[i];
			uint32 value = luminance_quant_table[i] * qualityFactor;
			value = (value + 0x200) >> 10;

			if (value == 0)
				value = 1;
			else if (value > 255)
				value = 255;

			CQT[index] = value;
			ICQT[index] = DSP_Division(0x8000, value);
		}
	}

	uint8* Encoder::CloseBitStream(uint8* output)
	{
		if (bitindex > 0)
		{
			lcode <<= (32 - bitindex);
			uint16 count = (bitindex + 7) >> 3;
			uint8* ptr = (uint8*)&lcode + 3;

			for (uint16 i = count; i > 0; i--)
			{
				if ((*output++ = *ptr--) == 0xff)
				{
					*output = 0;
				}
			}
		}

		//End of image marker
		*output++ = 0xff;
		*output++ = 0xd9;

		return output;
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

			data[ 0] = (int16)((x4 + x5) >> s1);
			data[32] = (int16)((x4 - x5) >> s1);
			data[16] = (int16)((x8 * c2 + x7 * c6) >> s3);
			data[48] = (int16)((x8 * c6 - x7 * c2) >> s3);
			data[56] = (int16)((x0 * c7 - x1 * c5 + x2 * c3 - x3 * c1) >> s3);
			data[40] = (int16)((x0 * c5 - x1 * c1 + x2 * c7 + x3 * c3) >> s3);
			data[24] = (int16)((x0 * c3 - x1 * c7 - x2 * c1 - x3 * c5) >> s3);
			data[ 8] = (int16)((x0 * c1 + x1 * c3 + x2 * c5 + x3 * c7) >> s3);

			data++;
		}
	}

	void Encoder::Quantization(int16* const data, uint16* const quant_table)
	{
		int32 value;
		for (int i = 63; i > 0; i--)
		{
			value = data[i] * quant_table[i];
			value = (value + 0x4000) >> 15;
			temp[zigzag_table[i]] = (int16)value;
		}
	}

	uint8* Encoder::Haffman(JpegEncoderStruct* jes, uint16 component, uint8* output)
	{
		uint16* dc_code_table = 0;
		uint16* dc_size_table = 0;
		uint16* ac_code_table = 0;
		uint16* ac_size_table = 0;
		int16* temp_ptr, coeff, last_dc;
		uint16 abs_coeff, huffcode, huffsize, runlenght = 0, data_size = 0, index;
		int16 bit_in_next_word;
		uint16 numbits;
		uint32 data;

		temp_ptr = temp;
		coeff = *temp_ptr++;

		if (component == 1)
		{
			dc_code_table = luminance_dc_code_table;
			dc_size_table = luminance_dc_size_table;
			ac_code_table = luminance_ac_code_table;
			ac_size_table = luminance_ac_size_table;
			last_dc = jes->ldc1;
			jes->ldc1 = coeff;
		}
		else
		{
			dc_code_table = chrominance_dc_code_table;
			dc_size_table = chrominance_ac_size_table;
			ac_code_table = chrominance_ac_code_table;
			ac_size_table = chrominance_ac_size_table;

			if (component == 2)
			{
				last_dc = jes->ldc2;
				jes->ldc2 = coeff;
			}
			else
			{
				last_dc = jes->ldc3;
				jes->ldc3 = coeff;
			}
		}

		coeff -= last_dc;
		abs_coeff = coeff < 0 ? -coeff-- : coeff;

		while (abs_coeff != 0)
		{
			abs_coeff >>= 1;
			data_size++;
		}

		huffcode = dc_code_table[data_size];
		huffsize = dc_size_table[data_size];

		coeff &= (1 << data_size) - 1;
		data = (huffcode << data_size) | coeff;
		numbits = huffsize + data_size;

		//PUTBITS
		{
			bit_in_next_word = (int16)(bitindex + numbits - 32);
			if (bit_in_next_word < 0)
			{
				lcode = (lcode << numbits) | data;
				bitindex += numbits;
			}
			else
			{
				lcode = (lcode << (numbits - 32)) | (data >> bit_in_next_word);
				if ((*output++ = (uint8)(lcode >> 24)) == 0xff)
					*output++ = 0;
				if ((*output++ = (uint8)(lcode >> 16)) == 0xff)
					*output++ = 0;
				if ((*output++ = (uint8)(lcode >> 8)) == 0xff)
					*output++ = 0;
				if ((*output++ = (uint8)(lcode)) == 0xff)
					*output++ = 0;

				lcode = data;
				bitindex = bit_in_next_word;
			}
		}

		for (int i = 63; i > 0; i--)
		{
			if ((coeff = *temp_ptr++) != 0)
			{
				while (runlenght > 15)
				{
					runlenght -= 16;
					data = ac_code_table[161];
					numbits = ac_size_table[161];
					//PUTBITS
					{
						bit_in_next_word = (int16)(bitindex + numbits - 32);
						if (bit_in_next_word < 0)
						{
							lcode = (lcode << numbits) | data;
							bitindex += numbits;
						}
						else
						{
							lcode = (lcode << (32 - bitindex)) | (data >> bit_in_next_word);
							if ((*output++ = (uint8)(lcode >> 24)) == 0xff)
								*output++ = 0;
							if ((*output++ = (uint8)(lcode >> 16)) == 0xff)
								*output++ = 0;
							if ((*output++ = (uint8)(lcode >> 8)) == 0xff)
								*output++ = 0;
							if ((*output++ = (uint8)(lcode)) == 0xff)
								*output++ = 0;
							lcode = data;
							bitindex = bit_in_next_word;
						}
					}
				}// end while

				abs_coeff = coeff < 0 ? -coeff-- : coeff;
				if ((abs_coeff >> 8) == 0)
				{
					data_size = bitsize[abs_coeff];
				}
				else
				{
					data_size = bitsize[abs_coeff >> 8] + 8;
				}

				index = runlenght * 10 + data_size;
				huffcode = ac_code_table[index];
				huffsize = ac_size_table[index];
				coeff &= (1 << data_size) - 1;
				data = (huffcode << data_size) | coeff;
				numbits = huffsize + data_size;
				//PUTBITS
				{
					bit_in_next_word = (int16)(bitindex + numbits - 32);
					if (bit_in_next_word < 0)
					{
						lcode = (lcode << numbits) | data;
						bitindex += numbits;
					}
					else
					{
						lcode = (lcode << (32 - numbits)) | (data >> bit_in_next_word);
						if ((*output++ = (uint8)(lcode >> 24)) == 0xff)
							*output++ = 0;
						if ((*output++ = (uint8)(lcode >> 16)) == 0xff)
							*output++ = 0;
						if ((*output++ = (uint8)(lcode >> 8)) == 0xff)
							*output++ = 0;
						if ((*output++ = (uint8)(lcode)) == 0xff)
							*output++ = 0;
						lcode = data;
						bitindex = bit_in_next_word;
					}
				}

				runlenght = 0;
			}
			else
			{
				runlenght++;
			}
		}

		if (runlenght != 0)
		{
			data = ac_code_table[0];
			numbits = ac_size_table[0];
			//PUTBITS
			{
				bit_in_next_word = (int16)(bitindex + numbits - 32);
				if (bit_in_next_word < 0)
				{
					lcode = (lcode << numbits) | data;
					bitindex += numbits;
				}
				else
				{
					lcode = (lcode << (32 - numbits)) | (data >> bit_in_next_word);
					if ((*output++ = (uint8)(lcode >> 24)) == 0xff)
						*output++ = 0;
					if ((*output++ = (uint8)(lcode >> 16)) == 0xff)
						*output++ = 0;
					if ((*output++ = (uint8)(lcode >> 8)) == 0xff)
						*output++ = 0;
					if ((*output++ = (uint8)(lcode)) == 0xff)
						*output++ = 0;
					lcode = data;
					bitindex = bit_in_next_word;
				}
			}
		}

		return output;
	}

	uint8* Encoder::EncodeMCU(JpegEncoderStruct* jes, uint32 format, uint8* output)
	{
		DCT(Y1);
		Quantization(Y1, ILQT);
		output = Haffman(jes, 1, output);

		if (format == FOUR_ZERO_ZERO)
		{
			return output;
		}

		DCT(Y2);
		Quantization(Y2, ILQT);
		output = Haffman(jes, 1, output);
		if (format == FOUR_TWO_TWO) goto chroma;

		DCT(Y3);
		Quantization(Y3, ILQT);
		output = Haffman(jes, 1, output);

		DCT(Y4);
		Quantization(Y4, ILQT);
		output = Haffman(jes, 1, output);

	chroma:
		DCT(CB);
		Quantization(CB, ICQT);
		output = Haffman(jes, 2, output);

		DCT(CR);
		Quantization(CR, ICQT);
		output = Haffman(jes, 3, output);

		return output;
	}

	uint32 Encoder::EncodeImage(uint8* output, uint8* input, uint32 qualityFactor, uint32 format, uint32 width, uint32 height)
	{
		uint8* out = output;

		JpegEncoderStruct jpegEncoderStruct;
		JpegEncoderStruct* jes = &jpegEncoderStruct;
		format = FOUR_TWO_TWO;
		RGB2YUV422(output, input, width, height);

		//initialize JPEG control structure
		Initialize(jes, format, width, height);

		//quantization init
		InitializeQuantizationTables(qualityFactor);

		//Write marker data
		output = WriteMarkers(output, format, width, height);

		for (int i = 1; i <= jes->vert_mcus; i++)
		{
			if (i < jes->vert_mcus)
				jes->rows = jes->mcu_height;
			else
				jes->rows = jes->rows_in_bottom_mcus;

			for (int j = 1; j <= jes->hori_mcus; j++)
			{
				if (j < jes->hori_mcus)
				{
					jes->cols = jes->mcu_width;
					jes->incr = jes->lenght_minus_mcu_width;
				}
				else
				{
					jes->cols = jes->cols_in_right_mcus;
					jes->incr = jes->lenght_minus_width;
				}

				assert(fnReadFormat);
				(this->*fnReadFormat)(jes, input);
				// encode data in MCU
				output = EncodeMCU(jes, format, output);
				input += jes->mcu_width_size;
			}

			input += jes->offset;
		}

		//Close Routine
		output = CloseBitStream(output);

		return (uint32)(output - out);
	}
}

