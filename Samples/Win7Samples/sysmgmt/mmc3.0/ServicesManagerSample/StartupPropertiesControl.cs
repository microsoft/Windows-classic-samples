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
using Microsoft.ManagementConsole;
using System.Management;

namespace Microsoft.ManagementConsole.Samples
{
	public class StartupPropertiesControl : System.Windows.Forms.UserControl	
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Constructor for property page control.
		/// </summary>
        /// <param name="parentStartupPropertyPage">Owner property sheet page</param>
        public StartupPropertiesControl(StartupPropertyPage parentStartupPropertyPage)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
            startupPropertyPage = parentStartupPropertyPage;
		}

        public void InitializePageControl()
        {
			serviceResultNode = (ResultNode)startupPropertyPage.ParentSheet.SelectionObject;
			
			ManagementObjectSearcher ServiceQuery = new ManagementObjectSearcher
				("Select * from Win32_Service Where DisplayName ='" + serviceResultNode.DisplayName + "'");

			foreach (ManagementObject ServiceObject in ServiceQuery.Get())
			{
				TextBoxDisplayName.Text = ServiceObject.GetPropertyValue("DisplayName").ToString();
				ComboBoxStartupType.SelectedItem = ServiceObject.GetPropertyValue("StartMode").ToString();
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
            this.TextBoxDisplayName = new System.Windows.Forms.TextBox();
            this.TextBoxStartupType = new System.Windows.Forms.TextBox();
            this.Divider = new System.Windows.Forms.GroupBox();
            this.ComboBoxStartupType = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // TextBoxDisplayName
            // 
            this.TextBoxDisplayName.BackColor = System.Drawing.SystemColors.MenuBar;
            this.TextBoxDisplayName.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.TextBoxDisplayName.Location = new System.Drawing.Point(12, 12);
            this.TextBoxDisplayName.Name = "TextBoxDisplayName";
            this.TextBoxDisplayName.Size = new System.Drawing.Size(251, 13);
            this.TextBoxDisplayName.TabIndex = 17;
            // 
            // TextBoxStartupType
            // 
            this.TextBoxStartupType.BackColor = System.Drawing.SystemColors.MenuBar;
            this.TextBoxStartupType.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.TextBoxStartupType.Location = new System.Drawing.Point(12, 68);
            this.TextBoxStartupType.Name = "TextBoxStartupType";
            this.TextBoxStartupType.Size = new System.Drawing.Size(100, 13);
            this.TextBoxStartupType.TabIndex = 18;
            this.TextBoxStartupType.Text = "Startup Type";
            // 
            // Divider
            // 
            this.Divider.BackColor = System.Drawing.SystemColors.ControlText;
            this.Divider.Location = new System.Drawing.Point(13, 56);
            this.Divider.Name = "Divider";
            this.Divider.Size = new System.Drawing.Size(354, 1);
            this.Divider.TabIndex = 20;
            this.Divider.TabStop = false;
            this.Divider.Text = "groupBox1";
            // 
            // ComboBoxStartupType
            // 
            this.ComboBoxStartupType.FormattingEnabled = true;
            this.ComboBoxStartupType.Items.AddRange(new object[] {
            "Automatic",
            "Disabled",
            "Manual"});
            this.ComboBoxStartupType.Location = new System.Drawing.Point(12, 97);
            this.ComboBoxStartupType.Name = "ComboBoxStartupType";
            this.ComboBoxStartupType.Size = new System.Drawing.Size(354, 21);
            this.ComboBoxStartupType.TabIndex = 21;
            this.ComboBoxStartupType.SelectedIndexChanged += new System.EventHandler(this.ComboBoxStartupType_SelectedIndexChanged);
            // 
            // StartupPropertiesControl
            // 
            this.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.Controls.Add(this.ComboBoxStartupType);
            this.Controls.Add(this.Divider);
            this.Controls.Add(this.TextBoxStartupType);
            this.Controls.Add(this.TextBoxDisplayName);
            this.ForeColor = System.Drawing.SystemColors.Control;
            this.Margin = new System.Windows.Forms.Padding(0);
            this.Name = "StartupPropertiesControl";
            this.Padding = new System.Windows.Forms.Padding(8);
            this.Size = new System.Drawing.Size(378, 354);
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

			ManagementObjectSearcher ServiceQuery = new ManagementObjectSearcher
				("Select * from Win32_Service Where DisplayName ='" + serviceResultNode.DisplayName + "'");

			object[] parameters = new object[1];
			parameters[0] = ComboBoxStartupType.SelectedItem.ToString();

			foreach (ManagementObject ServiceObject in ServiceQuery.Get())
			{
				ServiceObject.InvokeMethod("ChangeStartMode", parameters);			
			}

			serviceResultNode.SubItemDisplayNames[2] = ComboBoxStartupType.SelectedItem.ToString();
			return true;
		}

		/// <summary>
		/// Applies changes since the most recent apply.
		/// </summary>
		/// <returns>  true if changes can be applied. </returns>
		public bool CanOKChanges()
		{
			if (startupPropertyPage.Dirty)
			{
				ManagementObjectSearcher ServiceQuery = new ManagementObjectSearcher
								("Select * from Win32_Service Where DisplayName ='" + serviceResultNode.DisplayName + "'");

				object[] parameters = new object[1];
				parameters[0] = ComboBoxStartupType.SelectedItem.ToString();

				foreach (ManagementObject ServiceObject in ServiceQuery.Get())
				{
					ServiceObject.InvokeMethod("ChangeStartMode", parameters);

				}
				serviceResultNode.SubItemDisplayNames[2] = ComboBoxStartupType.SelectedItem.ToString();
			}
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
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void cancelToCloseButton_Click(object sender, System.EventArgs e)
		{
			startupPropertyPage.ParentSheet.CancelToClose();
			
		}

        private void ComboBoxStartupType_SelectedIndexChanged(object sender, EventArgs e)
        {
            startupPropertyPage.Dirty = true;
        }

		#endregion Event Handlers for Controls

		#region Members

		private StartupPropertyPage startupPropertyPage = null;
		private ResultNode serviceResultNode = null;

		#endregion Members


        #region Private Controls

        private TextBox TextBoxDisplayName;
        private TextBox TextBoxStartupType;
        private GroupBox Divider;
        private ComboBox ComboBoxStartupType;

        #endregion Private Controls
    
    } // class
} // namespace
