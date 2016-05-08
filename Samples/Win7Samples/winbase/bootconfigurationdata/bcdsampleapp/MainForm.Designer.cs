namespace Microsoft.Samples.BcdSampleApp
{
    partial class MainForm
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
			this.btnBackupStore = new System.Windows.Forms.Button();
			this.txtNewStorePath = new System.Windows.Forms.TextBox();
			this.gbBackup = new System.Windows.Forms.GroupBox();
			this.label2 = new System.Windows.Forms.Label();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label1 = new System.Windows.Forms.Label();
			this.txtImportStorePath = new System.Windows.Forms.TextBox();
			this.btnImportStore = new System.Windows.Forms.Button();
			this.gbDeleteOsEntry = new System.Windows.Forms.GroupBox();
			this.label3 = new System.Windows.Forms.Label();
			this.txtOSEntryToDelete = new System.Windows.Forms.TextBox();
			this.btnDeleteOsEntry = new System.Windows.Forms.Button();
			this.gbSystemEntries = new System.Windows.Forms.GroupBox();
			this.lstSystemStoreEntries = new System.Windows.Forms.ListBox();
			this.btnViewSystemEntries = new System.Windows.Forms.Button();
			this.gbEnumerate = new System.Windows.Forms.GroupBox();
			this.rbShowLegacy = new System.Windows.Forms.RadioButton();
			this.rbShowVista = new System.Windows.Forms.RadioButton();
			this.lstStoreObjects = new System.Windows.Forms.ListBox();
			this.btnEnumStoreObjs = new System.Windows.Forms.Button();
			this.gbCreateNewOsEntry = new System.Windows.Forms.GroupBox();
			this.chkAddToOSList = new System.Windows.Forms.CheckBox();
			this.rbCreateLegacyEntry = new System.Windows.Forms.RadioButton();
			this.label4 = new System.Windows.Forms.Label();
			this.rbCreateVistaEntry = new System.Windows.Forms.RadioButton();
			this.txtNewEntryDescription = new System.Windows.Forms.TextBox();
			this.btnCreateOSEntry = new System.Windows.Forms.Button();
			this.gbClone = new System.Windows.Forms.GroupBox();
			this.label6 = new System.Windows.Forms.Label();
			this.btnRefreshCloneList = new System.Windows.Forms.Button();
			this.txtCloneNewDescription = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.lstCurrentDisplayList = new System.Windows.Forms.ListBox();
			this.btnClone = new System.Windows.Forms.Button();
			this.btnCopyClipboard = new System.Windows.Forms.Button();
			this.btnViewDescription = new System.Windows.Forms.Button();
			this.btnShowAllByTypeDescrip = new System.Windows.Forms.Button();
			this.gbBackup.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.gbDeleteOsEntry.SuspendLayout();
			this.gbSystemEntries.SuspendLayout();
			this.gbEnumerate.SuspendLayout();
			this.gbCreateNewOsEntry.SuspendLayout();
			this.gbClone.SuspendLayout();
			this.SuspendLayout();
			// 
			// btnBackupStore
			// 
			this.btnBackupStore.Location = new System.Drawing.Point(32, 74);
			this.btnBackupStore.Name = "btnBackupStore";
			this.btnBackupStore.Size = new System.Drawing.Size(283, 34);
			this.btnBackupStore.TabIndex = 0;
			this.btnBackupStore.Text = "Backup System Store to non-system Store";
			this.btnBackupStore.UseVisualStyleBackColor = true;
			this.btnBackupStore.Click += new System.EventHandler(this.btnBackupStore_Click);
			// 
			// txtNewStorePath
			// 
			this.txtNewStorePath.Location = new System.Drawing.Point(118, 36);
			this.txtNewStorePath.Name = "txtNewStorePath";
			this.txtNewStorePath.Size = new System.Drawing.Size(197, 23);
			this.txtNewStorePath.TabIndex = 1;
			// 
			// gbBackup
			// 
			this.gbBackup.Controls.Add(this.label2);
			this.gbBackup.Controls.Add(this.txtNewStorePath);
			this.gbBackup.Controls.Add(this.btnBackupStore);
			this.gbBackup.Location = new System.Drawing.Point(12, 31);
			this.gbBackup.Name = "gbBackup";
			this.gbBackup.Size = new System.Drawing.Size(403, 124);
			this.gbBackup.TabIndex = 0;
			this.gbBackup.TabStop = false;
			this.gbBackup.Text = "Backup";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(29, 39);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(64, 15);
			this.label2.TabIndex = 5;
			this.label2.Text = "Store Path:";
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.txtImportStorePath);
			this.groupBox1.Controls.Add(this.btnImportStore);
			this.groupBox1.Location = new System.Drawing.Point(12, 163);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(403, 134);
			this.groupBox1.TabIndex = 1;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Restore Backup/Import to System Store";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(29, 39);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(64, 15);
			this.label1.TabIndex = 4;
			this.label1.Text = "Store Path:";
			// 
			// txtImportStorePath
			// 
			this.txtImportStorePath.Location = new System.Drawing.Point(118, 36);
			this.txtImportStorePath.Name = "txtImportStorePath";
			this.txtImportStorePath.Size = new System.Drawing.Size(197, 23);
			this.txtImportStorePath.TabIndex = 1;
			// 
			// btnImportStore
			// 
			this.btnImportStore.Location = new System.Drawing.Point(32, 74);
			this.btnImportStore.Name = "btnImportStore";
			this.btnImportStore.Size = new System.Drawing.Size(283, 44);
			this.btnImportStore.TabIndex = 0;
			this.btnImportStore.Text = "Import non-system store settings into System Store (overwriting current System St" +
				"ore)";
			this.btnImportStore.UseVisualStyleBackColor = true;
			this.btnImportStore.Click += new System.EventHandler(this.btnImportStore_Click);
			// 
			// gbDeleteOsEntry
			// 
			this.gbDeleteOsEntry.Controls.Add(this.label3);
			this.gbDeleteOsEntry.Controls.Add(this.txtOSEntryToDelete);
			this.gbDeleteOsEntry.Controls.Add(this.btnDeleteOsEntry);
			this.gbDeleteOsEntry.Location = new System.Drawing.Point(12, 321);
			this.gbDeleteOsEntry.Name = "gbDeleteOsEntry";
			this.gbDeleteOsEntry.Size = new System.Drawing.Size(403, 115);
			this.gbDeleteOsEntry.TabIndex = 2;
			this.gbDeleteOsEntry.TabStop = false;
			this.gbDeleteOsEntry.Text = "Delete OS Entry from System Store";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(29, 39);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(68, 15);
			this.label3.TabIndex = 4;
			this.label3.Text = "OS Entry Id:";
			// 
			// txtOSEntryToDelete
			// 
			this.txtOSEntryToDelete.Location = new System.Drawing.Point(118, 36);
			this.txtOSEntryToDelete.Name = "txtOSEntryToDelete";
			this.txtOSEntryToDelete.Size = new System.Drawing.Size(197, 23);
			this.txtOSEntryToDelete.TabIndex = 1;
			// 
			// btnDeleteOsEntry
			// 
			this.btnDeleteOsEntry.Location = new System.Drawing.Point(93, 73);
			this.btnDeleteOsEntry.Name = "btnDeleteOsEntry";
			this.btnDeleteOsEntry.Size = new System.Drawing.Size(171, 30);
			this.btnDeleteOsEntry.TabIndex = 0;
			this.btnDeleteOsEntry.Text = "Delete OS Entry By Id";
			this.btnDeleteOsEntry.UseVisualStyleBackColor = true;
			this.btnDeleteOsEntry.Click += new System.EventHandler(this.btnDeleteOsEntry_Click);
			// 
			// gbSystemEntries
			// 
			this.gbSystemEntries.Controls.Add(this.btnViewDescription);
			this.gbSystemEntries.Controls.Add(this.btnCopyClipboard);
			this.gbSystemEntries.Controls.Add(this.lstSystemStoreEntries);
			this.gbSystemEntries.Controls.Add(this.btnViewSystemEntries);
			this.gbSystemEntries.Location = new System.Drawing.Point(12, 454);
			this.gbSystemEntries.Name = "gbSystemEntries";
			this.gbSystemEntries.Size = new System.Drawing.Size(403, 326);
			this.gbSystemEntries.TabIndex = 3;
			this.gbSystemEntries.TabStop = false;
			this.gbSystemEntries.Text = "OS Entries in System Store By Id - Displayed List Items";
			// 
			// lstSystemStoreEntries
			// 
			this.lstSystemStoreEntries.FormattingEnabled = true;
			this.lstSystemStoreEntries.ItemHeight = 15;
			this.lstSystemStoreEntries.Location = new System.Drawing.Point(6, 38);
			this.lstSystemStoreEntries.Name = "lstSystemStoreEntries";
			this.lstSystemStoreEntries.ScrollAlwaysVisible = true;
			this.lstSystemStoreEntries.Size = new System.Drawing.Size(391, 124);
			this.lstSystemStoreEntries.TabIndex = 7;
			// 
			// btnViewSystemEntries
			// 
			this.btnViewSystemEntries.Location = new System.Drawing.Point(32, 180);
			this.btnViewSystemEntries.Name = "btnViewSystemEntries";
			this.btnViewSystemEntries.Size = new System.Drawing.Size(244, 30);
			this.btnViewSystemEntries.TabIndex = 0;
			this.btnViewSystemEntries.Text = "View System Store Displayed OS Entries";
			this.btnViewSystemEntries.UseVisualStyleBackColor = true;
			this.btnViewSystemEntries.Click += new System.EventHandler(this.btnViewSystemEntries_Click);
			// 
			// gbEnumerate
			// 
			this.gbEnumerate.Controls.Add(this.btnShowAllByTypeDescrip);
			this.gbEnumerate.Controls.Add(this.rbShowLegacy);
			this.gbEnumerate.Controls.Add(this.rbShowVista);
			this.gbEnumerate.Controls.Add(this.lstStoreObjects);
			this.gbEnumerate.Controls.Add(this.btnEnumStoreObjs);
			this.gbEnumerate.Location = new System.Drawing.Point(435, 578);
			this.gbEnumerate.Name = "gbEnumerate";
			this.gbEnumerate.Size = new System.Drawing.Size(377, 329);
			this.gbEnumerate.TabIndex = 6;
			this.gbEnumerate.TabStop = false;
			this.gbEnumerate.Text = "Enumerate Store Objects (including those not in display list)";
			// 
			// rbShowLegacy
			// 
			this.rbShowLegacy.AutoSize = true;
			this.rbShowLegacy.Location = new System.Drawing.Point(84, 59);
			this.rbShowLegacy.Name = "rbShowLegacy";
			this.rbShowLegacy.Size = new System.Drawing.Size(100, 19);
			this.rbShowLegacy.TabIndex = 9;
			this.rbShowLegacy.Text = "Legacy Entries";
			this.rbShowLegacy.UseVisualStyleBackColor = true;
			// 
			// rbShowVista
			// 
			this.rbShowVista.AutoSize = true;
			this.rbShowVista.Checked = true;
			this.rbShowVista.Location = new System.Drawing.Point(84, 36);
			this.rbShowVista.Name = "rbShowVista";
			this.rbShowVista.Size = new System.Drawing.Size(88, 19);
			this.rbShowVista.TabIndex = 8;
			this.rbShowVista.TabStop = true;
			this.rbShowVista.Text = "Vista Entries";
			this.rbShowVista.UseVisualStyleBackColor = true;
			// 
			// lstStoreObjects
			// 
			this.lstStoreObjects.FormattingEnabled = true;
			this.lstStoreObjects.ItemHeight = 15;
			this.lstStoreObjects.Location = new System.Drawing.Point(27, 96);
			this.lstStoreObjects.Name = "lstStoreObjects";
			this.lstStoreObjects.ScrollAlwaysVisible = true;
			this.lstStoreObjects.Size = new System.Drawing.Size(255, 124);
			this.lstStoreObjects.TabIndex = 7;
			// 
			// btnEnumStoreObjs
			// 
			this.btnEnumStoreObjs.Location = new System.Drawing.Point(27, 238);
			this.btnEnumStoreObjs.Name = "btnEnumStoreObjs";
			this.btnEnumStoreObjs.Size = new System.Drawing.Size(274, 30);
			this.btnEnumStoreObjs.TabIndex = 0;
			this.btnEnumStoreObjs.Text = "Show All Store Objects of Type by Guid";
			this.btnEnumStoreObjs.UseVisualStyleBackColor = true;
			this.btnEnumStoreObjs.Click += new System.EventHandler(this.btnEnumStoreObjs_Click);
			// 
			// gbCreateNewOsEntry
			// 
			this.gbCreateNewOsEntry.Controls.Add(this.chkAddToOSList);
			this.gbCreateNewOsEntry.Controls.Add(this.rbCreateLegacyEntry);
			this.gbCreateNewOsEntry.Controls.Add(this.label4);
			this.gbCreateNewOsEntry.Controls.Add(this.rbCreateVistaEntry);
			this.gbCreateNewOsEntry.Controls.Add(this.txtNewEntryDescription);
			this.gbCreateNewOsEntry.Controls.Add(this.btnCreateOSEntry);
			this.gbCreateNewOsEntry.Location = new System.Drawing.Point(435, 357);
			this.gbCreateNewOsEntry.Name = "gbCreateNewOsEntry";
			this.gbCreateNewOsEntry.Size = new System.Drawing.Size(377, 205);
			this.gbCreateNewOsEntry.TabIndex = 5;
			this.gbCreateNewOsEntry.TabStop = false;
			this.gbCreateNewOsEntry.Text = "Create New OS Entry";
			// 
			// chkAddToOSList
			// 
			this.chkAddToOSList.AutoSize = true;
			this.chkAddToOSList.Checked = true;
			this.chkAddToOSList.CheckState = System.Windows.Forms.CheckState.Checked;
			this.chkAddToOSList.Location = new System.Drawing.Point(84, 88);
			this.chkAddToOSList.Name = "chkAddToOSList";
			this.chkAddToOSList.Size = new System.Drawing.Size(234, 19);
			this.chkAddToOSList.TabIndex = 12;
			this.chkAddToOSList.Text = "Add to OS Display List (Recommended)";
			this.chkAddToOSList.UseVisualStyleBackColor = true;
			// 
			// rbCreateLegacyEntry
			// 
			this.rbCreateLegacyEntry.AutoSize = true;
			this.rbCreateLegacyEntry.Location = new System.Drawing.Point(120, 49);
			this.rbCreateLegacyEntry.Name = "rbCreateLegacyEntry";
			this.rbCreateLegacyEntry.Size = new System.Drawing.Size(129, 19);
			this.rbCreateLegacyEntry.TabIndex = 11;
			this.rbCreateLegacyEntry.Text = "Create Legacy Entry";
			this.rbCreateLegacyEntry.UseVisualStyleBackColor = true;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(30, 122);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(70, 15);
			this.label4.TabIndex = 4;
			this.label4.Text = "Description:";
			// 
			// rbCreateVistaEntry
			// 
			this.rbCreateVistaEntry.AutoSize = true;
			this.rbCreateVistaEntry.Checked = true;
			this.rbCreateVistaEntry.Location = new System.Drawing.Point(120, 26);
			this.rbCreateVistaEntry.Name = "rbCreateVistaEntry";
			this.rbCreateVistaEntry.Size = new System.Drawing.Size(117, 19);
			this.rbCreateVistaEntry.TabIndex = 10;
			this.rbCreateVistaEntry.TabStop = true;
			this.rbCreateVistaEntry.Text = "Create Vista Entry";
			this.rbCreateVistaEntry.UseVisualStyleBackColor = true;
			// 
			// txtNewEntryDescription
			// 
			this.txtNewEntryDescription.Location = new System.Drawing.Point(106, 119);
			this.txtNewEntryDescription.Name = "txtNewEntryDescription";
			this.txtNewEntryDescription.Size = new System.Drawing.Size(246, 23);
			this.txtNewEntryDescription.TabIndex = 1;
			// 
			// btnCreateOSEntry
			// 
			this.btnCreateOSEntry.Location = new System.Drawing.Point(94, 156);
			this.btnCreateOSEntry.Name = "btnCreateOSEntry";
			this.btnCreateOSEntry.Size = new System.Drawing.Size(171, 30);
			this.btnCreateOSEntry.TabIndex = 0;
			this.btnCreateOSEntry.Text = "Create OS Entry";
			this.btnCreateOSEntry.UseVisualStyleBackColor = true;
			this.btnCreateOSEntry.Click += new System.EventHandler(this.btnCreateOSEntry_Click);
			// 
			// gbClone
			// 
			this.gbClone.Controls.Add(this.label6);
			this.gbClone.Controls.Add(this.btnRefreshCloneList);
			this.gbClone.Controls.Add(this.txtCloneNewDescription);
			this.gbClone.Controls.Add(this.label5);
			this.gbClone.Controls.Add(this.lstCurrentDisplayList);
			this.gbClone.Controls.Add(this.btnClone);
			this.gbClone.Location = new System.Drawing.Point(435, 31);
			this.gbClone.Name = "gbClone";
			this.gbClone.Size = new System.Drawing.Size(377, 298);
			this.gbClone.TabIndex = 4;
			this.gbClone.TabStop = false;
			this.gbClone.Text = "Clone OS Entry";
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(30, 198);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(97, 15);
			this.label6.TabIndex = 10;
			this.label6.Text = "New Description:";
			// 
			// btnRefreshCloneList
			// 
			this.btnRefreshCloneList.Location = new System.Drawing.Point(239, 15);
			this.btnRefreshCloneList.Name = "btnRefreshCloneList";
			this.btnRefreshCloneList.Size = new System.Drawing.Size(79, 30);
			this.btnRefreshCloneList.TabIndex = 14;
			this.btnRefreshCloneList.Text = "Refresh";
			this.btnRefreshCloneList.UseVisualStyleBackColor = true;
			this.btnRefreshCloneList.Click += new System.EventHandler(this.btnRefreshCloneList_Click);
			// 
			// txtCloneNewDescription
			// 
			this.txtCloneNewDescription.Location = new System.Drawing.Point(132, 195);
			this.txtCloneNewDescription.Name = "txtCloneNewDescription";
			this.txtCloneNewDescription.Size = new System.Drawing.Size(198, 23);
			this.txtCloneNewDescription.TabIndex = 9;
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(22, 23);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(161, 15);
			this.label5.TabIndex = 13;
			this.label5.Text = "Current Boot Displayed Items";
			// 
			// lstCurrentDisplayList
			// 
			this.lstCurrentDisplayList.FormattingEnabled = true;
			this.lstCurrentDisplayList.ItemHeight = 15;
			this.lstCurrentDisplayList.Location = new System.Drawing.Point(46, 50);
			this.lstCurrentDisplayList.Name = "lstCurrentDisplayList";
			this.lstCurrentDisplayList.ScrollAlwaysVisible = true;
			this.lstCurrentDisplayList.Size = new System.Drawing.Size(255, 124);
			this.lstCurrentDisplayList.TabIndex = 7;
			// 
			// btnClone
			// 
			this.btnClone.Location = new System.Drawing.Point(57, 236);
			this.btnClone.Name = "btnClone";
			this.btnClone.Size = new System.Drawing.Size(244, 30);
			this.btnClone.TabIndex = 0;
			this.btnClone.Text = "Clone Selected Entry";
			this.btnClone.UseVisualStyleBackColor = true;
			this.btnClone.Click += new System.EventHandler(this.btnClone_Click);
			// 
			// btnCopyClipboard
			// 
			this.btnCopyClipboard.Location = new System.Drawing.Point(32, 252);
			this.btnCopyClipboard.Name = "btnCopyClipboard";
			this.btnCopyClipboard.Size = new System.Drawing.Size(244, 30);
			this.btnCopyClipboard.TabIndex = 8;
			this.btnCopyClipboard.Text = "Copy Selected ListItem to Clipboard";
			this.btnCopyClipboard.UseVisualStyleBackColor = true;
			this.btnCopyClipboard.Click += new System.EventHandler(this.btnCopyClipboard_Click);
			// 
			// btnViewDescription
			// 
			this.btnViewDescription.Location = new System.Drawing.Point(32, 216);
			this.btnViewDescription.Name = "btnViewDescription";
			this.btnViewDescription.Size = new System.Drawing.Size(244, 30);
			this.btnViewDescription.TabIndex = 9;
			this.btnViewDescription.Text = "View OS Entry Descriptions";
			this.btnViewDescription.UseVisualStyleBackColor = true;
			this.btnViewDescription.Click += new System.EventHandler(this.btnViewDescription_Click);
			// 
			// btnShowAllByTypeDescrip
			// 
			this.btnShowAllByTypeDescrip.Location = new System.Drawing.Point(27, 274);
			this.btnShowAllByTypeDescrip.Name = "btnShowAllByTypeDescrip";
			this.btnShowAllByTypeDescrip.Size = new System.Drawing.Size(274, 30);
			this.btnShowAllByTypeDescrip.TabIndex = 10;
			this.btnShowAllByTypeDescrip.Text = "Show All Store Objects of Type by Description";
			this.btnShowAllByTypeDescrip.UseVisualStyleBackColor = true;
			this.btnShowAllByTypeDescrip.Click += new System.EventHandler(this.btnShowAllByTypeDescrip_Click);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.AutoScroll = true;
			this.ClientSize = new System.Drawing.Size(817, 938);
			this.Controls.Add(this.gbClone);
			this.Controls.Add(this.gbCreateNewOsEntry);
			this.Controls.Add(this.gbEnumerate);
			this.Controls.Add(this.gbSystemEntries);
			this.Controls.Add(this.gbDeleteOsEntry);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.gbBackup);
			this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.Name = "MainForm";
			this.Text = "Bcd Sample Application - example usages of BcdSampleLib";
			this.gbBackup.ResumeLayout(false);
			this.gbBackup.PerformLayout();
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.gbDeleteOsEntry.ResumeLayout(false);
			this.gbDeleteOsEntry.PerformLayout();
			this.gbSystemEntries.ResumeLayout(false);
			this.gbEnumerate.ResumeLayout(false);
			this.gbEnumerate.PerformLayout();
			this.gbCreateNewOsEntry.ResumeLayout(false);
			this.gbCreateNewOsEntry.PerformLayout();
			this.gbClone.ResumeLayout(false);
			this.gbClone.PerformLayout();
			this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnBackupStore;
		private System.Windows.Forms.TextBox txtNewStorePath;
		private System.Windows.Forms.GroupBox gbBackup;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtImportStorePath;
		private System.Windows.Forms.Button btnImportStore;
		private System.Windows.Forms.GroupBox gbDeleteOsEntry;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox txtOSEntryToDelete;
		private System.Windows.Forms.Button btnDeleteOsEntry;
		private System.Windows.Forms.GroupBox gbSystemEntries;
		private System.Windows.Forms.ListBox lstSystemStoreEntries;
		private System.Windows.Forms.Button btnViewSystemEntries;
		private System.Windows.Forms.GroupBox gbEnumerate;
		private System.Windows.Forms.ListBox lstStoreObjects;
		private System.Windows.Forms.Button btnEnumStoreObjs;
		private System.Windows.Forms.RadioButton rbShowLegacy;
		private System.Windows.Forms.RadioButton rbShowVista;
		private System.Windows.Forms.GroupBox gbCreateNewOsEntry;
		private System.Windows.Forms.RadioButton rbCreateLegacyEntry;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.RadioButton rbCreateVistaEntry;
		private System.Windows.Forms.TextBox txtNewEntryDescription;
		private System.Windows.Forms.Button btnCreateOSEntry;
		private System.Windows.Forms.CheckBox chkAddToOSList;
		private System.Windows.Forms.GroupBox gbClone;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.ListBox lstCurrentDisplayList;
		private System.Windows.Forms.Button btnClone;
		private System.Windows.Forms.Button btnRefreshCloneList;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TextBox txtCloneNewDescription;
		private System.Windows.Forms.Button btnCopyClipboard;
		private System.Windows.Forms.Button btnViewDescription;
		private System.Windows.Forms.Button btnShowAllByTypeDescrip;
    }
}

