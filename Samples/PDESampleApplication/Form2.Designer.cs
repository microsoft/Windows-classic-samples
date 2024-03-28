namespace PDETestApp
{
    partial class Form2
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
            this.panel1 = new System.Windows.Forms.Panel();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.panel2 = new System.Windows.Forms.Panel();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.bufferOutputTextBox = new System.Windows.Forms.TextBox();
            this.button11 = new System.Windows.Forms.Button();
            this.button8 = new System.Windows.Forms.Button();
            this.bufferInputTextBox = new System.Windows.Forms.TextBox();
            this.button3 = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.button10 = new System.Windows.Forms.Button();
            this.button7 = new System.Windows.Forms.Button();
            this.button5 = new System.Windows.Forms.Button();
            this.listViewSelectedFile = new System.Windows.Forms.ListView();
            this.button2 = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.button9 = new System.Windows.Forms.Button();
            this.button6 = new System.Windows.Forms.Button();
            this.FolderSelect = new System.Windows.Forms.Button();
            this.listViewSelectedFolder = new System.Windows.Forms.ListView();
            this.button1 = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.fileBrowserDialog = new System.Windows.Forms.OpenFileDialog();
            this.folderBrowserDialog2 = new System.Windows.Forms.FolderBrowserDialog();
            this.label3 = new System.Windows.Forms.Label();
            this.panel1.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.panel2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.groupBox4);
            this.panel1.Controls.Add(this.groupBox3);
            this.panel1.Controls.Add(this.groupBox2);
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Location = new System.Drawing.Point(12, 40);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(588, 651);
            this.panel1.TabIndex = 0;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.panel2);
            this.groupBox4.Location = new System.Drawing.Point(4, 418);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(566, 230);
            this.groupBox4.TabIndex = 3;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Console";
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.textBox2);
            this.panel2.Location = new System.Drawing.Point(13, 15);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(528, 215);
            this.panel2.TabIndex = 0;
            // 
            // textBox2
            // 
            this.textBox2.BackColor = System.Drawing.SystemColors.InfoText;
            this.textBox2.ForeColor = System.Drawing.Color.Lime;
            this.textBox2.Location = new System.Drawing.Point(0, 0);
            this.textBox2.Multiline = true;
            this.textBox2.Name = "textBox2";
            this.textBox2.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox2.Size = new System.Drawing.Size(554, 217);
            this.textBox2.TabIndex = 0;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Controls.Add(this.label2);
            this.groupBox3.Controls.Add(this.bufferOutputTextBox);
            this.groupBox3.Controls.Add(this.button11);
            this.groupBox3.Controls.Add(this.button8);
            this.groupBox3.Controls.Add(this.bufferInputTextBox);
            this.groupBox3.Controls.Add(this.button3);
            this.groupBox3.Location = new System.Drawing.Point(4, 203);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(566, 208);
            this.groupBox3.TabIndex = 2;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Protect Text in memory with PDE";
            this.groupBox3.Enter += new System.EventHandler(this.groupBox3_Enter);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 118);
            this.label2.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(37, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Result";
            // 
            // bufferOutputTextBox
            // 
            this.bufferOutputTextBox.Location = new System.Drawing.Point(13, 134);
            this.bufferOutputTextBox.Multiline = true;
            this.bufferOutputTextBox.Name = "bufferOutputTextBox";
            this.bufferOutputTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.bufferOutputTextBox.Size = new System.Drawing.Size(542, 39);
            this.bufferOutputTextBox.TabIndex = 4;
            // 
            // button11
            // 
            this.button11.Location = new System.Drawing.Point(312, 179);
            this.button11.Name = "button11";
            this.button11.Size = new System.Drawing.Size(75, 23);
            this.button11.TabIndex = 3;
            this.button11.Text = "Unprotect";
            this.button11.UseVisualStyleBackColor = true;
            this.button11.Click += new System.EventHandler(this.BufferUnprotect_Click);
            // 
            // button8
            // 
            this.button8.Location = new System.Drawing.Point(231, 179);
            this.button8.Name = "button8";
            this.button8.Size = new System.Drawing.Size(75, 23);
            this.button8.TabIndex = 2;
            this.button8.Text = "Protect L2";
            this.button8.UseVisualStyleBackColor = true;
            this.button8.Click += new System.EventHandler(this.BufferL2_Click);
            // 
            // bufferInputTextBox
            // 
            this.bufferInputTextBox.Location = new System.Drawing.Point(13, 54);
            this.bufferInputTextBox.Multiline = true;
            this.bufferInputTextBox.Name = "bufferInputTextBox";
            this.bufferInputTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.bufferInputTextBox.Size = new System.Drawing.Size(542, 39);
            this.bufferInputTextBox.TabIndex = 1;
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(150, 179);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 0;
            this.button3.Text = "Protect L1";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.BufferL1_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.button10);
            this.groupBox2.Controls.Add(this.button7);
            this.groupBox2.Controls.Add(this.button5);
            this.groupBox2.Controls.Add(this.listViewSelectedFile);
            this.groupBox2.Controls.Add(this.button2);
            this.groupBox2.Location = new System.Drawing.Point(4, 111);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(566, 86);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Protect Files with PDE";
            // 
            // button10
            // 
            this.button10.Location = new System.Drawing.Point(312, 58);
            this.button10.Name = "button10";
            this.button10.Size = new System.Drawing.Size(75, 23);
            this.button10.TabIndex = 4;
            this.button10.Text = "Unprotect";
            this.button10.UseVisualStyleBackColor = true;
            this.button10.Click += new System.EventHandler(this.FileUnprotect_Click);
            // 
            // button7
            // 
            this.button7.Location = new System.Drawing.Point(231, 58);
            this.button7.Name = "button7";
            this.button7.Size = new System.Drawing.Size(75, 23);
            this.button7.TabIndex = 3;
            this.button7.Text = "Protect L2";
            this.button7.UseVisualStyleBackColor = true;
            this.button7.Click += new System.EventHandler(this.FileL2_Click);
            // 
            // button5
            // 
            this.button5.Location = new System.Drawing.Point(437, 21);
            this.button5.Margin = new System.Windows.Forms.Padding(2);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(80, 20);
            this.button5.TabIndex = 2;
            this.button5.Text = "Browse";
            this.button5.UseVisualStyleBackColor = true;
            this.button5.Click += new System.EventHandler(this.FileSelectBrowse_Click);
            // 
            // listViewSelectedFile
            // 
            this.listViewSelectedFile.Alignment = System.Windows.Forms.ListViewAlignment.Left;
            this.listViewSelectedFile.FullRowSelect = true;
            this.listViewSelectedFile.HideSelection = false;
            this.listViewSelectedFile.HoverSelection = true;
            this.listViewSelectedFile.Location = new System.Drawing.Point(13, 21);
            this.listViewSelectedFile.Margin = new System.Windows.Forms.Padding(2);
            this.listViewSelectedFile.MultiSelect = false;
            this.listViewSelectedFile.Name = "listViewSelectedFile";
            this.listViewSelectedFile.Scrollable = false;
            this.listViewSelectedFile.Size = new System.Drawing.Size(374, 23);
            this.listViewSelectedFile.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.listViewSelectedFile.TabIndex = 1;
            this.listViewSelectedFile.UseCompatibleStateImageBehavior = false;
            this.listViewSelectedFile.View = System.Windows.Forms.View.List;
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(150, 58);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 0;
            this.button2.Text = "Protect L1";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.FileL1_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.button9);
            this.groupBox1.Controls.Add(this.button6);
            this.groupBox1.Controls.Add(this.FolderSelect);
            this.groupBox1.Controls.Add(this.listViewSelectedFolder);
            this.groupBox1.Controls.Add(this.button1);
            this.groupBox1.Location = new System.Drawing.Point(4, 21);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(566, 84);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Protect Folders with PDE";
            // 
            // button9
            // 
            this.button9.Location = new System.Drawing.Point(312, 57);
            this.button9.Name = "button9";
            this.button9.Size = new System.Drawing.Size(75, 23);
            this.button9.TabIndex = 4;
            this.button9.Text = "Unprotect";
            this.button9.UseVisualStyleBackColor = true;
            this.button9.Click += new System.EventHandler(this.FolderUnprotect_Click);
            // 
            // button6
            // 
            this.button6.Location = new System.Drawing.Point(231, 57);
            this.button6.Name = "button6";
            this.button6.Size = new System.Drawing.Size(75, 23);
            this.button6.TabIndex = 3;
            this.button6.Text = "Protect L2";
            this.button6.UseVisualStyleBackColor = true;
            this.button6.Click += new System.EventHandler(this.FolderL2_Click);
            // 
            // FolderSelect
            // 
            this.FolderSelect.Location = new System.Drawing.Point(437, 18);
            this.FolderSelect.Margin = new System.Windows.Forms.Padding(2);
            this.FolderSelect.Name = "FolderSelect";
            this.FolderSelect.Size = new System.Drawing.Size(80, 23);
            this.FolderSelect.TabIndex = 2;
            this.FolderSelect.Text = "Browse";
            this.FolderSelect.UseVisualStyleBackColor = true;
            this.FolderSelect.Click += new System.EventHandler(this.FolderSelectBrowse_Click);
            // 
            // listViewSelectedFolder
            // 
            this.listViewSelectedFolder.Alignment = System.Windows.Forms.ListViewAlignment.Left;
            this.listViewSelectedFolder.FullRowSelect = true;
            this.listViewSelectedFolder.HideSelection = false;
            this.listViewSelectedFolder.HoverSelection = true;
            this.listViewSelectedFolder.Location = new System.Drawing.Point(13, 18);
            this.listViewSelectedFolder.Margin = new System.Windows.Forms.Padding(2);
            this.listViewSelectedFolder.MultiSelect = false;
            this.listViewSelectedFolder.Name = "listViewSelectedFolder";
            this.listViewSelectedFolder.Scrollable = false;
            this.listViewSelectedFolder.Size = new System.Drawing.Size(374, 23);
            this.listViewSelectedFolder.TabIndex = 1;
            this.listViewSelectedFolder.UseCompatibleStateImageBehavior = false;
            this.listViewSelectedFolder.View = System.Windows.Forms.View.List;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(150, 57);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 0;
            this.button1.Text = "Protect L1";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.FolderL1_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(16, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(256, 32);
            this.label1.TabIndex = 1;
            this.label1.Text = "PDE API Examples";
            // 
            // fileBrowserDialog
            // 
            this.fileBrowserDialog.FileName = "openFileDialog1";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(13, 35);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(149, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Input text to protect/unprotect";
            // 
            // Form2
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(615, 703);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.panel1);
            this.Name = "Form2";
            this.Text = "PDE";
            this.Load += new System.EventHandler(this.Form2_load);
            this.panel1.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox bufferInputTextBox;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog;
        private System.Windows.Forms.OpenFileDialog fileBrowserDialog;
        private System.Windows.Forms.Button FolderSelect;
        private System.Windows.Forms.ListView listViewSelectedFolder;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.ListView listViewSelectedFile;
        private System.Windows.Forms.Button button8;
        private System.Windows.Forms.Button button7;
        private System.Windows.Forms.Button button6;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog2;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.Button button10;
        private System.Windows.Forms.Button button9;
        private System.Windows.Forms.Button button11;
        private System.Windows.Forms.TextBox bufferOutputTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
    }
}