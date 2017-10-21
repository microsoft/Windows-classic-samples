/*	數位影像處理主函式
	Develop by Jimmy HU <s103360021@gmail.com>
	This program is licensed under GNU General Public License v3.
*/

//-----include header files, 引入標頭檔-----
#include "DIPDefine.h"													//	引入標頭檔DIPDefine.h
#include <math.h>														//	引入標頭檔math.h
#include <stdbool.h>													//	引入標頭檔stdbool.h
#include <stdio.h>														//	引入標頭檔stdio.h
#include <stdlib.h>														//	引入標頭檔stdlib.h
#include <string.h>														//	引入標頭檔string.h
#include "..\\unistd.h"													//	引入標頭檔unistd.h
#include "Conversion.h"

//-----funtion declaration, 副程式宣告區-----
/*	MainImageProcessing副程式撰寫串流影像主要處理流程，
	副程式輸入為(待處理之RGB影像, 待處理RGB影像之寬度, 待處理RGB影像之高度)
*/
void MainImageProcessing(unsigned char* InputImage, unsigned int ImageSizeX, unsigned int ImageSizeY);


/*  BmpReadFilesize副程式將輸入路徑之圖檔大小讀出並傳回
	副程式輸入為欲讀取大小之圖檔路徑(const char *型態字串，為欲讀取之檔名, FilenameExtension副檔名資訊)
	副程式輸出為圖檔大小(unsigned long型態)
 */
unsigned long BmpReadFilesize(const char *, const bool);				//	宣告BMP圖檔大小(Byte)讀取副程式BmpReadFilesize

/*  BmpReadXSize副程式將輸入路徑之圖檔xsize(寬度)讀出並傳回
	副程式輸入為欲讀取寬度大小之圖檔路徑(const char *型態，為欲讀取之檔名,FilenameExtension副檔名資訊)
	副程式輸出為圖檔寬度(unsigned long型態)
 */
unsigned long BmpReadXSize(const char *, const bool);					//	宣告BMP圖檔xsize(寬度)讀取副程式BmpReadXSize

/*  BmpReadYSize副程式將輸入路徑之圖檔ysize(高度)讀出並傳回
	副程式輸入為欲讀取高度大小之圖檔路徑(const char *型態，為欲讀取之檔名,FilenameExtension副檔名資訊)
	副程式輸出為圖檔高度(unsigned long型態)
 */
unsigned long BmpReadYSize(const char *, const bool);					//	宣告BMP圖檔ysize(高度)讀取副程式BmpReadYSize
/*	BmpRead副程式用於讀取BMP圖檔
	副程式輸入為(欲存放原始圖檔資料之unsigned char型態指標,欲讀取圖檔之寬度,欲讀取圖檔之高度,欲讀取之圖檔路徑,FilenameExtension副檔名資訊)
	副程式輸出：若順利讀取圖檔則傳回0，反之傳回-1
 */
char BmpRead(unsigned char*,const int,const int, const char *, const bool);
//	宣告BmpRead(BMP圖檔讀取)副程式
/*	BmpFileRead副程式整合BmpFillingByteCalc(BMP圖檔填補位元計算)副程式、
	BmpReadFilesize(BMP圖檔檔案大小讀取)副程式、
	BmpReadXSize(BMP圖檔檔案寬度讀取)副程式、
	BmpReadYSize(BMP圖檔檔案高度讀取)副程式與
	BmpRead(BMP圖檔檔案讀取)副程式之功能，用於讀取BMP圖檔
	副程式輸入為(欲讀取之圖檔路徑)，如："test.bmp"
	副程式輸出為BMPIMAGE影像資料結構(該影像資料結構包含檔名(FILENAME)、
	圖像寬度(XSIZE)、圖像高度(YSIZE)、填補位元(FILLINGBYTE)與圖像資料(IMAGE_DATA))
 */
BMPIMAGE BmpFileRead(const char *, const bool);							//	宣告BmpFileRead副程式
/*	BmpWriteV1副程式用於寫入BMP圖檔
	副程式輸入為(欲寫入圖檔之unsigned char型態指標資料,欲寫入圖檔之寬度,欲寫入圖檔之高度,欲寫入之圖檔路徑)
	副程式輸出：若順利寫入圖檔則傳回0，反之傳回-1
 */
int BmpWriteV1(const unsigned char*,const int,const int,const char*); 	//	宣告BmpWriteV1(BMP圖檔寫入)副程式
/*	BmpWriteV2副程式用於寫入BMP圖檔
	本副程式將BmpWriteV1副程式之輸入參數結構化，以BMPIMAGE結構圖檔資料傳入，為使用介面上的改進
	實作上仍呼叫BmpWriteV1副程式進行圖檔寫入
	副程式輸入為(欲寫入之BMPIMAGE結構圖檔資料)
	副程式輸出：若順利寫入圖檔則傳回0，反之傳回-1
 */
int BmpWriteV2(const BMPIMAGE); 										//	宣告BmpWriteV2(BMP圖檔寫入)副程式
/*	ViewBMPImage副程式用於呼叫圖片檢視器開啟圖檔
	副程式輸入為欲開啟檢視之圖檔絕對路徑(配合FILE_ROOT_PATH形成絕對路徑)
	副程式輸入為void(無)
 */
void ViewBMPImage(const char *);										//	宣告ViewBMPImage(BMP圖片檢視)副程式(以Windows圖片檢視器開啟)
/*	InitialIMGArray副程式用於生成BMP24RGB指標變數，並將其資料初始化為0
	副程式輸入為(欲生成BMP24RGB指標變數之圖像寬度,欲生成BMP24RGB指標變數之圖像高度)
	副程式輸出為生成之BMP24RGB指標變數
 */
BMP24RGB *InitialIMGArray(const int, const int);						//	宣告InitialIMGArray副程式
/*	RAWImageToArray副程式將來自BMP圖檔之圖像資料轉換至RGB型態二維陣列，
	轉換後的一個BMP24RGB型態二維陣列代表一張影像，其中：
	__________________
	|                |
	|                |
	|                |
	|                |
	|                |
	|                |
	|________________|
	 ↑為第一像素
	 
 */
BMP24RGB *RGBQUADtoBMP24RGB(const RGBQUAD* InputRGBQUADImage, const int xsize, const int ysize);
RGBQUAD* BMP24RGBtoRGBQUAD(const BMP24RGB* InputBMP24RGBImage, const int xsize, const int ysize);
BMP24RGB *RAWImageToArray(const unsigned char*, const int, const int);	//	宣告RAWImageToArray(BMP圖檔資料至陣列轉換)副程式
/*	ArrayToRAWImage副程式用於將RGB型態圖像二維陣列轉換至符合BMP圖檔格式之圖像資料(含填補位元)
	副程式輸入為(RGB型態圖像二維陣列,圖像寬度,圖像高度)
	副程式輸出為無號字元指標(unsigned char *)型態，符合BMP圖檔格式之圖像資料(含填補位元)
 */
unsigned char *ArrayToRAWImage(const BMP24RGB*,const int,const int);	//	宣告陣列至BMP圖檔資料轉換副程式

/*	BMP24RGBToHSV副程式用於將BMP24RGB型態之RGB像素資料轉換至HSV色彩空間
	副程式輸入為(欲轉換至HSV之BMP24RGB型態圖像資料,欲轉換至HSV之BMP24RGB型態圖像寬度,欲轉換至HSV之BMP24RGB型態圖像高度)
	副程式輸出為由BMP24RGB色彩空間轉換至HSV完成之影像資料
 */
HSV *BMP24RGBToHSV(const BMP24RGB*,const int,const int);				//	宣告BMP24RGB型態至HSV轉換副程式
/*	HSVToBMP24RGB副程式用於將HSV型態之像素資料轉換至BMP24RGB色彩空間
	副程式輸入為(欲轉換至BMP24RGB之HSV型態圖像資料,欲轉換至BMP24RGB之HSV型態圖像寬度,欲轉換至BMP24RGB之HSV型態圖像高度)
	副程式輸出為由HSV色彩空間轉換至BMP24RGB完成之影像資料
 */
BMP24RGB *HSVToBMP24RGB(const HSV*,const int,const int);				//	宣告BMP24RGB型態至HSV轉換副程式
/*	ImageDataToTxt副程式用於將圖像影像資料寫入txt檔
	副程式輸入為(欲寫入之txt檔案路徑,欲寫入之圖像影像資料,欲寫入之圖像影像寬度,欲寫入之圖像影像高度)
	副程式輸出：若順利寫入檔案則傳回true，反之傳回false
	寫入檔案資料舉例如下：
		第0個像素R=175	第0個像素G=255	第0個像素B=92
		第1個像素R=176	第1個像素G=255	第1個像素B=92
		第2個像素R=177	第2個像素G=255	第2個像素B=93
		第3個像素R=178	第3個像素G=255	第3個像素B=93
		第4個像素R=179	第4個像素G=255	第4個像素B=94
		第5個像素R=180	第5個像素G=255	第5個像素B=95
		第6個像素R=181	第6個像素G=255	第6個像素B=95
		第7個像素R=181	第7個像素G=255	第7個像素B=96
		第8個像素R=182	第8個像素G=255	第8個像素B=96
		第9個像素R=183	第9個像素G=255	第9個像素B=97
	下一代副程式預計
 */
bool ImageDataToTxt(const char*,const BMP24RGB*,const int,const int);	//	宣告ImageDataToTxt(圖像影像資料寫入txt檔)副程式
/*	HSVDataToTxt副程式用於將HSV型態影像資料寫入txt檔
	寫入檔案資料舉例如下：
		第0個像素H=89.391197	第0個像素S=0.639216	第0個像素V=255.000000
		第1個像素H=88.985390	第1個像素S=0.639216	第1個像素V=255.000000
		第2個像素H=88.775009	第2個像素S=0.635294	第2個像素V=255.000000
		第3個像素H=88.366875	第3個像素S=0.635294	第3個像素V=255.000000
		第4個像素H=88.151466	第4個像素S=0.631373	第4個像素V=255.000000
		第5個像素H=87.933411	第5個像素S=0.627451	第5個像素V=255.000000
		第6個像素H=87.520569	第6個像素S=0.627451	第6個像素V=255.000000
		第7個像素H=87.712685	第7個像素S=0.623529	第7個像素V=255.000000
		第8個像素H=87.297371	第8個像素S=0.623529	第8個像素V=255.000000
		第9個像素H=87.071434	第9個像素S=0.619608	第9個像素V=255.000000
	副程式輸入為(欲寫入之txt檔案路徑,欲寫入之HSV型態影像資料,欲寫入之圖像影像寬度,欲寫入之圖像影像高度)
	副程式輸出：若順利寫入檔案則傳回true，反之傳回false	
 */
bool HSVDataToTxt(const char*, const HSV*, const int, const int);		//	宣告HSVDataToTxt(HSV型態影像資料寫入txt檔)副程式
/*	ImgDifference2副程式用於執行兩BMP24RGB型態圖像像素之差(相減)運算
	運算方法為令：
	A為一5*5圖像，像素資料如下：
		-				-
		|	10	20	30	|
		|	40	10	20	|
		|	70	40	50	|
		-				-
	
	B為一5*5圖像，像素資料如下：
		-				-
		|	5	10	15	|
		|	20	5	10	|
		|	35	20	25	|
		-				-
	
	則圖像A-B可呼叫ImgDifference2，呼叫方法如：ImgDifference2(A, B, 5, 5);
	運算結果為：
		-				-
		|	5	10	15	|
		|	20	5	10	|
		|	35	20	25	|
		-				-
	為考慮相減為負數之情況，進行減法運算後取絕對值
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
 */
BMP24RGB *ImgDifference2(const BMP24RGB*,const BMP24RGB*,const int,const int);
/*	BMP24RGB2or副程式用於執行兩BMP24RGB型態圖像像素之OR運算
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	副程式輸入為(欲進行or運算之來源影像1, 欲進行or運算之來源影像2, 來源影像寬度, 來源影像高度)
	副程式輸出為執行像素or運算後之結果
 */
BMP24RGB *BMP24RGB2or(const BMP24RGB*,const BMP24RGB*,const int,const int);
//	宣告BMP24RGB2or(兩BMP24RGB型態圖像資料or運算)副程式
/*	BmpToGraylevel副程式用於將BMP24RGB型態圖像轉為灰階圖像(Graylevel Image)
	運算方法為令：
		R = (R + G + B) / 3
		G = (R + G + B) / 3
		B = (R + G + B) / 3
	副程式輸入為(欲轉換至灰階之BMP24RGB型態圖像資料, 欲轉換至灰階之圖像寬度, 欲轉換至灰階之圖像高度)
	副程式輸出為轉換為灰階後之BMP24RGB型態圖像資料
 */
BMP24RGB *BmpToGraylevel(const BMP24RGB*, const int, const int);		//	宣告BMP圖片資料轉灰階副程式
/*	BmpConvolution33用於計算給定3*3遮罩與輸入圖像進行摺積(Convolution)，
	但精確上來說，摺積過程中須將遮罩翻轉，但本副程式未進行翻轉，僅進行相乘
	舉例來說：
		若傳入之3*3遮罩為
			--		--
			| 1 1 1	 |
			| 1 1 1	 |
			| 1 1 1	 |
			--		--
		一圖像3*3區塊像素值如下：
		-				-
		|	10	20	30	|
		|	40	10	20	|
		|	70	40	50	|
		-				-
		計算摺積=(10*1+20*1+30*1+40*1+10*1+20*1+70*1+40*1+50*1)/9(除以遮罩權重總和)=290/9=32.2222
		
		3*3遮罩傳入方式為一陣列：
			假設陣列名稱為Mask，數值設定舉例如下：
			--							--
			| Mask[6] Mask[7] Mask[8]	 |
			| Mask[3] Mask[4] Mask[5]	 |
			| Mask[0] Mask[1] Mask[2]	 |
			--							--
			由左下角開始編號為0，由左至右，由下至上遞增
		
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	副程式輸入為(欲進行Convolution運算之BMP24RGB型態影像, 欲進行Convolution運算之BMP24RGB型態圖像之寬度, 欲進行Convolution運算之BMP24RGB型態圖像之高度, Convolution之Mask)
	副程式輸出為執行Convolution運算後之結果
 */
BMP24RGB *BmpConvolution33(const BMP24RGB*,const int,const int,const long double[]);
//	宣告BmpConvolution33(圖像3*3遮罩摺積)副程式
/*	BmpConvolution55用於計算給定5*5遮罩與輸入圖像進行摺積(Convolution)，
	但精確上來說，摺積過程中須將遮罩翻轉，但本副程式未進行翻轉，僅進行相乘
	舉例來說：
		若傳入之5*5遮罩為
			--			--
			| 1 1 1 1 1	 |
			| 1 1 1	1 1	 |
			| 1 1 1	1 1	 |
			| 1 1 1	1 1	 |
			| 1 1 1	1 1	 |
			--			--
		一圖像5*5區塊像素值如下：
		--						--
		|	99	96	82	24	78	 |
		|	9	20	3	28	94	 |
		|	33	35	40	52	66	 |
		|	37	34	27	26	69	 |
		|	29	1	67	31	47	 |
		--						--
		計算摺積=(	99 * 1 + 96 * 1 + 82 * 1 + 24 * 1 + 78 * 1 + 
					9 * 1 + 20 * 1 + 3 * 1 + 28 * 1 + 94 * 1 + 
					33 * 1 + 35 * 1 + 40 * 1 + 52 * 1 + 66 * 1 + 
					37 * 1 + 34 * 1 + 27 * 1 + 26 * 1 + 69 * 1 + 
					29 * 1 + 1 * 1 + 67 * 1 + 31 * 1 + 47)/25(除以遮罩權重總和)=1127 / 25 = 45.08
		
		5*5遮罩傳入方式為一陣列：
			假設陣列名稱為Mask，數值設定舉例如下：
			--												--
			| Mask[20] Mask[21] Mask[22] Mask[23] Mask[24]	 |
			| Mask[15] Mask[16] Mask[17] Mask[18] Mask[19]	 |
			| Mask[10] Mask[11] Mask[12] Mask[13] Mask[14]	 |
			| Mask[ 5] Mask[ 6] Mask[ 7] Mask[ 8] Mask[ 9]	 |
			| Mask[ 0] Mask[ 1] Mask[ 2] Mask[ 3] Mask[ 4]	 |
			--												--
			由左下角開始編號為0，由左至右，由下至上遞增
			
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	副程式輸入為(欲進行Convolution運算之BMP24RGB型態影像, 欲進行Convolution運算之BMP24RGB型態圖像之寬度, 欲進行Convolution運算之BMP24RGB型態圖像之高度, Convolution之Mask)
	副程式輸出為執行Convolution運算後之結果
 */
BMP24RGB *BmpConvolution55(const BMP24RGB*,const int,const int,const long double[]);
//	宣告BmpConvolution55(圖像5*5遮罩摺積)副程式
/*	BmpConvolution77用於計算給定7*7遮罩與輸入圖像進行摺積(Convolution)，
	但精確上來說，摺積過程中須將遮罩翻轉，但本副程式未進行翻轉，僅進行相乘
	舉例來說：
		若傳入之7*7遮罩為
			--			   --
			| 1 1 1 1 1 1 1	|
			| 1 1 1	1 1 1 1	|
			| 1 1 1	1 1 1 1	|
			| 1 1 1	1 1 1 1	|
			| 1 1 1	1 1 1 1	|
			| 1 1 1	1 1 1 1	|
			| 1 1 1	1 1 1 1	|
			--			   --
		一圖像7*7區塊像素值如下：
		--							   --
		|	99	96	82	24	78	20	84	|
		|	9	20	3	28	94	54	43	|
		|	33	35	40	52	66	73	12	|
		|	37	34	27	26	69	41	93	|
		|	29	1	67	31	47	55	34	|
		|	50	109	200	197	140	29	22	|
		|	184	13	238	97	170	253	239	|
		--							   --
		計算摺積=(	99 * 1 + 96 * 1 + 82 * 1 + 24 * 1 + 78 * 1 + 20 * 1 + 84 * 1 +
					9 * 1 + 20 * 1 + 3 * 1 + 28 * 1 + 94 * 1 + 54 * 1 + 43 * 1 +
					33 * 1 + 35 * 1 + 40 * 1 + 52 * 1 + 66 * 1 + 73 * 1 + 12 * 1 + 
					37 * 1 + 34 * 1 + 27 * 1 + 26 * 1 + 69 * 1 + 41 * 1 + 93 * 1 + 
					29 * 1 + 1 * 1 + 67 * 1 + 31 * 1 + 47 * 1 + 55 * 1 + 34 * 1 + 
					50 * 1 + 109 * 1 + 200 * 1 + 197 * 1 + 140 * 1 + 29 * 1 + 22 * 1 + 
					184 * 1 + 13 * 1 + 238 * 1 + 97 * 1 + 170 * 1 + 253 * 1 + 239
					)/( 7 * 7 )(除以遮罩權重總和) =
					3577 / 49 = 73
		
		7*7遮罩傳入方式為一陣列：
			假設陣列名稱為Mask，數值設定舉例如下：
			--																	--
			|	Mask[42] Mask[43] Mask[44] Mask[45] Mask[46] Mask[47] Mask[48]	 |
			|	Mask[35] Mask[36] Mask[37] Mask[38] Mask[39] Mask[40] Mask[41] 	 |
			|	Mask[28] Mask[29] Mask[30] Mask[31] Mask[32] Mask[33] Mask[34]	 |
			|	Mask[21] Mask[22] Mask[23] Mask[24] Mask[25] Mask[26] Mask[27]	 |
			|	Mask[14] Mask[15] Mask[16] Mask[17] Mask[18] Mask[19] Mask[20]	 |
			|	Mask[ 7] Mask[ 8] Mask[ 9] Mask[10] Mask[11] Mask[12] Mask[13]	 |
			|	Mask[ 0] Mask[ 1] Mask[ 2] Mask[ 3] Mask[ 4] Mask[ 5] Mask[ 6]	 |
			--																	--
			由左下角開始編號為0，由左至右，由下至上遞增
	
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	副程式輸入為(欲進行Convolution運算之BMP24RGB型態影像, 欲進行Convolution運算之BMP24RGB型態圖像之寬度, 欲進行Convolution運算之BMP24RGB型態圖像之高度, Convolution之Mask)
	副程式輸出為執行Convolution運算後之結果
 */
BMP24RGB *BmpConvolution77(const BMP24RGB*,const int,const int,const long double[]);
//	宣告BmpConvolution77(圖像7*7遮罩摺積)副程式
/*	BmpConvolution用於計算給定指定大小遮罩與輸入圖像進行摺積(Convolution)，
	但精確上來說，摺積過程中須將遮罩翻轉，但本副程式未進行翻轉，僅進行相乘。
	遮罩傳入方式為一陣列，由左下角開始編號為0，由左至右，由下至上遞增。
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同。
	本副程式輸入參數較多，依序說明如下：
	第一項參數為Convolution遮罩大小，若欲進行摺積之遮罩為7*7，則該參數傳入7
	第二項參數為欲進行Convolution運算之BMP24RGB型態影像
	第三項參數為欲進行Convolution運算之BMP24RGB型態圖像之寬度
	第四項參數為欲進行Convolution運算之BMP24RGB型態圖像之高度
	第五項參數為Convolution之Mask資料，需與第一項參數配合，若第一項參數傳入7，則該處Mask為一7*7大小之陣列
	副程式輸入為(Convolution遮罩大小, 欲進行Convolution運算之BMP24RGB型態影像, 欲進行Convolution運算之BMP24RGB型態圖像之寬度, 欲進行Convolution運算之BMP24RGB型態圖像之高度, Convolution之Mask)
	副程式輸出為執行Convolution運算後之結果
 */
BMP24RGB *BmpConvolution(const int, const BMP24RGB*,const int,const int,const long double[]);
//	宣告BmpConvolution(圖像摺積)副程式
/*	ImageSmoothing33V1(ImageSmoothing33 Version 1)副程式用於計算BMP24RGB型態圖像之3*3Mask平滑濾波
	運算時使用之3*3 Mask如下：
		-				-
		|	1/9	1/9	1/9	|
		|	1/9	1/9	1/9	|
		|	1/9	1/9	1/9	|
		-				-
	舉例而言：
	一圖像3*3區塊像素值如下：
		-				-
		|	10	20	30	|
		|	40	10	20	|
		|	70	40	50	|
		-				-
	計算平滑濾波得之像素值為10/9+20/9+30/9+40/9+10/9+20/9+70/9+40/9+50/9=290/9=32.2222
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	副程式輸入為(欲進行平滑濾波之BMP24RGB型態圖像資料,欲進行平滑濾波之圖像寬度,欲進行平滑濾波之圖像高度)
	副程式輸出為進行平滑濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *ImageSmoothing33V1(const BMP24RGB*,const int,const int);		//	宣告ImageSmoothing33V1(BMP圖檔3*3Mask平滑濾波 Version 1)副程式
/*	ImageSmoothing33V2(ImageSmoothing33 Version 2)副程式用於計算BMP24RGB型態圖像之3*3Mask平滑濾波
	運算使用BmpConvolution33副程式實現
	副程式輸入為(欲進行平滑濾波之BMP24RGB型態圖像資料,欲進行平滑濾波之圖像寬度,欲進行平滑濾波之圖像高度)
	副程式輸出為進行平滑濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *ImageSmoothing33V2(const BMP24RGB*,const int,const int);		//	宣告ImageSmoothing33V2(BMP圖檔3*3Mask平滑濾波 Version 2)副程式
/*	ImageSmoothing55副程式用於計算BMP24RGB型態圖像之5*5Mask平滑濾波
	運算使用BmpConvolution55副程式實現
	副程式輸入為(欲進行平滑濾波之BMP24RGB型態圖像資料,欲進行平滑濾波之圖像寬度,欲進行平滑濾波之圖像高度)
	副程式輸出為進行平滑濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *ImageSmoothing55(const BMP24RGB*,const int,const int);		//	宣告ImageSmoothing55(BMP圖檔5*5Mask平滑濾波)副程式
/*	ImageSmoothing77副程式用於計算BMP24RGB型態圖像之7*7Mask平滑濾波
	運算使用BmpConvolution77副程式實現
	副程式輸入為(欲進行平滑濾波之BMP24RGB型態圖像資料,欲進行平滑濾波之圖像寬度,欲進行平滑濾波之圖像高度)
	副程式輸出為進行平滑濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *ImageSmoothing77(const BMP24RGB*,const int,const int);		//	宣告ImageSmoothing77(BMP圖檔7*7Mask平滑濾波)副程式
/*	ImageSmoothing副程式用於計算BMP24RGB型態圖像之指定大小Mask平滑濾波
	運算使用BmpConvolution副程式實現
	第一項傳入參數為欲進行平滑濾波之Mask大小，若欲進行平滑濾波之遮罩為7*7，則該參數傳入7
	副程式輸入為(欲進行平滑濾波之Mask大小,欲進行平滑濾波之BMP24RGB型態圖像資料,欲進行平滑濾波之圖像寬度,欲進行平滑濾波之圖像高度)
	副程式輸出為進行平滑濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *ImageSmoothing(const int, const BMP24RGB*,const int,const int);
//	宣告ImageSmoothing(BMP圖檔平滑濾波)副程式
/*	MedianFilter33副程式用於計算BMP24RGB型態圖像之3*3Mask中值濾波
	副程式輸入為(欲進行中值濾波之BMP24RGB型態圖像資料,欲進行中值濾波之圖像寬度,欲進行中值濾波之圖像高度)
	副程式輸出為進行中值濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *MedianFilter33(const BMP24RGB*,const int,const int);			//	宣告MedianFilter33(BMP圖檔3*3中值濾波)副程式
/*	GaussianBlur33V1(3*3高斯濾波 Version 1)副程式用於計算BMP24RGB型態圖像資料之高斯模糊
	參考自維基百科：https://zh.wikipedia.org/wiki/%E9%AB%98%E6%96%AF%E6%A8%A1%E7%B3%8A
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	副程式輸入為(欲進行高斯濾波之BMP24RGB型態圖像資料,欲進行高斯濾波之圖像寬度,欲進行高斯濾波之圖像高度,常態分布的標準偏差σ)
	副程式輸出為進行高斯濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *GaussianBlur33V1(const BMP24RGB*,const int,const int,const long double);
//	宣告GaussianBlur33V1(3*3高斯濾波 Version 1)副程式
/*	GaussianBlur33V2(3*3高斯濾波 Version 2)副程式用於計算BMP24RGB型態圖像資料之高斯模糊
	參考自維基百科：https://zh.wikipedia.org/wiki/%E9%AB%98%E6%96%AF%E6%A8%A1%E7%B3%8A
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	運算使用BmpConvolution33副程式實現
	副程式輸入為(欲進行高斯濾波之BMP24RGB型態圖像資料,欲進行高斯濾波之圖像寬度,欲進行高斯濾波之圖像高度,常態分布的標準偏差σ)
	副程式輸出為進行高斯濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *GaussianBlur33V2(const BMP24RGB*, const int, const int, const long double);
//	宣告GaussianBlur33V2(3*3高斯濾波 Version 2)副程式
/*	GaussianBlur副程式用於計算任意大小之高斯濾波
	參考自維基百科：https://zh.wikipedia.org/wiki/%E9%AB%98%E6%96%AF%E6%A8%A1%E7%B3%8A
	本副程式將圖像像素之R、G、B成分分別處理，演算法相同
	運算使用BmpConvolution副程式實現
	本副程式輸入參數較多，依序說明如下：
	第一項參數為高斯濾波Convolution遮罩大小，若欲進行摺積之遮罩為7*7，則該參數傳入7
	第二項參數為欲進行高斯濾波之BMP24RGB型態圖像資料
	第三項參數為欲進行高斯濾波之圖像寬度
	第四項參數為欲進行高斯濾波之圖像高度
	第五項參數為常態分布的標準偏差σ
	副程式輸入為(高斯濾波遮罩大小,欲進行高斯濾波之BMP24RGB型態圖像資料,欲進行高斯濾波之圖像寬度,欲進行高斯濾波之圖像高度,常態分布的標準偏差σ)
	副程式輸出為進行高斯濾波後之BMP24RGB型態圖像資料
 */
BMP24RGB *GaussianBlur(const int, const BMP24RGB*, const int, const int, const long double);
//	宣告GaussianBlur(高斯濾波)副程式
/*	GaussianFigure2D副程式用於產生二維高斯資料圖形
	二維高斯圖像中心為峰值，逐漸向外遞減，因此該圖像中心為白色，以同心圓向外漸灰
	二維高斯計算使用NormalDistribution2D副程式
	副程式輸入為(欲生成二維高斯圖像寬度, 欲生成二維高斯圖像高度, 高斯分布的標準偏差σ)
	副程式輸出為二維高斯之BMP24RGB型態圖像資料
 */
BMP24RGB *GaussianFigure2D(const int, const int, const long double);	//	宣告GaussianFigure2D(二維高斯圖像)生成副程式
/*	NormalDistribution2D(二維常態分布計算)副程式用於計算二維常態分布數值
	公式如下：
		pow(M_E,-(pow(xlocation,2) + pow(ylocation,2)) / (2 * pow(StandardDeviation,2)))/(2 * M_PI * pow(StandardDeviation,2))
	公式中xlocation、ylocation為二維座標，StandardDeviation為常態分布的標準偏差σ
	副程式輸入為(xlocation,ylocation,StandardDeviation)
	副程式輸出為二維常態分布計算結果
 */
long double NormalDistribution2D(long double, long double, long double);//	宣告NormalDistribution2D(二維常態分布計算)副程式
BMP24RGB *ImageOCR(const BMP24RGB*,const int,const int);				//	宣告ImageOCR(影像OCR)副程式
/*	BMP24RGBGradient副程式用於計算BMP24RGB型態圖像資料之梯度
	運算方法為
		Step1：計算出Gx(X方向梯度)與Gy(Y方向梯度)，計算方法舉例如下：
		
		一圖像3*3區塊像素值如下：
		-				-
		|	10	20	30	|
		|	40	10	20	|
		|	70	40	50	|
		-				-
		
		則：
		Gx = 	-				-	-				-
				|	10	20	30	|	|	-1	0	1	|
				|	40	10	20	| * |	-1	0	1	| = 10 * (-1) + 40 * (-1) + 70 * (-1) + 
				|	70	40	50	|	|	-1	0	1	|	20 *   0  + 10 *   0  + 40 *   0  + 
				-				-	-				-	30 *   1  + 20 *   1  + 50 *   1  = -20
				
		Gy = 	-				-	-				-
				|	10	20	30	|	|	-1	-1	-1	|
				|	40	10	20	| * |	 0	 0	 0	| = 10 * (-1) + 20 * (-1) + 30 * (-1) + 
				|	70	40	50	|	|	 1	 1	 1	|	40 *   0  + 10 *   0  + 20 *   0  + 
				-				-	-				-	70 *   1  + 40 *   1  + 50 *   1  = 100
		
		Step2：計算梯度大小(Magnitude)與方向(Direction)
		藉由Gx與Gy計算梯度大小(Magnitude)與方向(Direction)，在此方向(Direction)為角度(deg)
		大小(Magnitude)之值域為[-765,765]
		方向(Direction)經atan函數計算之值域為[-90°,90°]，但實際上需考慮Gx與Gx之正負號以決定方向，
		若(Gx>0)且(Gy>0)，則方向(Direction)範圍介於0~90度
			ex：當Gx=1，Gy=1時，atan(1) = 0.785398163 rad = 45 deg，方向(Direction) = 45 deg
			當(Gx>0)且(Gy>0)時，方向(Direction) = atan( Gy / Gx )(deg)
		若(Gx<0)且(Gy>0)，則方向(Direction)範圍介於90~180度
			ex：當Gx=-1，Gy=1時，atan(-1) = -0.785398163 rad = -45 deg，但方向(Direction)應為135 deg
			當(Gx<0)且(Gy>0)時，方向(Direction) = atan( Gy / Gx )(deg) + 180°
		若(Gx<0)且(Gy<0)，則方向(Direction)範圍介於180~270度
			ex：當Gx=-1，Gy=-1時，atan(1) = 0.785398163 rad = 45 deg，但方向(Direction)應為225 deg
			當(Gx<0)且(Gy<0)時，方向(Direction) = atan( Gy / Gx )(deg) + 180°
		若(Gx>0)且(Gy<0)，則方向(Direction)範圍介於270~360度
			ex：當Gx=1，Gy=-1時，atan(-1) = -0.785398163 rad = -45 deg，但方向(Direction)應為315 deg
			當(Gx>0)且(Gy<0)時，方向(Direction) = atan( Gy / Gx )(deg) + 360°
		上述四項角度特性可歸納為：
		若(Gx>0)，則方向(Direction)範圍介於-90~90度(第一、四象限，右半平面)		第二象限|第一象限
		若(Gx<0)，則方向(Direction)範圍介於90~270度(第二、三象限，左半平面)		-----------------
																				第三象限|第四象限
		
		因此使用atan函數計算時，若(Gx<0)則計算結果需+180度，則方向(Direction)之值域為[-90,270]，
		此時左半平面之角度為正確，但右半平面之角度值域為[-90,90]，當右半平面之角度<0時，需將該角度+360
		
		故得：
		Magnitude = ( Gx ^ 2 + Gy ^ 2 ) ^ 0.5										//	計算梯度大小(Magnitude)
		Direction = atan( Gy / Gx ) * (180 / PI) 		if (Gx > 0) and (Gy >= 0)	//	計算梯度方向(Direction)－第一象限
					atan( Gy / Gx ) * (180 / PI) + 180°	if Gx < 0					//	計算梯度方向(Direction)－第二象限、第三象限
					atan( Gy / Gx ) * (180 / PI) + 360°	if (Gx > 0) and (Gy < 0)	//	計算梯度方向(Direction)－第四象限
							 90							if (Gx = 0) and (Gy > 0)	//	90°情況
							270							if (Gx = 0) and (Gy < 0)	//	270°情況
							0(Nan?)						if (Gx = 0) and (Gy = 0)	//	當Gx為0且Gy亦為0時，梯度方向無法定義

		本副程式將圖像像素之R、G、B成分分別處理，演算法相同
		
	副程式輸入為(欲計算梯度之BMP24RGB型態圖像資料,欲計算梯度之圖像寬度,欲計算梯度之圖像高度)
	副程式輸出為計算梯度後之BMP24RGB型態圖像資料
 */
BMP24RGB *BMP24RGBGradient(const BMP24RGB*,const int,const int);		//	宣告BMP24RGBGradient(BMP圖片資料梯度計算)副程式
/*	BMP24RGBSobelEdge副程式用於Sobel(索貝爾算子)邊緣偵測
	運算方法為
		Step1：計算出Gx(橫向邊緣檢測)與Gy(縱向邊緣檢測)，計算方法舉例如下：
		
		一圖像3*3區塊像素值如下：
		--				--
		|	10	20	30	 |
		|	40	10	20	 |
		|	70	40	50	 |
		--				--
		
		則：
		Gx = 	-				-	-				-
				|	10	20	30	|	|	-1	0	1	|
				|	40	10	20	| * |	-2	0	2	| = 10 * (-1) + 40 * (-2) + 70 * (-1) + 
				|	70	40	50	|	|	-1	0	1	|	20 *   0  + 10 *   0  + 40 *   0  + 
				-				-	-				-	30 *   1  + 20 *   2  + 50 *   1  = -40
				
		Gy = 	-				-	-				-
				|	10	20	30	|	|	-1	-2	-1	|
				|	40	10	20	| * |	 0	 0	 0	| = 10 * (-1) + 20 * (-2) + 30 * (-1) + 
				|	70	40	50	|	|	 1	 2	 1	|	40 *   0  + 10 *   0  + 20 *   0  + 
				-				-	-				-	70 *   1  + 40 *   2  + 50 *   1  = 120
		
		Step2：計算梯度大小(Magnitude)與方向(Direction)
		藉由Gx與Gy計算梯度大小(Magnitude)與方向(Direction)，在此方向(Direction)為角度(deg)
		大小(Magnitude)之值域為[-765,765]
		方向(Direction)經atan函數計算之值域為[-90°,90°]，但實際上需考慮Gx與Gx之正負號以決定方向，
		若(Gx>0)且(Gy>0)，則方向(Direction)範圍介於0~90度
			ex：當Gx=1，Gy=1時，atan(1) = 0.785398163 rad = 45 deg，方向(Direction) = 45 deg
			當(Gx>0)且(Gy>0)時，方向(Direction) = atan( Gy / Gx )(deg)
		若(Gx<0)且(Gy>0)，則方向(Direction)範圍介於90~180度
			ex：當Gx=-1，Gy=1時，atan(-1) = -0.785398163 rad = -45 deg，但方向(Direction)應為135 deg
			當(Gx<0)且(Gy>0)時，方向(Direction) = atan( Gy / Gx )(deg) + 180°
		若(Gx<0)且(Gy<0)，則方向(Direction)範圍介於180~270度
			ex：當Gx=-1，Gy=-1時，atan(1) = 0.785398163 rad = 45 deg，但方向(Direction)應為225 deg
			當(Gx<0)且(Gy<0)時，方向(Direction) = atan( Gy / Gx )(deg) + 180°
		若(Gx>0)且(Gy<0)，則方向(Direction)範圍介於270~360度
			ex：當Gx=1，Gy=-1時，atan(-1) = -0.785398163 rad = -45 deg，但方向(Direction)應為315 deg
			當(Gx>0)且(Gy<0)時，方向(Direction) = atan( Gy / Gx )(deg) + 360°
		上述四項角度特性可歸納為：
		若(Gx>0)，則方向(Direction)範圍介於-90~90度(第一、四象限，右半平面)		第二象限|第一象限
		若(Gx<0)，則方向(Direction)範圍介於90~270度(第二、三象限，左半平面)		-----------------
																				第三象限|第四象限
		
		因此使用atan函數計算時，若(Gx<0)則計算結果需+180度，則方向(Direction)之值域為[-90,270]，
		此時左半平面之角度為正確，但右半平面之角度值域為[-90,90]，當右半平面之角度<0時，需將該角度+360
		
		故得：
		Magnitude = ( Gx ^ 2 + Gy ^ 2 ) ^ 0.5										//	計算梯度大小(Magnitude)
		Direction = atan( Gy / Gx ) * (180 / PI) 		if (Gx > 0) and (Gy >= 0)	//	計算梯度方向(Direction)－第一象限
					atan( Gy / Gx ) * (180 / PI) + 180°	if Gx < 0					//	計算梯度方向(Direction)－第二象限、第三象限
					atan( Gy / Gx ) * (180 / PI) + 360°	if (Gx > 0) and (Gy < 0)	//	計算梯度方向(Direction)－第四象限
							 90							if (Gx = 0) and (Gy > 0)	//	90°情況
							270							if (Gx = 0) and (Gy < 0)	//	270°情況
							0(Nan?)						if (Gx = 0) and (Gy = 0)	//	當Gx為0且Gy亦為0時，梯度方向無法定義
		
		本副程式將圖像像素之R、G、B成分分別處理，演算法相同
		
		參考自維基百科：https://zh.wikipedia.org/wiki/%E7%B4%A2%E8%B2%9D%E7%88%BE%E7%AE%97%E5%AD%90
 */
BMP24RGB *BMP24RGBSobelEdge(const BMP24RGB*,const int,const int);		//	宣告BMP24RGBSobelEdge(BMP圖片資料Sobel邊緣偵測)副程式
/*	RGBHistogramEqualization副程式用於對BMP24RGB型態影像進行Histogram Equalization(直方圖等化)
	運算方法分別對R、G、B進行像素值統計、累積，分別Histogram Equalization(直方圖等化)，但該方法可能導致圖像顏色改變
	副程式輸入為(欲進行直方圖等化之BMP24RGB型態影像資料,影像寬度,影像高度)
	副程式輸出為直方圖等化後之BMP24RGB型態影像資料
 */
BMP24RGB *RGBHistogramEqualization(const BMP24RGB*,const int,const int);//	宣告RGBHistogramEqualization(RGB灰階影像直方圖等化)副程式
/*	BMPHaarWavelet副程式用於對BMP24RGB型態影像進行哈爾小波轉換
	運算方法分別對R、G、B進行哈爾小波轉換，具模式設定參數(Mode)，配合HaarWaveletMode集合宣告：
	HaarWaveletMode集合宣告HorizontalHighPass(水平高通濾波)、HorizontalLowPass(水平低通濾波)、VerticalHighPass(垂直高通濾波)與VerticalLowPass(垂直低通濾波)
	四種模式，分別說明如下：
		- HorizontalHighPass(水平高通濾波)模式
		計算水平像素間的差值，若水平像素間差異愈大，則輸出像素值愈大，使用遮罩為
						--				--
						|	-1,	0,	1	 |
						--				--
		故經過該模式輸出圖片將保留垂直紋理
		- HorizontalLowPass(水平低通濾波)模式
		計算水平像素平均值，使用遮罩為
						--					--
						|	0.5,	0,	0.5	 |
						--					--
		具有水平方向模糊之效果
		- VerticalHighPass(垂直高通濾波)
		計算垂直像素間的差值，若垂直像素間差異愈大，則輸出像素值愈大，使用遮罩為
						--		--
						|	-1	 |
						|	0	 |
						|	1	 |
						--		--
		故經過該模式輸出圖片將保留水平紋理
		- VerticalLowPass(垂直低通濾波)
		計算垂直像素平均值，使用遮罩為
						--		--
						|	0.5	 |
						|	0	 |
						|	0.5	 |
						--		--
		具有垂直方向模糊之效果
	副程式輸入為(欲進行Haar Wavelet之BMP24RGB型態影像資料,影像寬度,影像高度,Haar小波轉換模式)
	副程式輸出為Haar小波轉換後之BMP24RGB型態影像資料
 */
BMP24RGB *BMPHaarWavelet(const BMP24RGB*,const int,const int, const char);
/*	BMPHaarWavelet2副程式用於對BMP24RGB型態影像進行二階哈爾(Haar)小波轉換
	運算方法說明如下：
		由於圖像為二維結構，Haar小波濾波方向可分為水平濾波與垂直濾波，
		該副程式執行二階哈爾(Haar)小波轉換時
 */
BMP24RGB *BMPHaarWavelet2(const BMP24RGB*,const int,const int, const char);
/*	HSVHistogramEqualization副程式用於對HSV型態影像進行Histogram Equalization(直方圖等化)
	運算方法僅對於HSV色彩空間之Value進行Histogram Equalization(直方圖等化)
	副程式輸入為(欲進行直方圖等化之HSV型態影像資料,影像寬度,影像高度)
	副程式輸出為直方圖等化後之HSV型態影像資料
 */
HSV *HSVHistogramEqualization(const HSV*,const int,const int);			//	宣告HSVHistogramEqualization(HSV影像直方圖等化)副程式

/*	HueToBMP24RGB副程式用於將HSV型態影像資料中之Hue資料取出填入BMP24RGB圖像
	由於本程式中使用HSV色彩空間之Hue值域為0～360，但BMP24RGB型態之R、G、B之值域為0～255
	故令：
		R=Hue * 255 / 360；G=Hue * 255 / 360；B=Hue * 255 / 360
	即可完成將Hue填入至BMP24RGB型態
	副程式輸入為(HSV型態影像資料,HSV型態影像寬度,HSV型態影像高度)
	副程式輸出為取自HSV型態影像之Hue填入得BMP24RGB圖像資料
 */
BMP24RGB *HueToBMP24RGB(const HSV*,const int,const int);				//	宣告HueToBMP24RGB副程式
/*	SaturationToBMP24RGB副程式用於將HSV型態影像資料中之Saturation資料取出填入BMP24RGB圖像
	由於本程式中使用HSV色彩空間之Saturation值域為0～1，但BMP24RGB型態之R、G、B之值域為0～255
	故令：
		R=Saturation * 255；G=Saturation * 255；B=Saturation * 255
	即可完成將Saturation填入至BMP24RGB型態
	副程式輸入為(HSV型態影像資料,HSV型態影像寬度,HSV型態影像高度)
	副程式輸出為取自HSV型態影像之Saturation填入得BMP24RGB圖像資料
 */
BMP24RGB *SaturationToBMP24RGB(const HSV*,const int,const int);			//	宣告SaturationToBMP24RGB副程式
/*	ValueToBMP24RGB副程式用於將HSV型態影像資料中之Value資料取出填入BMP24RGB圖像
	由於本程式中使用HSV色彩空間之Value值域為0～255，BMP24RGB型態之R、G、B之值域亦為0～255
	故直接令：
		R=Value；G=Value；B=Value
	即可完成將Value填入至BMP24RGB型態
	副程式輸入為(HSV型態影像資料,HSV型態影像寬度,HSV型態影像高度)
	副程式輸出為取自HSV型態影像之Value填入得BMP24RGB圖像資料
 */
BMP24RGB *ValueToBMP24RGB(const HSV*,const int,const int);				//	宣告ValueToBMP24RGB副程式
/*	HSVSkin副程式用於透過HSV色彩空間資訊中，由給定之H、S、V之範圍過濾出皮膚資訊
	在此副程式中設定H範圍為15～50；S範圍為0.23～0.68；凡像素色彩資訊在此範圍中皆保留原像素資訊，
	在範圍外則抑制像素明度(Value)資訊(輸出像素之Value=原像素Value*0.3)
	副程式輸入為(HSV型態影像資料,HSV型態影像寬度,HSV型態影像高度)
	副程式輸出為過濾皮膚資訊之HSV型態影像
 */
HSV *HSVSkin(const HSV*,const int,const int);							//	宣告HSVSkin副程式
/*	BmpFillingByteCalc(BMP圖檔資料區填補位元計算)副程式
 */
unsigned char BmpFillingByteCalc(const unsigned int);					//	宣告BmpFillingByteCalc(計算填充位元組大小)副程式
bool FileExistCheck(char *);											//	宣告FileExistCheck(檔案存在檢查)副程式
bool FileReadPermissionCheck(const char *);								//	宣告FileReadPermissionCheck(檔案讀取權限檢查)副程式
bool FileWritePermissionCheck(const char *);							//	宣告FileWritePermissionCheck(檔案寫入權限檢查)副程式
bool FileWrite(const char *,const char *,const char *);					//	宣告FileWrite(檔案寫入)副程式，執行文字檔案寫入
/*	UCharBubbleSort副程式用於排序unsigned char數值陣列
	副程式輸入為(欲排序之unsigned char數值指標(該指標含結束字元),欲排序數值個數,排序方式)
	在此排序方式輸入0為由小至大排序；輸入1為由大至小排序
	副程式輸出為排序完成之unsigned char數值指標
 */
unsigned char *UCharBubbleSort(const unsigned char *,const unsigned long long int,const bool);
int Compare(const void *,const void *);									//	宣告Compare副程式(供qsort排序使用)
/**	CountCharPointStr副程式
	該副程式用於計算字串指標長度；
	第一項參數為欲計算長度之字串；
	第二項參數為是否顯示計算細節之bool變數，若輸入Ture則顯示計算細節，反之則無
	如：CountCharPointStr("ABC", False)，可得長度為3
	如：CountCharPointStr("123456789a", True)，可顯示
	第1個字元為：1
	第2個字元為：2
	第3個字元為：3
	第4個字元為：4
	第5個字元為：5
	第6個字元為：6
	第7個字元為：7
	第8個字元為：8
	第9個字元為：9
	第10個字元為：a	
	並得長度為10
**/
unsigned long long int CountCharPointStr(const char *, const bool);
void Show_char_point_str(const char *);									//	宣告Show_char_point_str(顯示字元指標)副程式
/*	ShowUCharPointStr副程式用於顯示無號字元指標內容
	副程式輸入為(欲顯示內容之無號字元指標,無號字元指標長度)
	副程式輸出為void
 */
void ShowUCharPointStr(const unsigned char *,unsigned long long int);	//	宣告ShowUCharPointStr(顯示無號字元指標)副程式
/*	ShowLongDouble副程式用於顯示long double(長浮點數)數值
	雖long double(長浮點數)可記錄±1.7×10^(-308)~±1.7×10^308範圍內的數值
	由於ShowLongDouble副程式處理與運算上數值寬度為64位元
	因此可處理之InputNumber最大上限為2^64-1
	副程式輸入為欲顯示之long double型態數值
	副程式輸出為void
 */
void ShowLongDouble(const long double InputNumber);						//	宣告ShowLongDouble副程式
/*	InitialIMGArrayTest副程式用於測試InitialIMGArray副程式
	本測試副程式之輸入、輸出皆為void，所有參數皆設計於測試副程式中，
	以觀察使用副程式所需宣告與語法
	副程式輸入為void
	副程式輸出為void
 */
void InitialIMGArrayTest(void);											//	宣告InitialIMGArrayTest副程式
/*	BmpReadFilesizeTest副程式用於測試BmpReadFilesize副程式
	本測試副程式之輸入、輸出皆為void，所有參數皆設計於測試副程式中，
	以觀察使用副程式所需宣告與語法
	本副程式包含BmpReadFilesize副程式兩種用法－輸入檔案路徑包含副檔名與不含副檔名
	副程式輸入為void
	副程式輸出為void
 */
void BmpReadFilesizeTest(void);
/*	BmpReadXSizeTest副程式用於測試BmpReadXSize副程式
	本測試副程式之輸入、輸出皆為void，所有參數皆設計於測試副程式中，
	以觀察使用副程式所需宣告與語法
	副程式輸入為void
	副程式輸出為void
 */
void BmpReadXSizeTest(void);											//	宣告BmpReadXSizeTest副程式
/*	BmpReadYSizeTest副程式用於測試BmpReadYSize副程式
	本測試副程式之輸入、輸出皆為void，所有參數皆設計於測試副程式中，
	以觀察使用副程式所需宣告與語法
	副程式輸入為void
	副程式輸出為void
 */
void BmpReadYSizeTest(void);											//	宣告BmpReadYSizeTest副程式
/*	BmpReadTest副程式用於測試BmpRead副程式
	本測試副程式之輸入、輸出皆為void，所有參數皆設計於測試副程式中，
	以觀察使用副程式所需宣告與語法
	副程式輸入為void
	副程式輸出為void
 */
void BmpReadTest(void);													//	宣告BmpReadTest副程式
/*	BmpWriteV2Test副程式用於測試BmpWriteV2副程式
	本測試副程式之輸入、輸出皆為void，所有參數皆設計於測試副程式中，
	以觀察使用副程式所需宣告與語法
	副程式輸入為void
	副程式輸出為void
 */
void BmpWriteV2Test(void);												//	宣告BmpWriteV2Test副程式