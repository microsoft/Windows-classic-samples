//
//  <copyright file="DGSelectCtrl.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System.Windows.Forms;
using System.Linq;
using System;
using Contoso.EmailService;
using System.Collections.ObjectModel;

namespace Contoso.HostedEmail.DistributionGroup
{
    public partial class DGSelectCtrl : UserControl
    {
        public DGSelectCtrl()
        {
            InitializeComponent();
        }

        public void SetContent(global::DistributionGroup[] groups, string[] selectedIds)
        {
            if (groups == null || groups.Length == 0)
            {
                this.clbDGs.Visible = false;
                this.btnClearAll.Enabled = false;
                this.btnSelectAll.Enabled = false;
            }
            else
            {
                this.lblNoDGs.Visible = false;
                this.clbDGs.Items.Clear();
                this.clbDGs.SuspendLayout();
                foreach (var group in groups)
                {
                    this.clbDGs.Items.Add(group, selectedIds.Contains(group.Id));
                }
                // specify the property name to display, it's "Name" in this case
                this.clbDGs.DisplayMember = "Name";
                this.clbDGs.ResumeLayout(true);
                UpdateButtonState(null);
                this.clbDGs.ItemCheck += (object sender, ItemCheckEventArgs e) =>
                    {
                        UpdateButtonState(e);
                        RaisePropertyChange();
                    };
            }
        }

        public ReadOnlyCollection<global::DistributionGroup> SelectedGroups
        {
            get
            {
                return this.clbDGs.CheckedItems.Cast<global::DistributionGroup>().ToList().AsReadOnly();
            }
        }

        private void UpdateButtonState(ItemCheckEventArgs e)
        {
            var checkedCount = GetCheckedIndices(e);
            this.btnSelectAll.Enabled = this.clbDGs.Items.Count > checkedCount;
            this.btnClearAll.Enabled = checkedCount > 0;
        }

        private void btnSelectAll_Click(object sender, System.EventArgs e)
        {
            CheckAllItems(true);
        }

        private void btnClearAll_Click(object sender, System.EventArgs e)
        {
            CheckAllItems(false);
        }

        private void CheckAllItems(bool beChecked)
        {
            for (int i = 0; i < this.clbDGs.Items.Count; ++i) this.clbDGs.SetItemChecked(i, beChecked);
        }

        // ItemCheck event of CheckboxListView occurs before the check state change, therefore, we should count the changing one in event handler
        private int GetCheckedIndices(ItemCheckEventArgs e)
        {
            var count = this.clbDGs.CheckedIndices.Count;
            if (e == null) return count;
            var difference = (e.NewValue == CheckState.Checked) ? 1 : -1;
            return count + difference;
        }

        #region Event
        public event EventHandler PropertyChanged;

        private void RaisePropertyChange()
        {
            var tmpHandler = PropertyChanged;
            if (tmpHandler != null) tmpHandler(this, null);
        }
        #endregion
    }
}
