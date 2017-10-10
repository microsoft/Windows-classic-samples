//-----include header files, 引入標頭檔-----
#include "MFCaptureD3D.h"												//	include "MFCaptureD3D.h" header file, 引入標頭檔MFCaptureD3D.h
#include "BufferLock.h"													//	include "BufferLock.h" header file, 引入標頭檔BufferLock.h
#include <stdio.h>														//	include "stdio.h" header file, 引入標頭檔stdio.h

//-----funtion implementation, 副程式實作-----
__forceinline BYTE Clip(int clr)
{
	return (BYTE)(clr < 0 ? 0 : (clr > 255 ? 255 : clr));
}

__forceinline RGBQUAD ConvertYCrCbToRGB(
	int y,
	int cr,
	int cb
)
{
	RGBQUAD rgbq;

	int c = y - 16;
	int d = cb - 128;
	int e = cr - 128;

	rgbq.rgbRed = Clip((298 * c + 409 * e + 128) >> 8);
	rgbq.rgbGreen = Clip((298 * c - 100 * d - 208 * e + 128) >> 8);
	rgbq.rgbBlue = Clip((298 * c + 516 * d + 128) >> 8);

	return rgbq;
}