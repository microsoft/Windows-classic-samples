//
//  <copyright file="UserPropertyAdvancedForm.Designer.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

namespace Contoso.HostedEmail.DashboardAddin
{
    partial class UserPropertyAdvancedForm
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
            if (disposing)
            {
                if (this.manager != null)
                {
                    this.manager.Dispose();
                }
                if (components != null)
                {
                    components.Dispose();
                }
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UserPropertyAdvancedForm));
            this.tlpOutter = new System.Windows.Forms.TableLayoutPanel();
            this.tlpInner = new System.Windows.Forms.TableLayoutPanel();
            this.labelTitle = new System.Windows.Forms.Label();
            this.labelEmail = new System.Windows.Forms.Label();
            this.textBoxEmail = new System.Windows.Forms.TextBox();
            this.checkBoxActiveSync = new System.Windows.Forms.CheckBox();
            this.tlpButton = new System.Windows.Forms.TableLayoutPanel();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.tlpOutter.SuspendLayout();
            this.tlpInner.SuspendLayout();
            this.tlpButton.SuspendLayout();
            this.SuspendLayout();
            // 
            // tlpOutter
            // 
            resources.ApplyResources(this.tlpOutter, "tlpOutter");
            this.tlpOutter.Controls.Add(this.tlpInner, 0, 0);
            this.tlpOutter.Controls.Add(this.tlpButton, 0, 1);
            this.tlpOutter.Name = "tlpOutter";
            // 
            // tlpInner
            // 
            resources.ApplyResources(this.tlpInner, "tlpInner");
            this.tlpInner.Controls.Add(this.labelTitle, 0, 0);
            this.tlpInner.Controls.Add(this.labelEmail, 0, 1);
            this.tlpInner.Controls.Add(this.textBoxEmail, 0, 2);
            this.tlpInner.Controls.Add(this.checkBoxActiveSync, 0, 3);
            this.tlpInner.Name = "tlpInner";
            // 
            // labelTitle
            // 
            resources.ApplyResources(this.labelTitle, "labelTitle");
            this.labelTitle.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(51)))), ((int)(((byte)(153)))));
            this.labelTitle.Name = "labelTitle";
            // 
            // labelEmail
            // 
            resources.ApplyResources(this.labelEmail, "labelEmail");
            this.labelEmail.Name = "labelEmail";
            // 
            // textBoxEmail
            // 
            resources.ApplyResources(this.textBoxEmail, "textBoxEmail");
            this.textBoxEmail.Name = "textBoxEmail";
            // 
            // checkBoxActiveSync
            // 
            resources.ApplyResources(this.checkBoxActiveSync, "checkBoxActiveSync");
            this.checkBoxActiveSync.Name = "checkBoxActiveSync";
            this.checkBoxActiveSync.UseVisualStyleBackColor = true;
            // 
            // tlpButton
            // 
            resources.ApplyResources(this.tlpButton, "tlpButton");
            this.tlpButton.Controls.Add(this.buttonCancel, 1, 0);
            this.tlpButton.Controls.Add(this.buttonOK, 0, 0);
            this.tlpButton.Name = "tlpButton";
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            resources.ApplyResources(this.buttonCancel, "buttonCancel");
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.AutoEllipsis = true;
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            resources.ApplyResources(this.buttonOK, "buttonOK");
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // UserPropertyAdvancedForm
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.tlpOutter);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "UserPropertyAdvancedForm";
            this.tlpOutter.ResumeLayout(false);
            this.tlpOutter.PerformLayout();
            this.tlpInner.ResumeLayout(false);
            this.tlpInner.PerformLayout();
            this.tlpButton.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tlpOutter;
        private System.Windows.Forms.TableLayoutPanel tlpInner;
        private System.Windows.Forms.TableLayoutPanel tlpButton;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Label labelTitle;
        private System.Windows.Forms.Label labelEmail;
        private System.Windows.Forms.TextBox textBoxEmail;
        private System.Windows.Forms.CheckBox checkBoxActiveSync;
    }
}