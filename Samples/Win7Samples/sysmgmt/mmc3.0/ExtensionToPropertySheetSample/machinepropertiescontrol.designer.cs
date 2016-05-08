namespace Microsoft.ManagementConsole.Samples
{
    partial class MachinePropertiesControl
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
            this.MachineName = new System.Windows.Forms.TextBox();
            this.MachineNamePrompt = new System.Windows.Forms.Label();
            this.ComputerInfo = new System.Windows.Forms.GroupBox();
            this.ComputerInfo.SuspendLayout();
            this.SuspendLayout();
            // 
            // MachineName
            // 
            this.MachineName.Location = new System.Drawing.Point(113, 24);
            this.MachineName.Name = "MachineName";
            this.MachineName.Size = new System.Drawing.Size(136, 20);
            this.MachineName.TabIndex = 6;
            this.MachineName.TextChanged += new System.EventHandler(this.MachineName_TextChanged);
            // 
            // MachineNamePrompt
            // 
            this.MachineNamePrompt.Location = new System.Drawing.Point(41, 41);
            this.MachineNamePrompt.Name = "MachineNamePrompt";
            this.MachineNamePrompt.Size = new System.Drawing.Size(80, 23);
            this.MachineNamePrompt.TabIndex = 5;
            this.MachineNamePrompt.Text = "Machine Name";
            // 
            // ComputerInfo
            // 
            this.ComputerInfo.Controls.Add(this.MachineName);
            this.ComputerInfo.Location = new System.Drawing.Point(14, 17);
            this.ComputerInfo.Name = "ComputerInfo";
            this.ComputerInfo.Size = new System.Drawing.Size(264, 100);
            this.ComputerInfo.TabIndex = 9;
            this.ComputerInfo.TabStop = false;
            this.ComputerInfo.Text = "Computer Info";
            // 
            // MachinePropertiesControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.MachineNamePrompt);
            this.Controls.Add(this.ComputerInfo);
            this.Name = "MachinePropertiesControl";
            this.Size = new System.Drawing.Size(302, 132);
            this.ComputerInfo.ResumeLayout(false);
            this.ComputerInfo.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label MachineNamePrompt;
        private System.Windows.Forms.GroupBox ComputerInfo;
        /// <summary>
        /// 
        /// </summary>
        public System.Windows.Forms.TextBox MachineName;
    }
}
