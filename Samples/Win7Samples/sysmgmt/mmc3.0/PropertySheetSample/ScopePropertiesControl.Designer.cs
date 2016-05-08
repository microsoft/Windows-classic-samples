namespace Microsoft.ManagementConsole.Samples
{
    partial class ScopePropertiesControl
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
            this.DisplayName = new System.Windows.Forms.TextBox();
            this.DisplayNamePrompt = new System.Windows.Forms.Label();
            this.ScopeInfo = new System.Windows.Forms.GroupBox();
            this.SuspendLayout();
            // 
            // DisplayName
            // 
            this.DisplayName.Location = new System.Drawing.Point(118, 41);
            this.DisplayName.Name = "DisplayName";
            this.DisplayName.Size = new System.Drawing.Size(136, 20);
            this.DisplayName.TabIndex = 6;
            this.DisplayName.TextChanged += new System.EventHandler(this.DisplayName_TextChanged);
            // 
            // DisplayNamePrompt
            // 
            this.DisplayNamePrompt.Location = new System.Drawing.Point(38, 41);
            this.DisplayNamePrompt.Name = "DisplayNamePrompt";
            this.DisplayNamePrompt.Size = new System.Drawing.Size(72, 23);
            this.DisplayNamePrompt.TabIndex = 5;
            this.DisplayNamePrompt.Text = "Display Name";
            // 
            // ScopeInfo
            // 
            this.ScopeInfo.Location = new System.Drawing.Point(14, 17);
            this.ScopeInfo.Name = "ScopeInfo";
            this.ScopeInfo.Size = new System.Drawing.Size(264, 100);
            this.ScopeInfo.TabIndex = 9;
            this.ScopeInfo.TabStop = false;
            this.ScopeInfo.Text = "Scope Node";
            // 
            // ScopePropertiesControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.DisplayName);
            this.Controls.Add(this.DisplayNamePrompt);
            this.Controls.Add(this.ScopeInfo);
            this.Name = "ScopePropertiesControl";
            this.Size = new System.Drawing.Size(302, 132);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox DisplayName;
        private System.Windows.Forms.Label DisplayNamePrompt;
        private System.Windows.Forms.GroupBox ScopeInfo;
    }
}
