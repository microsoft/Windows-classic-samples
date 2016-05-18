//=======================================================================================;
//
//  This source code is only intended as a supplement to existing Microsoft documentation. 
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=======================================================================================;

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Management;
using Microsoft.ManagementConsole;

namespace Microsoft.ManagementConsole.Samples
{
	/// <summary>
	/// Summary description for TestPageControl.
	/// </summary>
	public class GeneralPropertiesControl : System.Windows.Forms.UserControl	
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Constructor for property page control.
		/// </summary>
        /// <param name="parentgeneralPropertyPage"> Owner property sheet page. </param>
        public GeneralPropertiesControl(GeneralPropertyPage parentgeneralPropertyPage)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
            generalPropertyPage = parentgeneralPropertyPage;
		}

        public void InitializePageControl()
        {
			serviceResultNode = (ResultNode)generalPropertyPage.ParentSheet.SelectionObject;
			
			ManagementObjectSearcher ServiceQuery = new ManagementObjectSearcher
				("Select * from Win32_Service Where DisplayName ='" + serviceResultNode.DisplayName + "'");

			foreach (ManagementObject ServiceObject in ServiceQuery.Get())
			{
                TextBoxDisplayName.Text = ServiceObject.GetPropertyValue("DisplayName").ToString();
				TextBoxName.Text = ServiceObject.GetPropertyValue("Name").ToString();
				TextBoxDescription.Text = ServiceObject.GetPropertyValue("Description").ToString();
			}
		}


		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose(bool disposing)
		{
			if (disposing)
			{
				if (components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.LabelDisplayName = new System.Windows.Forms.Label();
            this.LabelDescription = new System.Windows.Forms.Label();
            this.TextBoxName = new System.Windows.Forms.TextBox();
            this.TextBoxDisplayName = new System.Windows.Forms.TextBox();
            this.TextBoxDescription = new System.Windows.Forms.TextBox();
            this.Divider = new System.Windows.Forms.GroupBox();
            this.PictureBoxIcon = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.PictureBoxIcon)).BeginInit();
            this.SuspendLayout();
            // 
            // LabelDisplayName
            // 
            this.LabelDisplayName.AutoSize = true;
            this.LabelDisplayName.ForeColor = System.Drawing.SystemColors.ControlText;
            this.LabelDisplayName.Location = new System.Drawing.Point(12, 68);
            this.LabelDisplayName.Name = "LabelDisplayName";
            this.LabelDisplayName.Size = new System.Drawing.Size(71, 13);
            this.LabelDisplayName.TabIndex = 10;
            this.LabelDisplayName.Text = "Display Name:";
            // 
            // LabelDescription
            // 
            this.LabelDescription.AutoSize = true;
            this.LabelDescription.ForeColor = System.Drawing.SystemColors.ControlText;
            this.LabelDescription.Location = new System.Drawing.Point(12, 89);
            this.LabelDescription.Name = "LabelDescription";
            this.LabelDescription.Size = new System.Drawing.Size(95, 13);
            this.LabelDescription.TabIndex = 11;
            this.LabelDescription.Text = "ServiceDescription:";
            // 
            // TextBoxName
            // 
            this.TextBoxName.BackColor = System.Drawing.SystemColors.Menu;
            this.TextBoxName.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.TextBoxName.Location = new System.Drawing.Point(48, 17);
            this.TextBoxName.Name = "TextBoxName";
            this.TextBoxName.Size = new System.Drawing.Size(249, 13);
            this.TextBoxName.TabIndex = 12;
            // 
            // TextBoxDisplayName
            // 
            this.TextBoxDisplayName.BackColor = System.Drawing.SystemColors.MenuBar;
            this.TextBoxDisplayName.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.TextBoxDisplayName.Location = new System.Drawing.Point(115, 68);
            this.TextBoxDisplayName.Name = "TextBoxDisplayName";
            this.TextBoxDisplayName.Size = new System.Drawing.Size(251, 13);
            this.TextBoxDisplayName.TabIndex = 13;
            // 
            // TextBoxDescription
            // 
            this.TextBoxDescription.AutoSize = false;
            this.TextBoxDescription.BackColor = System.Drawing.SystemColors.Menu;
            this.TextBoxDescription.Location = new System.Drawing.Point(12, 105);
            this.TextBoxDescription.Multiline = true;
            this.TextBoxDescription.Name = "TextBoxDescription";
            this.TextBoxDescription.Size = new System.Drawing.Size(257, 151);
            this.TextBoxDescription.TabIndex = 14;
            // 
            // Divider
            // 
            this.Divider.BackColor = System.Drawing.SystemColors.ControlText;
            this.Divider.Location = new System.Drawing.Point(12, 56);
            this.Divider.Name = "Divider";
            this.Divider.Size = new System.Drawing.Size(354, 1);
            this.Divider.TabIndex = 16;
            this.Divider.TabStop = false;
            this.Divider.Text = "groupBox1";
            // 
            // PictureBoxIcon
            // 
            this.PictureBoxIcon.InitialImage = Microsoft.ManagementConsole.Samples.Resource1.bitmap1;
            this.PictureBoxIcon.Location = new System.Drawing.Point(12, 17);
            this.PictureBoxIcon.Name = "PictureBoxIcon";
            this.PictureBoxIcon.Size = new System.Drawing.Size(32, 32);
            this.PictureBoxIcon.TabIndex = 17;
            this.PictureBoxIcon.TabStop = false;
            // 
            // GeneralPropertiesControl
            // 
            this.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.Controls.Add(this.PictureBoxIcon);
            this.Controls.Add(this.Divider);
            this.Controls.Add(this.TextBoxDescription);
            this.Controls.Add(this.TextBoxDisplayName);
            this.Controls.Add(this.TextBoxName);
            this.Controls.Add(this.LabelDescription);
            this.Controls.Add(this.LabelDisplayName);
            this.ForeColor = System.Drawing.SystemColors.Control;
            this.Name = "GeneralPropertiesControl";
            this.Padding = new System.Windows.Forms.Padding(8);
            this.Size = new System.Drawing.Size(378, 354);
            ((System.ComponentModel.ISupportInitialize)(this.PictureBoxIcon)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		#region Methods Called by Owner Page

		/// <summary>
		/// Applies changes since the most recent apply.
		/// </summary>
		/// <returns> true if changes can be applied. </returns>
		public bool CanApplyChanges()
		{
			return true;
		}

		/// <summary>
		/// Applies changes since the most recent apply.
		/// </summary>
		/// <returns>  true if changes can be applied. </returns>
		public bool CanOKChanges()
		{
			
			return true;
		}

		/// <summary>
		/// Cancels changes since the most recent apply.
		/// </summary>
		/// <returns> true if changes can be reverted. </returns>
		public bool CanCancelChanges()
		{
			return true;
		}

		/// <summary>
		/// Can the page allow de-activation?
		/// </summary>
		/// <returns> true if the page can be deactivated. </returns>
		public bool CanKillActive()
		{
			return true;
		}
		
		#endregion Methods Called by Owner Page

		#region Event Handlers for Controls

		/// <summary>
		/// Disables cancel button and changes OK to Close.
		/// </summary>
		private void cancelToCloseButton_Click(object sender, System.EventArgs e)
		{
			generalPropertyPage.ParentSheet.CancelToClose();
		}

		#endregion Event Handlers for Controls

		#region Members

		private GeneralPropertyPage generalPropertyPage = null;
		private ResultNode serviceResultNode = null;

		#endregion Members

        private Label LabelDisplayName;
        private Label LabelDescription;
        private TextBox TextBoxName;
        private TextBox TextBoxDisplayName;
        private TextBox TextBoxDescription;
        private GroupBox Divider;
        private PictureBox PictureBoxIcon;

	} // class
} // namespace
