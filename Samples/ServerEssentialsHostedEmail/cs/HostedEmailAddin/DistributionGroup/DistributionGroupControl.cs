//
//  <copyright file="DistributionGroupControl.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using Contoso.EmailService;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;
using Microsoft.WindowsServerSolutions.Common.ProviderFramework;
using Microsoft.WindowsServerSolutions.HostedEmail;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using System.Globalization;

namespace Contoso.HostedEmail.DistributionGroup
{
    public partial class DistributionGroupControl : UserControl
    {
        private FormPropertyBag propertyBag = null;
        public DistributionGroupControl(FormPropertyBag bag)
        {
            if (bag == null)
            {
                throw new ArgumentNullException("bag");
            }
            this.propertyBag = bag;
            InitializeComponent();
        }

        public string GetDistributionGroupsString()
        {
            return string.Join(Constants.ExtendedParam_DGs_Delimiter.ToString(), this.dgSelectCtrl.SelectedGroups.Select((g)=>{return g.Id;}));
        }

        #region Display
        private delegate void DisplayDelegate();
        private void ShowSplash()
        {
            if (!this.InvokeRequired)
            {
                this.tlpError.Visible = false;
                this.dgSelectCtrl.Visible = false;
                this.tlpSplash.Visible = true;
            }
            else
            {
                this.BeginInvoke(new DisplayDelegate(this.ShowSplash));
            }
        }

        private void ShowError()
        {
            if (!this.InvokeRequired)
            {
                this.tlpError.Visible = true;
                this.dgSelectCtrl.Visible = false;
                this.tlpSplash.Visible = false;
                this.pbError.Image = System.Drawing.SystemIcons.Error.ToBitmap();
            }
            else
            {
                this.BeginInvoke(new DisplayDelegate(this.ShowError));
            }
        }

        private void ShowDistributionGroups()
        {
            if (!this.InvokeRequired)
            {
                this.tlpError.Visible = false;
                this.tlpSplash.Visible = false;
                this.dgSelectCtrl.Visible = true;
            }
            else
            {
                this.BeginInvoke(new DisplayDelegate(this.ShowDistributionGroups));
            }
        }

        private void ShowNoEmailAccountAssigned()
        {
            if (!this.InvokeRequired)
            {
                this.tlpError.Visible = true;
                this.dgSelectCtrl.Visible = false;
                this.tlpSplash.Visible = false;
                this.pbError.Image = System.Drawing.SystemIcons.Warning.ToBitmap();
                this.lblError.Text = string.Format(CultureInfo.CurrentUICulture, Resources.DGTab_NoEmailAccountMsg, this.propertyBag.UserName);

            }
            else
            {
                this.BeginInvoke(new DisplayDelegate(this.ShowNoEmailAccountAssigned));
            }
        }
        #endregion

        #region Event
        public event EventHandler PropertyChanged
        {
            add { this.dgSelectCtrl.PropertyChanged += value; }
            remove { this.dgSelectCtrl.PropertyChanged -= value; }
        }
        #endregion

        #region Data Loading & Updating
        private string[] userDGs = null;
        private global::DistributionGroup[] allDGs = null;
        private string accountId = string.Empty;
        private void LoadEmailAccountInfo(string wssUserName)
        {
            HostedEmailManager manager = new HostedEmailManager(Constants.AdaptorId);
            if (!manager.Connect())
            {
                throw new InvalidOperationException("HostedEmailManager connection failed");
            }
            var info = manager.GetAccount(wssUserName);

            if (info == null || info.ExtendedProperties == null || !info.ExtendedProperties.ContainsKey(Constants.ExtendedParam_DGs))
            {
                throw new InvalidOperationException("Cannot load distribution groups that the user belongs to");
            }
            accountId = info.AccountId;
            userDGs = info.ExtendedProperties[Constants.ExtendedParam_DGs].Split(Constants.ExtendedParam_DGs_Delimiter);
        }

        private void LoadDistributionGroups()
        {
            allDGs = MockEmailService.SingleInstance.GetDistributionGroups();
        }

        private void OnDataLoaded()
        {
            if (!InvokeRequired)
            {
                this.dgSelectCtrl.SetContent(this.allDGs, this.userDGs);
                ShowDistributionGroups();
            }
            else
            {
                this.BeginInvoke(new DisplayDelegate(OnDataLoaded));
            }
        }

        internal void StartLoadingData()
        {
            ShowSplash();
            ThreadPool.QueueUserWorkItem((state) =>
                {
                    // Load email account bound with wss user
                    try
                    {
                        LoadEmailAccountInfo(this.propertyBag.UserName);
                    }
                    catch (InvalidOperationException)
                    {
                        ShowError();
                    }
                    catch (OperationInvokeException e)
                    {
                        HostedEmailProviderException hepe = e.InnerException as HostedEmailProviderException;
                        if (hepe != null && hepe.Fault == HostedEmailConfigureFault.WssUserNotAssigned)
                        {
                            ShowNoEmailAccountAssigned();
                        }
                        else
                        {
                            ShowError();
                        }
                        return;
                    }

                    // Load distribution groups
                    LoadDistributionGroups();

                    OnDataLoaded();
                });
        }

        // This method will be call in adorner's post execution task which is not in UI thread.
        internal void UpdateDistributionGroups()
        {
            if (!this.propertyBag.AddinData.ContainsKey(Constants.ExtendedParam_DGs)) return;
            Dictionary<string, string> extProps = new Dictionary<string,string>();
            extProps[Constants.ExtendedParam_DGs] = this.propertyBag.AddinData[Constants.ExtendedParam_DGs] as string;
            MockEmailService.SingleInstance.UpdateAccount(new EmailAccountInfo(extProps) { AccountId = accountId});
        }
        #endregion
    }
}
