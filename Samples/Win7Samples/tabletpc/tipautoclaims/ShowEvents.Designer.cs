namespace TIPAutoClaims
{
    partial class ShowEvents
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
            this.m_richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // m_richTextBox1
            // 
            this.m_richTextBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.m_richTextBox1.Location = new System.Drawing.Point(0, 0);
            this.m_richTextBox1.Name = "m_richTextBox1";
            this.m_richTextBox1.Size = new System.Drawing.Size(377, 370);
            this.m_richTextBox1.TabIndex = 2;
            this.m_richTextBox1.Text = "";
            // 
            // ShowEvents
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(377, 370);
            this.Controls.Add(this.m_richTextBox1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "ShowEvents";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "TIP Event Log";
            this.Load += new System.EventHandler(this.ShowEvents_Load);
            this.ResumeLayout(false);

        }

        #endregion

        public System.Windows.Forms.RichTextBox m_richTextBox1;
    }
}