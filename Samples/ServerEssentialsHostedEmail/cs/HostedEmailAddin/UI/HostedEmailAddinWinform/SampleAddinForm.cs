//
//  <copyright file="SampleAddinForm.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Globalization;
using System.Threading;
using System.Windows.Forms;
using System.IO;
using Microsoft.WindowsServerSolutions.HostedEmail;
using Microsoft.WindowsServerSolutions.Controls;
using Microsoft.WindowsServerSolutions.Common;
using System.Drawing;
using Contoso.EmailService;

namespace Contoso.HostedEmail.DashboardAddin
{

    public partial class SampleAddinForm : UserControl
    {
        private HostedEmailManager manager = new HostedEmailManager(HostedEmailIntegrationManager.EnabledAddinId);
        private string[] domains = null;
        private EmailAccountInfo[] emailAccountsInfo = null;

        public SampleAddinForm()
        {
            InitializeComponent();
            LoadIntegratedOrUnintegratedForm();
        }

        private static bool ContosoEmailEnabled
        {
            get
            {
                return (HostedEmailIntegrationManager.IsEnabled() && HostedEmailIntegrationManager.EnabledAddinId.Equals(Constants.AdaptorId));
            }
        }

        private void LoadIntegratedOrUnintegratedForm()
        {
            this.SuspendLayout();

            Uri serviceWebsite = new Uri(Resources.ContosoServices_DefaultWebUrl);
            string phoneNumber = Resources.ContosoServices_DefaultPhoneNumber;
            string supportEmail = Resources.ContosoServices_DefaultSupportEmail;
            string companyName = Resources.ContosoServices_DefaultCompanyName;
            string serviceName = Resources.ContosoServices_DefaultServiceName;

            if (!ContosoEmailEnabled)
            {
                tableLayoutPanelIntegration.Visible = false;
                tableLayoutPanelNotIntegration.Visible = true;
                tableLayoutPanelSubscription.Visible = false;

                //set tableLayoutPanelIntegration's ColumnStyle to AutoSize, so it could auto-hide.
                int colPosition = tableLayoutPanelTop.GetColumn(tableLayoutPanelIntegration);
                tableLayoutPanelTop.ColumnStyles[colPosition].SizeType = SizeType.AutoSize;

                tableLayoutPanelTop.Controls.Remove(tableLayoutPanelIntegration);
                tableLayoutPanelTop.Controls.Remove(tableLayoutPanelSubscription);
            }
            else
            {
                tableLayoutPanelIntegration.Visible = true;
                tableLayoutPanelNotIntegration.Visible = false;
                tableLayoutPanelSubscription.Visible = true;

                //set tableLayoutPanelNotIntegration's ColumnStyle to AutoSize, so it could auto-hide.
                int colPosition = tableLayoutPanelTop.GetColumn(tableLayoutPanelNotIntegration);
                tableLayoutPanelTop.ColumnStyles[colPosition].SizeType = SizeType.AutoSize;
                tableLayoutPanelTop.Controls.Remove(tableLayoutPanelNotIntegration);

                labelSigninAccount.Text = Resources.ContosoServices_DefaultAdminAccount;
                try
                {
                    HostedEmailServiceInfo info = HostedEmailIntegrationManager.Configuration.Service;
                    if (info != null)
                    {
                        HostedEmailSupportContactInfo ContosoEmailSupport = info.SupportContact;
                        if (ContosoEmailSupport != null)
                        {
                            phoneNumber = ContosoEmailSupport.PhoneNumber;
                            supportEmail = ContosoEmailSupport.EmailAddress;
                        }
                        if (info.Provider.Name != null)
                        {
                            companyName = info.Provider.Name;
                            serviceWebsite = info.Provider.Website;
                        }
                        if (info.ShortName != null)
                        {
                            serviceName = info.ShortName;
                        }
                        if (CredentialManager.AdminUserName != null)
                        {
                            labelSigninAccount.Text = CredentialManager.AdminUserName;
                        }
                    }

                }
                catch (Exception)
                {
                    //Do nothing, use default value.
                }

                //Show notice for users when addin retrieving data
                UpdateDomains(true, false);
                UpdateMailBoxNumber(true, false);

                //retrieving data
                ThreadPool.QueueUserWorkItem((state) =>
                {
                    if (manager.Connect())
                    {
                        Manager_ConnectCompleted();
                    }
                    else
                    {
                        Manager_ConnectFailed();
                    }
                });
            }

            dashboardLinkLabelContosoURL.LinkUrl = serviceWebsite;
            dashboardLinkLabelContosoURL.Text = serviceWebsite.ToString();
            labelPhoneNumber.Text = phoneNumber;
            labelCompanyName.Text = companyName;
            dashboardLinkLabelContosoEmail.Text = supportEmail;
            dashboardLinkLabelContosoEmail.LinkUrl = new Uri("mailto:" + supportEmail);
            if (ContosoEmailEnabled)
            {
                labelServerIsIntegratedDescription.Text = string.Format(CultureInfo.CurrentCulture, Resources.ContosoServices_IntegratedDescription, serviceName);
            }
            else
            {
                labelServerNotIntegratedDescription.Text = string.Format(CultureInfo.CurrentCulture, Resources.ContosoServices_NotIntegratedDescription, serviceName);
            }

            this.ResumeLayout();
        }

        private void Manager_ConnectCompleted()
        {
            manager.HostedEmailAccountUpdated += OnEmailAccountUpdated;
            QueryDomainNames();
            QueryMailboxNumber();
        }

        private void Manager_ConnectFailed()
        {
            UpdateDomains(false, true);
            UpdateMailBoxNumber(false, true);
        }

        private void OnEmailAccountUpdated(object sender, HostedEmailAccountUpdatedEventArgs e)
        {
            QueryMailboxNumber();
        }

        private void QueryDomainNames()
        {
            manager.BeginGetDomains((sender, e) =>
            {
                if (e.Error != null)
                {
                    UpdateDomains(false, true);
                }
                else
                {
                    domains = e.Result;
                    UpdateDomains(false, false);
                }
            });
        }

        private void QueryMailboxNumber()
        {
            manager.BeginGetAllAccounts((sender, e) =>
            {
                if (e.Error != null)
                {
                    UpdateMailBoxNumber(false, true);
                }
                else
                {
                    emailAccountsInfo = e.Result;
                    UpdateMailBoxNumber(false, false);
                }
            });
        }

        private void UpdateDomains(bool inProgress, bool errorOccurred)
        {
            if (InvokeRequired)
            {
                this.Invoke(
                    new Action(() =>
                    {
                        UpdateDomains(inProgress, errorOccurred);
                    }));
            }
            else
            {
                if (errorOccurred)
                {
                    labelFirstEmailDomain.Text = Resources.ContosoServicesSubTab_ErrorDescription;
                }
                else if (inProgress)
                {
                    labelFirstEmailDomain.Text = Resources.ContosoServicesSubTab_LoadingDescription;

                }
                else
                {
                    labelFirstEmailDomain.Text = "";

                    if (domains != null && domains.Length > 0)
                    {
                        labelFirstEmailDomain.Text = string.Join(Environment.NewLine, domains);
                    }
                }
                labelFirstEmailDomain.Visible = true;

            }
        }

        private void UpdateMailBoxNumber(bool inProgress, bool errorOccurred)
        {
            if (InvokeRequired)
            {
                this.Invoke(
                    new Action(() =>
                    {
                        UpdateMailBoxNumber(inProgress, errorOccurred);
                    }));
            }
            else
            {
                if (errorOccurred)
                {
                    labelMailNumber.Text = Resources.ContosoServicesSubTab_ErrorDescription;
                }
                else if (inProgress)
                {
                    labelMailNumber.Text = Resources.ContosoServicesSubTab_LoadingDescription;
                }
                else
                {
                    labelMailNumber.Text = string.Format(CultureInfo.CurrentCulture, Resources.ContosoServicesSubTab_MailNumberDataString, emailAccountsInfo.Length);

                }
                labelMailNumber.Visible = true;
            }
        }

        private void dashboardLinkLabelClickToIntegrate_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            string configureWizardRelativePath = "Wssg.HostedEmailConfigureWizard.vbs";
            System.Diagnostics.Process.Start(configureWizardRelativePath);
        }

        private void drawLineBetweenTwoLabels(Label up, Label down, PaintEventArgs e)
        {
            int x1 = up.Location.X;
            int y1 = up.Location.Y + up.Height;
            int y2 = down.Location.Y;
            int y = (y1 + y2) / 2;
            int end_x = x1 + up.Width;
            int start_x = x1;

            Pen pen = new Pen(SystemColors.ControlText, 1);
            Point point1 = new Point(start_x, y);
            Point point2 = new Point(end_x, y);
            e.Graphics.DrawLine(pen, point1, point2);
        }

        private void drawLineBetweenTwoLabels(Label up, Label down, int lineWidth, PaintEventArgs e)
        {
            int x1 = up.Location.X;
            int y1 = up.Location.Y + up.Height;
            int y2 = down.Location.Y;
            int y = (y1 + y2) / 2;
            int end_x = x1 + lineWidth;
            int start_x = x1;

            Pen pen = new Pen(SystemColors.ControlText, 1);
            Point point1 = new Point(start_x, y);
            Point point2 = new Point(end_x, y);
            e.Graphics.DrawLine(pen, point1, point2);
        }

        private void tableLayoutPanelServiceProvider_Paint(object sender, PaintEventArgs e)
        {
            drawLineBetweenTwoLabels(labelServiceProvider, labelServiceName, tableLayoutPanelServiceProvider.Width, e);
        }

        private void tableLayoutPanelIntegration_Paint(object sender, PaintEventArgs e)
        {
            drawLineBetweenTwoLabels(labelServerIntegratedTitle, labelServerIsIntegratedDescription, tableLayoutPanelIntegration.Width, e);
        }

        private void tableLayoutPanelNotIntegration_Paint(object sender, PaintEventArgs e)
        {
            drawLineBetweenTwoLabels(labelServerNotIntegrationTitle, labelServerNotIntegratedDescription, tableLayoutPanelNotIntegration.Width, e);
        }

        private void tableLayoutPanelSubscription_Paint(object sender, PaintEventArgs e)
        {
            //Subscription panel has same line width with ServiceProvider panel
            drawLineBetweenTwoLabels(labelSubscription, labelTotalMailboxes, tableLayoutPanelServiceProvider.Width, e);
        }

    }
}