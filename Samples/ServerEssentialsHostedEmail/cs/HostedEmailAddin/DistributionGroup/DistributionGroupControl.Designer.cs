//
//  <copyright file="DistributionGroupControl.Designer.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

namespace Contoso.HostedEmail.DistributionGroup
{
    partial class DistributionGroupControl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DistributionGroupControl));
            this.tlpMain = new System.Windows.Forms.TableLayoutPanel();
            this.tlpSplash = new System.Windows.Forms.TableLayoutPanel();
            this.lblLoading = new System.Windows.Forms.Label();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.tlpError = new System.Windows.Forms.TableLayoutPanel();
            this.pbError = new System.Windows.Forms.PictureBox();
            this.lblError = new System.Windows.Forms.Label();
            this.dgSelectCtrl = new Contoso.HostedEmail.DistributionGroup.DGSelectCtrl();
            this.tlpMain.SuspendLayout();
            this.tlpSplash.SuspendLayout();
            this.tlpError.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbError)).BeginInit();
            this.SuspendLayout();
            // 
            // tlpMain
            // 
            resources.ApplyResources(this.tlpMain, "tlpMain");
            this.tlpMain.Controls.Add(this.tlpSplash, 0, 0);
            this.tlpMain.Controls.Add(this.tlpError, 0, 1);
            this.tlpMain.Controls.Add(this.dgSelectCtrl, 0, 2);
            this.tlpMain.Name = "tlpMain";
            // 
            // tlpSplash
            // 
            resources.ApplyResources(this.tlpSplash, "tlpSplash");
            this.tlpSplash.Controls.Add(this.lblLoading, 0, 0);
            this.tlpSplash.Controls.Add(this.progressBar, 0, 1);
            this.tlpSplash.Name = "tlpSplash";
            // 
            // lblLoading
            // 
            resources.ApplyResources(this.lblLoading, "lblLoading");
            this.lblLoading.Name = "lblLoading";
            // 
            // progressBar
            // 
            resources.ApplyResources(this.progressBar, "progressBar");
            this.progressBar.Name = "progressBar";
            this.progressBar.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
            // 
            // tlpError
            // 
            resources.ApplyResources(this.tlpError, "tlpError");
            this.tlpError.Controls.Add(this.pbError, 0, 0);
            this.tlpError.Controls.Add(this.lblError, 1, 0);
            this.tlpError.Name = "tlpError";
            // 
            // pbError
            // 
            resources.ApplyResources(this.pbError, "pbError");
            this.pbError.Name = "pbError";
            this.pbError.TabStop = false;
            // 
            // lblError
            // 
            resources.ApplyResources(this.lblError, "lblError");
            this.lblError.Name = "lblError";
            // 
            // dgSelectCtrl
            // 
            resources.ApplyResources(this.dgSelectCtrl, "dgSelectCtrl");
            this.dgSelectCtrl.Name = "dgSelectCtrl";
            // 
            // DistributionGroupControl
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tlpMain);
            this.Name = "DistributionGroupControl";
            this.tlpMain.ResumeLayout(false);
            this.tlpMain.PerformLayout();
            this.tlpSplash.ResumeLayout(false);
            this.tlpSplash.PerformLayout();
            this.tlpError.ResumeLayout(false);
            this.tlpError.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbError)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tlpMain;
        private System.Windows.Forms.TableLayoutPanel tlpSplash;
        private System.Windows.Forms.Label lblLoading;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.TableLayoutPanel tlpError;
        private System.Windows.Forms.PictureBox pbError;
        private System.Windows.Forms.Label lblError;
        private DGSelectCtrl dgSelectCtrl;
    }
}
