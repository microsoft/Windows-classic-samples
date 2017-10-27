using System;
using System.Collections.Generic;
using System.IO;                                                                //	使用System.IO函式庫
using System.Linq;
using System.Text;                                                              //	使用System.Text函式庫
using System.Threading.Tasks;

namespace DIPCodeConfigTool.CSharpFiles                                         //	DIPCodeConfigTool.CSharpFiles命名空間
{																				//	進入命名空間
	class FileIO                                                                //	FileIO類別
	{                                                                           //	進入FileIO類別
		public readonly static FileIO Instance = new FileIO();					//	宣告公用唯讀靜態物件，作為外部存取介面
		/// <summary>
		/// FileWrite方法
		/// </summary>
		/// <param name="File_name">為欲寫入檔案名稱</param>
		/// <param name="Input_string">為欲寫入檔案之字串資料</param>
		public void FileWrite(string File_name, string Input_string)			//  宣告FileWrite方法，將資料寫入檔案
		{                                                                       //  進入FileWrite方法
			FileStream file_stream = new FileStream(File_name, FileMode.Append);
			//  建立檔案指標，指向指定檔案名稱，模式為傳入之File_mode
			byte[] Input_data = System.Text.Encoding.Default.GetBytes(Input_string);
			//  將填入資料轉為位元陣列
			file_stream.Write(Input_data, 0, Input_data.Length);                //  寫入資料至檔案中
			file_stream.Flush();                                                //  清除緩衝區
			file_stream.Close();                                                //  關閉檔案
		}                                                                       //  結束FileWrite方法
		/// <summary>
		/// FileWrite方法，將資料寫入檔案
		/// </summary>
		/// <param name="File_name">為欲寫入檔案名稱</param>
		/// <param name="Input_string">為欲寫入檔案之字串資料</param>
		/// <param name="File_mode">為開啟檔案模式</param>
		public void FileWrite(string File_name, string Input_string, FileMode File_mode)
		{                                                                       //  進入FileWrite方法
			FileStream file_stream = new FileStream(File_name, File_mode);      //  建立檔案指標，指向指定檔案名稱，模式為傳入之File_mode
			byte[] Input_data = System.Text.Encoding.Default.GetBytes(Input_string);
			//  將填入資料轉為位元陣列
			file_stream.Write(Input_data, 0, Input_data.Length);                //  寫入資料至檔案中
			file_stream.Flush();                                                //  清除緩衝區
			file_stream.Close();                                                //  關閉檔案
		}                                                                       //  結束FileWrite方法
		/// <summary>
		/// FileWrite方法，將資料寫入檔案
		/// </summary>
		/// <param name="File_name">為欲寫入檔案名稱</param>
		/// <param name="Input_string">為欲寫入檔案之字串資料</param>
		/// <param name="File_mode">為開啟檔案模式</param>
		/// <param name="encoding">為檔案寫入編碼格式</param>
		public void FileWrite(string File_name, string Input_string, FileMode File_mode, Encoding encoding)
		{                                                                       //  進入FileWrite方法
			FileStream file_stream = new FileStream(File_name, File_mode);      //  建立檔案指標，指向指定檔案名稱，模式為傳入之File_mode
			byte[] Input_data = encoding.GetBytes(Input_string);				//  將填入資料轉為位元陣列
			file_stream.Write(Input_data, 0, Input_data.Length);                //  寫入資料至檔案中
			file_stream.Flush();                                                //  清除緩衝區
			file_stream.Close();                                                //  關閉檔案
		}                                                                       //  結束FileWrite方法
		public string ReadTxTFile(string file_name, Encoding encoding)          //  宣告ReadTxTFile方法
		{                                                                       //  進入ReadTxTFile方法
			//***區域變數宣告***
			System.IO.StreamReader textreader;                                  //  宣告textreader為System.IO.StreamReader物件
			string input_string;                                                //  宣告讀入字串
			input_string = "";                                                  //  初始化input_string為空字串
			try																	//	以try語法嘗試執行讀取檔案方法
			{																	//	進入try敘述
				textreader = new System.IO.StreamReader(file_name, encoding);   //	以指定encoding讀取檔案file_name
			}																	//	結束try敘述
			catch (Exception)
			{
				return "";														//	若發生讀取例外則回傳空字串
				throw;
			}
			input_string = textreader.ReadToEnd();                              //  讀取檔案至結尾，將檔案內容填入input_string
			textreader.Close();                                                 //	關閉檔案
			return input_string;                                                //  回傳讀取得字串資料
		}                                                                       //  結束ReadTxTFile方法
	}                                                                           //	結束FileIO類別
}																				//	結束命名空間
