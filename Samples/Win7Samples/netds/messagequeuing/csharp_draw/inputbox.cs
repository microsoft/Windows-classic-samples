// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace CSharp_Draw
{
	/// <summary>
	/// Summary description for InputBox.
	/// </summary>
	public class InputBox : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button btnCancel;
		public System.Windows.Forms.TextBox textBoxName;
		public bool bResult;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components;

		public InputBox()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
			bResult=false;
			textBoxName.Text=System.Environment.ExpandEnvironmentVariables("%USERNAME%");
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.textBoxName = new System.Windows.Forms.TextBox();
			this.btnCancel = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.btnOK = new System.Windows.Forms.Button();
			this.textBoxName.Location = new System.Drawing.Point(8, 96);
			this.textBoxName.Size = new System.Drawing.Size(336, 20);
			this.textBoxName.TabIndex = 1;
			this.textBoxName.Text = "";
			this.btnCancel.Location = new System.Drawing.Point(296, 48);
			this.btnCancel.Size = new System.Drawing.Size(48, 24);
			this.btnCancel.TabIndex = 3;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
			this.label1.Location = new System.Drawing.Point(8, 16);
			this.label1.Size = new System.Drawing.Size(280, 24);
			this.label1.TabIndex = 0;
			this.label1.Text = "Enter the name of the local input queue for drawings:";
			this.btnOK.Location = new System.Drawing.Point(296, 16);
			this.btnOK.Size = new System.Drawing.Size(48, 24);
			this.btnOK.TabIndex = 2;
			this.btnOK.Text = "OK";
			this.btnOK.Click += new System.EventHandler(this.btnOK_Click);
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.ClientSize = new System.Drawing.Size(357, 131);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {this.btnCancel,
																		  this.btnOK,
																		  this.textBoxName,
																		  this.label1});
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Text = "C#_Draw: Input Queue Name";

		}
		#endregion

		private void btnCancel_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void btnOK_Click(object sender, System.EventArgs e)
		{
			bResult = true;
			this.Close();
		}
	}
}
