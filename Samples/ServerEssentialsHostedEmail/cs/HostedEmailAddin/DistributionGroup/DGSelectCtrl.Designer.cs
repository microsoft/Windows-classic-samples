//
//  <copyright file="DGSelectCtrl.Designer.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

namespace Contoso.HostedEmail.DistributionGroup
{
    partial class DGSelectCtrl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DGSelectCtrl));
            this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
            this.lblNoDGs = new System.Windows.Forms.Label();
            this.btnSelectAll = new System.Windows.Forms.Button();
            this.btnClearAll = new System.Windows.Forms.Button();
            this.clbDGs = new System.Windows.Forms.CheckedListBox();
            this.tlpMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // tlpMain
            // 
            resources.ApplyResources(this.tlpMain, "tlpMain");
            this.tlpMain.Controls.Add(this.lblNoDGs, 0, 0);
            this.tlpMain.Controls.Add(this.btnSelectAll, 1, 2);
            this.tlpMain.Controls.Add(this.btnClearAll, 2, 2);
            this.tlpMain.Controls.Add(this.clbDGs, 0, 1);
            this.tlpMain.Name = "tlpMain";
            // 
            // lblNoDGs
            // 
            resources.ApplyResources(this.lblNoDGs, "lblNoDGs");
            this.tlpMain.SetColumnSpan(this.lblNoDGs, 3);
            this.lblNoDGs.Name = "lblNoDGs";
            // 
            // btnSelectAll
            // 
            resources.ApplyResources(this.btnSelectAll, "btnSelectAll");
            this.btnSelectAll.Name = "btnSelectAll";
            this.btnSelectAll.UseVisualStyleBackColor = true;
            this.btnSelectAll.Click += new System.EventHandler(this.btnSelectAll_Click);
            // 
            // btnClearAll
            // 
            resources.ApplyResources(this.btnClearAll, "btnClearAll");
            this.btnClearAll.Name = "btnClearAll";
            this.btnClearAll.UseVisualStyleBackColor = true;
            this.btnClearAll.Click += new System.EventHandler(this.btnClearAll_Click);
            // 
            // clbDGs
            // 
            this.clbDGs.CheckOnClick = true;
            this.tlpMain.SetColumnSpan(this.clbDGs, 3);
            resources.ApplyResources(this.clbDGs, "clbDGs");
            this.clbDGs.FormattingEnabled = true;
            this.clbDGs.Items.AddRange(new object[] {
            resources.GetString("clbDGs.Items"),
            resources.GetString("clbDGs.Items1"),
            resources.GetString("clbDGs.Items2")});
            this.clbDGs.Name = "clbDGs";
            this.clbDGs.ThreeDCheckBoxes = true;
            // 
            // DGSelectCtrl
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tlpMain);
            this.Name = "DGSelectCtrl";
            this.tlpMain.ResumeLayout(false);
            this.tlpMain.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tlpMain;
        private System.Windows.Forms.Button btnSelectAll;
        private System.Windows.Forms.Button btnClearAll;
        private System.Windows.Forms.CheckedListBox clbDGs;
        private System.Windows.Forms.Label lblNoDGs;
    }
}
