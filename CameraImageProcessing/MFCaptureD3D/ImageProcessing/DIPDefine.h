/*	C語言影像處理函式庫
	Develop by Jimmy HU <s103360021@gmail.com>
	This program is licensed under GNU General Public License v3.
	DIPDefine.h定義影像處理函數所使用之結構、資料型態
 */

//-----全域定義區-----
#define MAX_PATH 256													//	定義檔案路徑最長長度為256字元
#define FILE_ROOT_PATH "\\"
//	定義檔案根目錄路徑位置(用於開啟圖檔)
#define True true														//	定義True為true
#define False false														//	定義False為false
//#define DebugMode														//	定義程式為DebugMode
#define _USE_MATH_DEFINES												//	定義_USE_MATH_DEFINES

//-----全域結構、資料型態宣告區-----
typedef struct BMP24RGB													//	宣告24位元BMP圖檔像素RGB資料結構
{																		//	進入BMP24RGB資料結構
	unsigned char R;													//	宣告R成分變數
	unsigned char G;													//	宣告G成分變數
	unsigned char B;													//	宣告B成分變數
}BMP24RGB;																//	結束BMP24RGB資料結構
typedef struct HSV														//	宣告HSV資料結構
{																		//	進入HSV資料結構
	long double H;														//	宣告H成分變數(H為色相Hue，值域為0～360)
	long double S;														//	宣告S成分變數(S為飽和度Saturation，值域為0～1)
	long double V;														//	宣告V成分變數(V為明度Value，值域為0～255)
}HSV;																	//	結束HSV資料結構
/*	BMPIMAGE結構建立BMP影像物件，該物件包含：
		●檔名(FILENAME)，長度最長為MAX_PATH
		●圖像寬度(XSIZE)
		●圖像高度(YSIZE)
		●填補位元(FILLINGBYTE)，配合BMP圖像資料格式
		●圖像資料(IMAGE_DATA)
 */
typedef struct BMPIMAGE													//	宣告BMPIMAGE資料結構
{																		//	進入BMPIMAGE資料結構
	char FILENAME[MAX_PATH];											//	宣告輸入讀取檔案檔名變數FILENAME
	/*	IMAGE_DATA影像指標可使用陣列方式存取，在一張影像中：
　　　 __________________
　　　 |                |
　　　 |                |
　　　 |                |
　　　 |                |
　　　 |                |
　　　 |                |
　　　 |________________|
　　　  ↑為第一像素 
　　　 	IMAGE_DATA[0]代表第一像素的藍色(B)，由淡到濃分成8位元，0代表沒有藍色，255代表全藍 
　　　 	IMAGE_DATA[1]代表第一像素的綠色(G)，由淡到濃分成8位元，0代表沒有綠色，255代表全綠
　　　 	IMAGE_DATA[2]代表第一像素的紅色(R)，由淡到濃分成8位元，0代表沒有紅色，255代表全紅
　　　 	IMAGE_DATA[3]代表第二像素的藍色(B)，由淡到濃分成8位元，0代表沒有藍色，255代表全藍 
		IMAGE_DATA[4]代表第二像素的綠色(G)，由淡到濃分成8位元，0代表沒有綠色，255代表全綠
		IMAGE_DATA[5]代表第二像素的紅色(R)，由淡到濃分成8位元，0代表沒有紅色，255代表全紅
		但由於BMP檔案格式可能存在填補位元(當影像寬度不為4的倍數時)，IMAGE_DATA陣列的index對應至圖像像素資料有可能不連續，
		以RAWImageToArray將IMAGE_DATA陣列轉換至BMP24RGB型態二維陣列，與二維圖像完全對應
	*/
	unsigned int XSIZE;													//	宣告X軸像素變數
	unsigned int YSIZE;													//	宣告Y軸像素變數
	unsigned char FILLINGBYTE;											//	宣告填充位元組大小
	unsigned char *IMAGE_DATA;											//	宣告影像資料指標*IMAGE_DATA
}BMPIMAGE;																//	結束BMPIMAGE資料結構
typedef struct BMP24RGBIMAGE											//	宣告BMP24RGBIMAGE資料結構
{																		//	進入BMP24RGBIMAGE資料結構
	unsigned int XSIZE;													//	宣告X軸像素變數
	unsigned int YSIZE;													//	宣告Y軸像素變數
	BMP24RGB *IMAGE_DATA;												//	宣告影像資料指標*IMAGE_DATA
}BMP24RGBIMAGE;															//	結束BMP24RGBIMAGE資料結構
typedef struct HSVIMAGE													//	宣告HSVIMAGE資料結構
{																		//	進入HSVIMAGE資料結構
	unsigned int XSIZE;													//	宣告X軸像素變數
	unsigned int YSIZE;													//	宣告Y軸像素變數
	HSV *IMAGE_DATA;													//	宣告影像資料指標*IMAGE_DATA
}HSVIMAGE;																//	結束HSVIMAGE資料結構
/*	HaarWaveletMode集合中宣告Haar濾波模式，說明如下：
		HorizontalHighPass	－	水平方向高通濾波
		HorizontalLowPass	－	水平方向低通濾波
		VerticalHighPass	－	垂直方向高通濾波
		VerticalLowPass		－	垂直方向低通濾波
	各模式計算細節記錄於BMPHaarWavelet副程式宣告處說明
 */
enum HaarWaveletMode													//	宣告HaarWaveletMode集合(用於HaarWavelet副程式)
{																		//	進入HaarWaveletMode集合(用於HaarWavelet副程式)
	HorizontalHighPass,													//	定義Haar小波轉換模式HorizontalHighPass
	HorizontalLowPass,													//	定義Haar小波轉換模式HorizontalLowPass
	VerticalHighPass,													//	定義Haar小波轉換模式VerticalHighPass
	VerticalLowPass,													//	定義Haar小波轉換模式VerticalLowPass
};																		//	結束HaarWaveletMode集合(用於HaarWavelet副程式)
/*	HaarWavelet2Mode集合中宣告二階Haar濾波模式，命名格式說明如下：
		[水平方向濾波方式]+[垂直方向濾波方式]
	即：
		HighHigh	－	
		水平方向高通濾波(呼叫HaarWavelet副程式HorizontalHighPass模式)、垂直方向高通濾波(呼叫HaarWavelet副程式VerticalHighPass模式)
		HighLow		－	
		水平方向高通濾波(呼叫HaarWavelet副程式HorizontalHighPass模式)、垂直方向低通濾波(呼叫HaarWavelet副程式VerticalLowPass模式)
		LowHigh		－	
		水平方向低通濾波(呼叫HaarWavelet副程式HorizontalLowPass模式)、垂直方向高通濾波(呼叫HaarWavelet副程式VerticalHighPass模式)
		LowLow		－	
		水平方向低通濾波(呼叫HaarWavelet副程式HorizontalLowPass模式)、垂直方向低通濾波(呼叫HaarWavelet副程式VerticalLowPass模式)
	並給定二進位冪次方數值，以便選定多重模式(如：HighHigh|HighLow、HighLow|LowHigh)
 */
enum HaarWavelet2Mode													//	宣告HaarWavelet2Mode列舉(用於HaarWavelet2副程式)
{																		//	進入HaarWavelet2Mode列舉(用於HaarWavelet2副程式)
	HighHigh = 1,														//	定義二階Haar小波轉換模式HighHigh
	HighLow = 2,														//	定義二階Haar小波轉換模式HighLow
	LowHigh = 4,														//	定義二階Haar小波轉換模式LowHigh
	LowLow = 8,															//	定義二階Haar小波轉換模式LowLow
};																		//	結束HaarWavelet2Mode列舉(用於HaarWavelet2副程式)

/*	YUVGUID用於YUVtoRGB24函數作為YUV編碼型態參數
	
	GUID					格式描述
	MEDIASUBTYPE_RGB1		2色，每个像素用1位表示，需要调色板
	MEDIASUBTYPE_RGB4		16色，每个像素用4位表示，需要调色板
	MEDIASUBTYPE_RGB8		256色，每个像素用8位表示，需要调色板
	MEDIASUBTYPE_RGB565		每个像素用16位表示，RGB分量分别使用5位、6位、5位
	MEDIASUBTYPE_RGB555		每个像素用16位表示，RGB分量都使用5位（剩下的1位不用）
	MEDIASUBTYPE_RGB24		每个像素用24位表示，RGB分量各使用8位
	MEDIASUBTYPE_RGB32		每个像素用32位表示，RGB分量各使用8位（剩下的8位不用）
	MEDIASUBTYPE_ARGB32		每个像素用32位表示，RGB分量各使用8位（剩下的8位用于表示Alpha通道值）
	MEDIASUBTYPE_YUY2		YUY2格式，以4:2:2方式打包
	MEDIASUBTYPE_YUYV		YUYV格式（实际格式与YUY2相同）
	MEDIASUBTYPE_YVYU		YVYU格式，以4:2:2方式打包
	MEDIASUBTYPE_UYVY		UYVY格式，以4:2:2方式打包
	MEDIASUBTYPE_AYUV		带Alpha通道的4:4:4 YUV格式
	MEDIASUBTYPE_Y41P		Y41P格式，以4:1:1方式打包
	MEDIASUBTYPE_Y411		Y411格式（实际格式与Y41P相同）
	MEDIASUBTYPE_Y211		Y211格式
	MEDIASUBTYPE_IF09		IF09格式
	MEDIASUBTYPE_IYUV		IYUV格式
	MEDIASUBTYPE_YV12		YV12格式
	MEDIASUBTYPE_YVU9		YVU9格式

	Reference：http://blog.csdn.net/zoutian007/article/details/7585511
*/
enum YUVGUID															//	宣告YUVGUID列舉(用於YUV與RGB轉換)
{																		//	進入YUVGUID列舉
	MEDIASUBTYPE_RGB1 = 1,												//	條列YUVGUID內容
	MEDIASUBTYPE_RGB4 = 2,												//	條列YUVGUID內容
	MEDIASUBTYPE_RGB8 = 3,												//	條列YUVGUID內容
	MEDIASUBTYPE_RGB565 = 4,											//	條列YUVGUID內容
	MEDIASUBTYPE_RGB555 = 5,											//	條列YUVGUID內容
	MEDIASUBTYPE_RGB24 = 6,												//	條列YUVGUID內容
	MEDIASUBTYPE_RGB32 = 7,												//	條列YUVGUID內容
	MEDIASUBTYPE_ARGB32 = 8,											//	條列YUVGUID內容
	MEDIASUBTYPE_YUY2 = 9,												//	條列YUVGUID內容
	MEDIASUBTYPE_YUYV = 10,												//	條列YUVGUID內容
	MEDIASUBTYPE_YVYU = 11,												//	條列YUVGUID內容
	MEDIASUBTYPE_UYVY = 12,												//	條列YUVGUID內容
	MEDIASUBTYPE_AYUV = 13,												//	條列YUVGUID內容
	MEDIASUBTYPE_Y41P = 14,												//	條列YUVGUID內容
	MEDIASUBTYPE_Y411 = 15,												//	條列YUVGUID內容
	MEDIASUBTYPE_Y211 = 16,												//	條列YUVGUID內容
	MEDIASUBTYPE_IF09 = 17,												//	條列YUVGUID內容
	MEDIASUBTYPE_IYUV = 18,												//	條列YUVGUID內容
	MEDIASUBTYPE_YV12 = 19,												//	條列YUVGUID內容
	MEDIASUBTYPE_YVU9 = 20,												//	條列YUVGUID內容
};																		//	結束YUVGUID列舉