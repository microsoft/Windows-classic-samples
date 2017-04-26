namespace RadialControllerWinForms
{
    partial class Form1
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
            this.MenuSuppressionCheckBox = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // MenuSuppressionCheckBox
            //
            this.MenuSuppressionCheckBox.AutoSize = true;
            this.MenuSuppressionCheckBox.Location = new System.Drawing.Point(13, 13);
            this.MenuSuppressionCheckBox.Name = "MenuSuppressionCheckBox";
            this.MenuSuppressionCheckBox.Size = new System.Drawing.Size(178, 21);
            this.MenuSuppressionCheckBox.TabIndex = 0;
            this.MenuSuppressionCheckBox.Text = "Suppress Default Menu";
            this.MenuSuppressionCheckBox.UseVisualStyleBackColor = true;
            this.MenuSuppressionCheckBox.CheckedChanged += new System.EventHandler(this.MenuSuppressionCheckBox_CheckedChanged);
            //
            // Form1
            //
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(455, 410);
            this.Controls.Add(this.MenuSuppressionCheckBox);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "Form1";
            this.Text = "Radial Controller C# Classic Sample";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox MenuSuppressionCheckBox;
    }
}

