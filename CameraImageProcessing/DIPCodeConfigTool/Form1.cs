using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DIPCodeConfigTool
{                                                                               //	進入命名空間
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
			tabControlInitialize();
		}
		void tabControlInitialize()
		{
			CSharpFiles.ConstItemsClass.Instance.WriteHeader("test.txt");
		}
	}
}																				//	結束命名空間
