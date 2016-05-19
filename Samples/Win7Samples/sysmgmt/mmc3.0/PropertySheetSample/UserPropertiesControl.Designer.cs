namespace Microsoft.ManagementConsole.Samples
{
    partial class UserPropertiesControl
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
            this.Birthday = new System.Windows.Forms.TextBox();
            this.BirthdayPrompt = new System.Windows.Forms.Label();
            this.UserName = new System.Windows.Forms.TextBox();
            this.UserNamePrompt = new System.Windows.Forms.Label();
            this.UserInfo = new System.Windows.Forms.GroupBox();
            this.SuspendLayout();
            // 
            // Birthday
            // 
            this.Birthday.Location = new System.Drawing.Point(118, 73);
            this.Birthday.Name = "Birthday";
            this.Birthday.Size = new System.Drawing.Size(136, 20);
            this.Birthday.TabIndex = 8;
            this.Birthday.TextChanged += new System.EventHandler(this.Birthday_TextChanged);
            // 
            // BirthdayPrompt
            // 
            this.BirthdayPrompt.Location = new System.Drawing.Point(38, 73);
            this.BirthdayPrompt.Name = "BirthdayPrompt";
            this.BirthdayPrompt.Size = new System.Drawing.Size(72, 23);
            this.BirthdayPrompt.TabIndex = 7;
            this.BirthdayPrompt.Text = "Birthday";
            // 
            // UserName
            // 
            this.UserName.Location = new System.Drawing.Point(118, 41);
            this.UserName.Name = "UserName";
            this.UserName.Size = new System.Drawing.Size(136, 20);
            this.UserName.TabIndex = 6;
            this.UserName.TextChanged += new System.EventHandler(this.UserName_TextChanged);
            // 
            // UserNamePrompt
            // 
            this.UserNamePrompt.Location = new System.Drawing.Point(38, 41);
            this.UserNamePrompt.Name = "UserNamePrompt";
            this.UserNamePrompt.Size = new System.Drawing.Size(72, 23);
            this.UserNamePrompt.TabIndex = 5;
            this.UserNamePrompt.Text = "Name";
            // 
            // UserInfo
            // 
            this.UserInfo.Location = new System.Drawing.Point(14, 17);
            this.UserInfo.Name = "UserInfo";
            this.UserInfo.Size = new System.Drawing.Size(264, 100);
            this.UserInfo.TabIndex = 9;
            this.UserInfo.TabStop = false;
            this.UserInfo.Text = "User";
            // 
            // PropertiesControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.Birthday);
            this.Controls.Add(this.BirthdayPrompt);
            this.Controls.Add(this.UserName);
            this.Controls.Add(this.UserNamePrompt);
            this.Controls.Add(this.UserInfo);
            this.Name = "PropertiesControl";
            this.Size = new System.Drawing.Size(302, 132);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox Birthday;
        private System.Windows.Forms.Label BirthdayPrompt;
        private System.Windows.Forms.TextBox UserName;
        private System.Windows.Forms.Label UserNamePrompt;
        private System.Windows.Forms.GroupBox UserInfo;
    }
}
