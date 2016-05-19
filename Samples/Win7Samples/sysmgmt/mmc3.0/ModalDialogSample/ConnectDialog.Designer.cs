//=======================================================================================
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
//=======================================================================================
namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// Gets name of server to connect to
    /// </summary>
    partial class ConnectDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.ConnectToPrompt = new System.Windows.Forms.Label();
            this.ConnectToServerName = new System.Windows.Forms.TextBox();
            this.Connect = new System.Windows.Forms.Button();
            this.Cancel = new System.Windows.Forms.Button();
            this.Instruction = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // ConnectToPrompt
            // 
            this.ConnectToPrompt.AutoSize = true;
            this.ConnectToPrompt.Location = new System.Drawing.Point(21, 56);
            this.ConnectToPrompt.Name = "ConnectToPrompt";
            this.ConnectToPrompt.Size = new System.Drawing.Size(160, 13);
            this.ConnectToPrompt.TabIndex = 0;
            this.ConnectToPrompt.Text = "Enter server name to Connect To";
            // 
            // ConnectToServerName
            // 
            this.ConnectToServerName.Location = new System.Drawing.Point(187, 53);
            this.ConnectToServerName.Name = "ConnectToServerName";
            this.ConnectToServerName.Size = new System.Drawing.Size(118, 20);
            this.ConnectToServerName.TabIndex = 1;
            // 
            // Connect
            // 
            this.Connect.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Connect.Location = new System.Drawing.Point(166, 93);
            this.Connect.Name = "Connect";
            this.Connect.Size = new System.Drawing.Size(75, 23);
            this.Connect.TabIndex = 2;
            this.Connect.Text = "Connect";
            // 
            // Cancel
            // 
            this.Cancel.CausesValidation = false;
            this.Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Cancel.Location = new System.Drawing.Point(247, 93);
            this.Cancel.Name = "Cancel";
            this.Cancel.Size = new System.Drawing.Size(75, 23);
            this.Cancel.TabIndex = 3;
            this.Cancel.Text = "Cancel";
            // 
            // Instruction
            // 
            this.Instruction.AutoSize = true;
            this.Instruction.Location = new System.Drawing.Point(21, 19);
            this.Instruction.Name = "Instruction";
            this.Instruction.Size = new System.Drawing.Size(391, 13);
            this.Instruction.TabIndex = 4;
            this.Instruction.Text = "(This example shows the use of a modal dialog using SnapIn.Console.ShowDialog)";
            // 
            // ConnectDialog
            // 
            this.AcceptButton = this.Connect;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.Cancel;
            this.ClientSize = new System.Drawing.Size(471, 129);
            this.Controls.Add(this.Instruction);
            this.Controls.Add(this.Cancel);
            this.Controls.Add(this.Connect);
            this.Controls.Add(this.ConnectToServerName);
            this.Controls.Add(this.ConnectToPrompt);
            this.Name = "ConnectDialog";
            this.Text = "Connect To ...";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label ConnectToPrompt;
        private System.Windows.Forms.Button Connect;
        private System.Windows.Forms.Button Cancel;
        private System.Windows.Forms.Label Instruction;

        /// <summary>
        /// Name of Server
        /// </summary>
        public System.Windows.Forms.TextBox ConnectToServerName;
    }
}