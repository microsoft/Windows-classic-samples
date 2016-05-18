// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


// note please read the readme.txt file for notes on escalation/LUA 
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Samples.Utils;
using System.Management;
using ROOT.WMI;
using Microsoft.Samples.BcdSampleLib;

namespace Microsoft.Samples.BcdSampleApp
{
    public partial class MainForm : Form
    {

        public MainForm()
        {

            InitializeComponent();

        }

		private void btnBackupStore_Click(object sender, EventArgs e)
        {

			string newStorePath = this.txtNewStorePath.Text.Trim();
			if (newStorePath == string.Empty)
			{
				MessageBox.Show("Please enter correct path for a backup store");
			}

			bool success = BcdStore_API.BackupSystemStoreToFile(newStorePath);
			if (success)
			{
				MessageBox.Show("System store successfully backed up.!");
			}
			else
			{
				MessageBox.Show("Some failure occurred backing up system store.");
			}
  
        }

		private void btnImportStore_Click(object sender, EventArgs e)
		{

			string newStorePath = this.txtImportStorePath.Text.Trim();
			if (newStorePath == string.Empty)
			{
				MessageBox.Show("Please enter correct path for a backup store");
			}
			 
			bool success = BcdStore_API.RestoreSystemStoreFromBackupFile(newStorePath);
			if (success)
			{
				MessageBox.Show("Successfully imported (restored) from non-system saved store file!");
			}
			else
			{
				MessageBox.Show("Some failure occurred restoring.");
			}

		}

		private void btnDeleteOsEntry_Click(object sender, EventArgs e)
		{

			string osEntryId = this.txtOSEntryToDelete.Text.Trim();
			if (osEntryId.Substring(0, 1) != "{")
			{
				MessageBox.Show("Must surround guid for os entry with curly braces '{' and '}' ");
				return;
			}
			if (osEntryId.Trim().Length == 0)
			{
				MessageBox.Show("os entry is blank");
				return;
			}
			if (MessageBox.Show("Are you sure you wish to delete this os entry?", "", 
					MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button2) != DialogResult.Yes)
				return;

			bool success = BcdStore_API.DeleteOsEntry(osEntryId);

			if (success)
			{

				// clean out any guid references in the display list, to avoid a COM error for invalid guid list item in display list pointing to
				// a guid which no longer exists in the store.

				List<string> guidDisplayList = BcdStore_API.GetBootManagerGUIDEntriesDisplayListAsList();
				bool badOneInDisplayList = false;
				List<string> moddedList = new List<string>();
				foreach (string guid in guidDisplayList)
				{
					if (guid == osEntryId)
					{
						badOneInDisplayList = true;
					}
					else
						moddedList.Add(guid);
				}
				if (badOneInDisplayList)
				{
					string errorDetails;

					success = BcdStore_API.SetOSDisplayListGuids(moddedList.ToArray() , "", out errorDetails);
					if (!success)
					{
						MessageBox.Show("deletion went thru okay, but trouble removing some of the guid references in the display list");
						return;
					}

				}

			}
			else
				MessageBox.Show("failure to delete");

			MessageBox.Show("successful deletion");

		}

		private void btnViewSystemEntries_Click(object sender, EventArgs e)
		{

			ListBox lb = this.lstSystemStoreEntries;

			FillListBoxWithCurrentOSListDisplay(lb);

		}

		private void FillListBoxWithCurrentOSListDisplay(ListBox lb)
		{
			FillListBoxWithCurrentOSListDisplay(lb, false);
		}

		private void FillListBoxWithCurrentOSListDisplay(ListBox lb, bool viewDescrips)
		{

			List<string> guidList = BcdStore_API.GetBootManagerGUIDEntriesDisplayListAsList();
			if (guidList == null)
				MessageBox.Show("trouble getting guid list");

			lb.Items.Clear();

			foreach (string guid in guidList)
			{
				string descrip = BcdStore_API.GetDescriptionForGuid(guid, "");
				if (viewDescrips)
				{
					if (descrip == null)
					{
						descrip = guid; // if no description default to showing guid, this is how the actual boot loader does it also
					}
				}

				if ( ! viewDescrips )
					lb.Items.Add(guid);
				else
					lb.Items.Add(descrip);

			}

		}

		private void btnEnumStoreObjs_Click(object sender, EventArgs e)
		{

			UInt32 lookupType;

			if (this.rbShowVista.Checked)
			{
				lookupType = Constants.BCDE_VISTA_OS_ENTRY;
			}
			else
			{
				lookupType = Constants.BCDE_LEGACY_OS_ENTRY;
			}

			List<string> guidList = BcdStore_API.EnumerateObjectsByType(lookupType, "");

			if ( guidList != null )
			{

				this.lstStoreObjects.Items.Clear();
				foreach (string guid in guidList)
				{
					this.lstStoreObjects.Items.Add(guid);
				}
			}

		}

		private void btnCreateOSEntry_Click(object sender, EventArgs e)
		{

			string newDescrip = txtNewEntryDescription.Text.Trim();

			if (newDescrip == string.Empty)
			{
				MessageBox.Show("Please enter a Description for this new OS entry");
				return;
			}

			BcdStore_API.OSEntryTypes entryType;

			if ( this.rbCreateLegacyEntry.Checked )
				entryType = BcdStore_API.OSEntryTypes.Legacy;
			else
				entryType = BcdStore_API.OSEntryTypes.Vista;

			string newGuid;
			bool success = BcdStore_API.CreateNewOSEntry(entryType, "", out newGuid);

			if (!success)
			{
				MessageBox.Show("trouble creating new os entry object and adding to the store");
				return;
			}

			success = BcdStore_API.ChangeOSEntryDescription(newGuid, newDescrip, "");
			if (!success)
			{
				MessageBox.Show("trouble naming description for os entry, will have no description (will default to GUID in oslist typically)");
				// continue don't return here, try to add to oslist anyway
			}

			// add new item to displayed os list (this is a separate operation to the add to the store, which does not add to the
			// os display list by default).
			if (this.chkAddToOSList.Checked)
			{
				string errorDetails;
				bool displayListAddOkay = BcdStore_API.AddGuidToOSDisplayList(newGuid, "", out errorDetails);
				if (!displayListAddOkay)
				{
					MessageBox.Show(String.Format("trouble adding newly created OS entry to OS display list, error msg:{0}", errorDetails));
					return;

				}
			}
			MessageBox.Show("new (empty) OS entry of designated type successfully added to store");

		}


		private void btnClone_Click(object sender, EventArgs e)
		{

			ListBox lb = this.lstCurrentDisplayList;
			if ( this.lstCurrentDisplayList.Items.Count == 0)
			{
				
				FillListBoxWithCurrentOSListDisplay(lb);
				if ( this.lstCurrentDisplayList.Items.Count == 0)
					MessageBox.Show("no current boot items found, error of some kind has occurred");
				else
					MessageBox.Show("select item to clone and retry");

				return;
			}

			string clonedItem = this.lstCurrentDisplayList.SelectedItem.ToString();

			string newDescription = this.txtCloneNewDescription.Text.Trim();

			string errorDetails;

			bool success = BcdStore_API.CloneExistingOSEntry(clonedItem, newDescription, "", out  errorDetails);
			if (!success)
			{
				MessageBox.Show("error cloning item, details: " + errorDetails);
				return;
			}
			else
			{
				MessageBox.Show("successfully cloned item");
			}


			FillListBoxWithCurrentOSListDisplay(lb);

		}

		private void btnRefreshCloneList_Click(object sender, EventArgs e)
		{

			ListBox lb = this.lstCurrentDisplayList;
			FillListBoxWithCurrentOSListDisplay(lb);

		}

		private void btnCopyClipboard_Click(object sender, EventArgs e)
		{

			ListBox lb = this.lstSystemStoreEntries;
			if (lb.Items.Count == 0)
			{
				MessageBox.Show("no items in the listbox");
			}

			if (lb.SelectedItem == null)
			{
				return;
			}

			Clipboard.SetDataObject(lb.SelectedItem.ToString(), true);

		}

		private void btnViewDescription_Click(object sender, EventArgs e)
		{

			ListBox lb = this.lstSystemStoreEntries;

			FillListBoxWithCurrentOSListDisplay(lb, true);

		}

		private void btnShowAllByTypeDescrip_Click(object sender, EventArgs e)
		{


			UInt32 lookupType;

			if (this.rbShowVista.Checked)
			{
				lookupType = Constants.BCDE_VISTA_OS_ENTRY;
			}
			else
			{
				lookupType = Constants.BCDE_LEGACY_OS_ENTRY;
			}

			List<string> guidList = BcdStore_API.EnumerateObjectsByType(lookupType, "");

			if (guidList != null)
			{

				this.lstStoreObjects.Items.Clear();
				foreach (string guid in guidList)
				{
					string descrip = BcdStore_API.GetDescriptionForGuid(guid, "");
					if (descrip == null) descrip = guid;
					this.lstStoreObjects.Items.Add(descrip);
				}
			}

		}


    }
}