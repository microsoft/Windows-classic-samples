using System;                                                                   //	使用System函式庫
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DIPCodeConfigTool.CSharpFiles                                         //	DIPCodeConfigTool.CSharpFiles命名空間
{																				//	進入命名空間
	class ConstItemsClass                                                       //	ConstItemsClass類別
	{                                                                           //	進入ConstItemsClass類別
		public readonly static ConstItemsClass Instance = new ConstItemsClass();//	宣告公用唯讀靜態物件，作為外部存取介面
		public const string CodeHeader =                                        //	建立CodeHeader字串物件
			@"/*	C語言影像處理函式庫
	Develop by Jimmy HU<s103360021@gmail.com> 
	This program is licensed under GNU General Public License v3.
	DIPDefine.h定義影像處理函數所使用之結構、資料型態
*/";																			//	建立CodeHeader內容
		public ConstItemsClass()                                                //	ConstItemsClass類別建構子
		{                                                                       //	進入ConstItemsClass類別建構子

		}                                                                       //	結束ConstItemsClass類別建構子
		public void WriteHeader(string FileName)                                //	WriteHeader方法
		{                                                                       //	進入WriteHeader方法
			FileIO.Instance.FileWrite(FileName, CodeHeader, System.IO.FileMode.Create);
			//	建立檔案
			return;                                                             //	結束方法
		}                                                                       //	結束WriteHeader方法
	}                                                                           //	結束ConstItemsClass類別
}																				//	結束命名空間
