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
namespace Microsoft.ManagementConsole.Samples
{
    partial class InitializationWizard
    {
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
            this.ValueText = new System.Windows.Forms.Label();
            this.ImplementationNote = new System.Windows.Forms.Label();
            this.SnapInName = new System.Windows.Forms.TextBox();
            this.NamePrompt = new System.Windows.Forms.Label();
            this.Ok = new System.Windows.Forms.Button();
            this.panel1 = new System.Windows.Forms.Panel();
            this.Cancel = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // ValueText
            // 
            this.ValueText.AutoSize = true;
            this.ValueText.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ValueText.Location = new System.Drawing.Point(12, 18);
            this.ValueText.Name = "ValueText";
            this.ValueText.Size = new System.Drawing.Size(430, 13);
            this.ValueText.TabIndex = 0;
            this.ValueText.Text = "(This Initialization Wizard is fired when the snapin is added to the console).";
            // 
            // ImplementationNote
            // 
            this.ImplementationNote.AutoSize = true;
            this.ImplementationNote.Location = new System.Drawing.Point(25, 40);
            this.ImplementationNote.Name = "ImplementationNote";
            this.ImplementationNote.Size = new System.Drawing.Size(480, 13);
            this.ImplementationNote.TabIndex = 1;
            this.ImplementationNote.Text = "NOTE: You can override the  SnapIn.ShowInitializationWizard() handling to add a f" +
                "orm like this one.   ";
            // 
            // SnapInName
            // 
            this.SnapInName.Location = new System.Drawing.Point(178, 15);
            this.SnapInName.Name = "SnapInName";
            this.SnapInName.Size = new System.Drawing.Size(160, 20);
            this.SnapInName.TabIndex = 2;
            // 
            // NamePrompt
            // 
            this.NamePrompt.AutoSize = true;
            this.NamePrompt.Location = new System.Drawing.Point(33, 92);
            this.NamePrompt.Name = "NamePrompt";
            this.NamePrompt.Size = new System.Drawing.Size(152, 13);
            this.NamePrompt.TabIndex = 3;
            this.NamePrompt.Text = "Supply a Name for this SnapIn :";
            // 
            // Ok
            // 
            this.Ok.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Ok.Location = new System.Drawing.Point(178, 52);
            this.Ok.Name = "Ok";
            this.Ok.Size = new System.Drawing.Size(73, 23);
            this.Ok.TabIndex = 4;
            this.Ok.Text = "Ok";
            this.Ok.Click += new System.EventHandler(this.Continue_Click);
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.Cancel);
            this.panel1.Controls.Add(this.Ok);
            this.panel1.Controls.Add(this.SnapInName);
            this.panel1.Location = new System.Drawing.Point(12, 73);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(475, 100);
            this.panel1.TabIndex = 5;
            // 
            // Cancel
            // 
            this.Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Cancel.Location = new System.Drawing.Point(268, 52);
            this.Cancel.Name = "Cancel";
            this.Cancel.Size = new System.Drawing.Size(70, 23);
            this.Cancel.TabIndex = 5;
            this.Cancel.Text = "Cancel";
            // 
            // InitializationWizard
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(526, 185);
            this.Controls.Add(this.NamePrompt);
            this.Controls.Add(this.ImplementationNote);
            this.Controls.Add(this.ValueText);
            this.Controls.Add(this.panel1);
            this.Name = "InitializationWizard";
            this.Text = "SnapIn - Initialization Wizard";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label ValueText;
        private System.Windows.Forms.Label ImplementationNote;
        private System.Windows.Forms.TextBox SnapInName;
        private System.Windows.Forms.Label NamePrompt;
        private System.Windows.Forms.Button Ok;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button Cancel;
    }
}