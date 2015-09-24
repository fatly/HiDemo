#ifndef __CORE_BITMAP_H__
#define __CORE_BITMAP_H__

//#include <windows.h>
#include "Defines.h"
#include "Object.h"
#include "XBitmap.h"

// namespace e
// {
	//-------------------------------------------------------------------------------
	//----------------------device-iddependent-bitmap-------------------------
	//-------------------------------------------------------------------------------
// 	class Bitmap : public RefCountObj
// 	{
// 	public:
// 		Bitmap(void);
// 		Bitmap(const Bitmap &r);
// 		Bitmap(const char* fileName, bool reverse = true);
// 		Bitmap(int width, int height, int bitCount, const uint8* bits = 0, bool init = true);
// 		virtual ~Bitmap(void);
// 		const Bitmap & operator=(const Bitmap & r);
// 		//默认分配是BGRA或者BGR格式
// 		bool Load(const char* fileName, bool reverse = true);
// 		bool Alloc(int width, int height, int bitCount, const uint8* bits = 0, bool init = true);
// 		void SwapChannel(int channel0, int channel1);
// 		void SetColor8(uint8 color = 0x00);
// 		void SetColor24(RGBA color = 0x00000000);//RGBA
// 		void SetColor24(uint8 r, uint8 g, uint8 b);
// 		void SetColor32(RGBA color = 0x00000000);//RGBA
// 		void SetColor32(uint8 r, uint8 g, uint8 b, uint8 a);
// 		int PixelBytes(void) const;
// 		int WidthBytes(void) const;
// 		int Width(void) const;
// 		int Height(void) const;
// 		int Channels(void) const;
// 		uint8* Get(uint x, uint y) const;
// 		bool IsValid(void) const;
// 		bool ExtendAlpha(uint8 alpahValue);
// 		void Reverse(bool vertical = true);
// 		void Swap(Bitmap & r);
// 		void Clear(void);
// 		Bitmap* Clone(void) const;
// 		Bitmap* Clone(uint x0, uint y0, uint x1, uint y1) const;
// 		Bitmap* Clone(int channel) const;
// 		bool Save(const char* fileName, bool reverse = true) const;
// 	private:
// 		void Initialize(void);
// 		void Cleanup(void);
// 		//垂直或水平翻转
// 		void Reverse8(bool vertical);
// 		void Reverse24(bool vertical);
// 		void Reverse32(bool vertical);
// 	public:
// 		//pixel data
// 		uint8*  bits;
// 		uint32  size;
// 		//bitmap file header
// 		uint16  bfType;
// 		uint32  bfSize;
// 		uint16  bfReserved1;
// 		uint16  bfReserved2;
// 		uint32  bfOffBits;
// 		//bitmap info header
// 		uint32  biSize;
// 		uint32  biWidth;
// 		uint32  biHeight;
// 		uint16  biPlanes;
// 		uint16  biBitCount;
// 		uint32  biCompression;
// 		uint32  biSizeImage;
// 		uint32  biXPelsPerMeter;
// 		uint32  biYPelsPerMeter;
// 		uint32  biClrUsed;
// 		uint32  biClrImportant;
// 	};
// 	
// 	//Save DIB to filesystem
// 	//bool SaveBitmap(HBITMAP hBitmap, const char* fileName);
// 
// 	bool Save2PPM(const Bitmap* bitmap, const char* fileName);
//}

namespace e
{
#ifdef INTEGER_CHANNELS
	typedef XBitmap<uint8> Bitmap;
#else
	typedef XBitmap<float> Bitmap;
#endif
}

#endif
