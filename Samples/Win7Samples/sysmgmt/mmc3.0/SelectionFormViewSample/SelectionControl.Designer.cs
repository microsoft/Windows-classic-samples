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
    /// Form container for a ListView control 
    /// </summary>
    partial class SelectionControl
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.UserListView = new System.Windows.Forms.ListView();         
            this.SuspendLayout();
            // 
            // UserListView
            // 
            this.UserListView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.UserListView.FullRowSelect = true;
            this.UserListView.GridLines = true;
            this.UserListView.Location = new System.Drawing.Point(15, 17);
            this.UserListView.Name = "UserListView";
            this.UserListView.Size = new System.Drawing.Size(121, 115);
            this.UserListView.TabIndex = 0;
            this.UserListView.View = System.Windows.Forms.View.Details;
            this.UserListView.MouseClick += new System.Windows.Forms.MouseEventHandler(this.UserListView_MouseClick);
            this.UserListView.SelectedIndexChanged += new System.EventHandler(this.UserListView_SelectedIndexChanged);
            
            // 
            // SelectionControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.UserListView);
            this.Name = "SelectionControl";          
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView UserListView;      
    }
}
