//
//  <copyright file="SampleAddinForm.Designer.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

namespace Contoso.HostedEmail.DashboardAddin
{
    partial class SampleAddinForm
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
                this.manager.Dispose();
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SampleAddinForm));
            this.labelServerIntegratedTitle = new System.Windows.Forms.Label();
            this.labelSigninAccount = new System.Windows.Forms.Label();
            this.labelSigninAccountDescription = new System.Windows.Forms.Label();
            this.labelServerIsIntegratedDescription = new System.Windows.Forms.Label();
            this.tableLayoutPanelTop = new System.Windows.Forms.TableLayoutPanel();
            this.tableLayoutPanelSubscription = new System.Windows.Forms.TableLayoutPanel();
            this.labelTotalMailboxes = new System.Windows.Forms.Label();
            this.labelEmailDomains = new System.Windows.Forms.Label();
            this.labelSubscription = new System.Windows.Forms.Label();
            this.labelMailNumber = new System.Windows.Forms.Label();
            this.labelFirstEmailDomain = new System.Windows.Forms.Label();
            this.tableLayoutPanelIntegration = new System.Windows.Forms.TableLayoutPanel();
            this.tableLayoutPanelNotIntegration = new System.Windows.Forms.TableLayoutPanel();
            this.labelServerNotIntegrationTitle = new System.Windows.Forms.Label();
            this.labelServerNotIntegratedDescription = new System.Windows.Forms.Label();
            this.dashboardLinkLabelClickToIntegrate = new Microsoft.WindowsServerSolutions.Controls.DashboardLinkLabel();
            this.labelUserTab = new System.Windows.Forms.Label();
            this.tableLayoutPanelServiceProvider = new System.Windows.Forms.TableLayoutPanel();
            this.labelServiceProvider = new System.Windows.Forms.Label();
            this.labelServiceName = new System.Windows.Forms.Label();
            this.labelCompany = new System.Windows.Forms.Label();
            this.labelCompanyName = new System.Windows.Forms.Label();
            this.labelSupport = new System.Windows.Forms.Label();
            this.labelPhoneNumber = new System.Windows.Forms.Label();
            this.dashboardLinkLabelContosoURL = new Microsoft.WindowsServerSolutions.Controls.DashboardLinkLabel();
            this.dashboardLinkLabelContosoEmail = new Microsoft.WindowsServerSolutions.Controls.DashboardLinkLabel();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.panelTop = new System.Windows.Forms.Panel();
            this.tableLayoutPanelTop.SuspendLayout();
            this.tableLayoutPanelSubscription.SuspendLayout();
            this.tableLayoutPanelIntegration.SuspendLayout();
            this.tableLayoutPanelNotIntegration.SuspendLayout();
            this.tableLayoutPanelServiceProvider.SuspendLayout();
            this.panelTop.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelServerIntegratedTitle
            // 
            resources.ApplyResources(this.labelServerIntegratedTitle, "labelServerIntegratedTitle");
            this.tableLayoutPanelIntegration.SetColumnSpan(this.labelServerIntegratedTitle, 2);
            this.labelServerIntegratedTitle.Name = "labelServerIntegratedTitle";
            // 
            // labelSigninAccount
            // 
            resources.ApplyResources(this.labelSigninAccount, "labelSigninAccount");
            this.labelSigninAccount.Name = "labelSigninAccount";
            // 
            // labelSigninAccountDescription
            // 
            resources.ApplyResources(this.labelSigninAccountDescription, "labelSigninAccountDescription");
            this.tableLayoutPanelIntegration.SetColumnSpan(this.labelSigninAccountDescription, 2);
            this.labelSigninAccountDescription.Name = "labelSigninAccountDescription";
            // 
            // labelServerIsIntegratedDescription
            // 
            resources.ApplyResources(this.labelServerIsIntegratedDescription, "labelServerIsIntegratedDescription");
            this.tableLayoutPanelIntegration.SetColumnSpan(this.labelServerIsIntegratedDescription, 2);
            this.labelServerIsIntegratedDescription.Name = "labelServerIsIntegratedDescription";
            // 
            // tableLayoutPanelTop
            // 
            resources.ApplyResources(this.tableLayoutPanelTop, "tableLayoutPanelTop");
            this.tableLayoutPanelTop.Controls.Add(this.tableLayoutPanelSubscription, 0, 5);
            this.tableLayoutPanelTop.Controls.Add(this.tableLayoutPanelIntegration, 2, 3);
            this.tableLayoutPanelTop.Controls.Add(this.tableLayoutPanelNotIntegration, 3, 3);
            this.tableLayoutPanelTop.Controls.Add(this.labelUserTab, 0, 1);
            this.tableLayoutPanelTop.Controls.Add(this.tableLayoutPanelServiceProvider, 0, 3);
            this.tableLayoutPanelTop.Name = "tableLayoutPanelTop";
            // 
            // tableLayoutPanelSubscription
            // 
            resources.ApplyResources(this.tableLayoutPanelSubscription, "tableLayoutPanelSubscription");
            this.tableLayoutPanelTop.SetColumnSpan(this.tableLayoutPanelSubscription, 5);
            this.tableLayoutPanelSubscription.Controls.Add(this.labelTotalMailboxes, 0, 2);
            this.tableLayoutPanelSubscription.Controls.Add(this.labelEmailDomains, 0, 3);
            this.tableLayoutPanelSubscription.Controls.Add(this.labelSubscription, 0, 0);
            this.tableLayoutPanelSubscription.Controls.Add(this.labelMailNumber, 2, 2);
            this.tableLayoutPanelSubscription.Controls.Add(this.labelFirstEmailDomain, 1, 4);
            this.tableLayoutPanelSubscription.GrowStyle = System.Windows.Forms.TableLayoutPanelGrowStyle.FixedSize;
            this.tableLayoutPanelSubscription.Name = "tableLayoutPanelSubscription";
            this.tableLayoutPanelSubscription.Paint += new System.Windows.Forms.PaintEventHandler(this.tableLayoutPanelSubscription_Paint);
            // 
            // labelTotalMailboxes
            // 
            resources.ApplyResources(this.labelTotalMailboxes, "labelTotalMailboxes");
            this.tableLayoutPanelSubscription.SetColumnSpan(this.labelTotalMailboxes, 2);
            this.labelTotalMailboxes.Name = "labelTotalMailboxes";
            // 
            // labelEmailDomains
            // 
            resources.ApplyResources(this.labelEmailDomains, "labelEmailDomains");
            this.tableLayoutPanelSubscription.SetColumnSpan(this.labelEmailDomains, 3);
            this.labelEmailDomains.Name = "labelEmailDomains";
            // 
            // labelSubscription
            // 
            resources.ApplyResources(this.labelSubscription, "labelSubscription");
            this.tableLayoutPanelSubscription.SetColumnSpan(this.labelSubscription, 3);
            this.labelSubscription.Name = "labelSubscription";
            // 
            // labelMailNumber
            // 
            resources.ApplyResources(this.labelMailNumber, "labelMailNumber");
            this.labelMailNumber.Name = "labelMailNumber";
            // 
            // labelFirstEmailDomain
            // 
            resources.ApplyResources(this.labelFirstEmailDomain, "labelFirstEmailDomain");
            this.tableLayoutPanelSubscription.SetColumnSpan(this.labelFirstEmailDomain, 2);
            this.labelFirstEmailDomain.Name = "labelFirstEmailDomain";
            // 
            // tableLayoutPanelIntegration
            // 
            resources.ApplyResources(this.tableLayoutPanelIntegration, "tableLayoutPanelIntegration");
            this.tableLayoutPanelIntegration.Controls.Add(this.labelServerIsIntegratedDescription, 0, 2);
            this.tableLayoutPanelIntegration.Controls.Add(this.labelServerIntegratedTitle, 0, 0);
            this.tableLayoutPanelIntegration.Controls.Add(this.labelSigninAccountDescription, 0, 3);
            this.tableLayoutPanelIntegration.Controls.Add(this.labelSigninAccount, 1, 4);
            this.tableLayoutPanelIntegration.GrowStyle = System.Windows.Forms.TableLayoutPanelGrowStyle.FixedSize;
            this.tableLayoutPanelIntegration.Name = "tableLayoutPanelIntegration";
            this.tableLayoutPanelIntegration.Paint += new System.Windows.Forms.PaintEventHandler(this.tableLayoutPanelIntegration_Paint);
            // 
            // tableLayoutPanelNotIntegration
            // 
            resources.ApplyResources(this.tableLayoutPanelNotIntegration, "tableLayoutPanelNotIntegration");
            this.tableLayoutPanelNotIntegration.Controls.Add(this.labelServerNotIntegrationTitle, 0, 0);
            this.tableLayoutPanelNotIntegration.Controls.Add(this.labelServerNotIntegratedDescription, 0, 2);
            this.tableLayoutPanelNotIntegration.Controls.Add(this.dashboardLinkLabelClickToIntegrate, 0, 3);
            this.tableLayoutPanelNotIntegration.GrowStyle = System.Windows.Forms.TableLayoutPanelGrowStyle.FixedSize;
            this.tableLayoutPanelNotIntegration.Name = "tableLayoutPanelNotIntegration";
            this.tableLayoutPanelNotIntegration.Paint += new System.Windows.Forms.PaintEventHandler(this.tableLayoutPanelNotIntegration_Paint);
            // 
            // labelServerNotIntegrationTitle
            // 
            resources.ApplyResources(this.labelServerNotIntegrationTitle, "labelServerNotIntegrationTitle");
            this.labelServerNotIntegrationTitle.Name = "labelServerNotIntegrationTitle";
            // 
            // labelServerNotIntegratedDescription
            // 
            resources.ApplyResources(this.labelServerNotIntegratedDescription, "labelServerNotIntegratedDescription");
            this.labelServerNotIntegratedDescription.Name = "labelServerNotIntegratedDescription";
            // 
            // dashboardLinkLabelClickToIntegrate
            // 
            this.dashboardLinkLabelClickToIntegrate.ActiveLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(51)))), ((int)(((byte)(152)))));
            resources.ApplyResources(this.dashboardLinkLabelClickToIntegrate, "dashboardLinkLabelClickToIntegrate");
            this.dashboardLinkLabelClickToIntegrate.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.dashboardLinkLabelClickToIntegrate.LinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(102)))), ((int)(((byte)(255)))));
            this.dashboardLinkLabelClickToIntegrate.LinkUrl = null;
            this.dashboardLinkLabelClickToIntegrate.Name = "dashboardLinkLabelClickToIntegrate";
            this.dashboardLinkLabelClickToIntegrate.TabStop = true;
            this.dashboardLinkLabelClickToIntegrate.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.dashboardLinkLabelClickToIntegrate_LinkClicked);
            // 
            // labelUserTab
            // 
            resources.ApplyResources(this.labelUserTab, "labelUserTab");
            this.tableLayoutPanelTop.SetColumnSpan(this.labelUserTab, 4);
            this.labelUserTab.Name = "labelUserTab";
            // 
            // tableLayoutPanelServiceProvider
            // 
            resources.ApplyResources(this.tableLayoutPanelServiceProvider, "tableLayoutPanelServiceProvider");
            this.tableLayoutPanelServiceProvider.Controls.Add(this.labelServiceProvider, 0, 0);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.labelServiceName, 0, 2);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.labelCompany, 0, 3);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.labelCompanyName, 1, 4);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.labelSupport, 0, 6);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.labelPhoneNumber, 1, 7);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.dashboardLinkLabelContosoURL, 1, 5);
            this.tableLayoutPanelServiceProvider.Controls.Add(this.dashboardLinkLabelContosoEmail, 1, 8);
            this.tableLayoutPanelServiceProvider.Name = "tableLayoutPanelServiceProvider";
            this.tableLayoutPanelServiceProvider.Paint += new System.Windows.Forms.PaintEventHandler(this.tableLayoutPanelServiceProvider_Paint);
            // 
            // labelServiceProvider
            // 
            resources.ApplyResources(this.labelServiceProvider, "labelServiceProvider");
            this.tableLayoutPanelServiceProvider.SetColumnSpan(this.labelServiceProvider, 2);
            this.labelServiceProvider.Name = "labelServiceProvider";
            // 
            // labelServiceName
            // 
            resources.ApplyResources(this.labelServiceName, "labelServiceName");
            this.tableLayoutPanelServiceProvider.SetColumnSpan(this.labelServiceName, 2);
            this.labelServiceName.Name = "labelServiceName";
            // 
            // labelCompany
            // 
            resources.ApplyResources(this.labelCompany, "labelCompany");
            this.tableLayoutPanelServiceProvider.SetColumnSpan(this.labelCompany, 2);
            this.labelCompany.Name = "labelCompany";
            // 
            // labelCompanyName
            // 
            resources.ApplyResources(this.labelCompanyName, "labelCompanyName");
            this.labelCompanyName.Name = "labelCompanyName";
            // 
            // labelSupport
            // 
            resources.ApplyResources(this.labelSupport, "labelSupport");
            this.tableLayoutPanelServiceProvider.SetColumnSpan(this.labelSupport, 2);
            this.labelSupport.Name = "labelSupport";
            // 
            // labelPhoneNumber
            // 
            resources.ApplyResources(this.labelPhoneNumber, "labelPhoneNumber");
            this.labelPhoneNumber.Name = "labelPhoneNumber";
            // 
            // dashboardLinkLabelContosoURL
            // 
            this.dashboardLinkLabelContosoURL.ActiveLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(51)))), ((int)(((byte)(152)))));
            resources.ApplyResources(this.dashboardLinkLabelContosoURL, "dashboardLinkLabelContosoURL");
            this.dashboardLinkLabelContosoURL.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.dashboardLinkLabelContosoURL.LinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(102)))), ((int)(((byte)(255)))));
            this.dashboardLinkLabelContosoURL.LinkUrl = null;
            this.dashboardLinkLabelContosoURL.Name = "dashboardLinkLabelContosoURL";
            this.dashboardLinkLabelContosoURL.TabStop = true;
            // 
            // dashboardLinkLabelContosoEmail
            // 
            this.dashboardLinkLabelContosoEmail.ActiveLinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(51)))), ((int)(((byte)(152)))));
            resources.ApplyResources(this.dashboardLinkLabelContosoEmail, "dashboardLinkLabelContosoEmail");
            this.dashboardLinkLabelContosoEmail.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.dashboardLinkLabelContosoEmail.LinkColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(102)))), ((int)(((byte)(255)))));
            this.dashboardLinkLabelContosoEmail.LinkUrl = null;
            this.dashboardLinkLabelContosoEmail.Name = "dashboardLinkLabelContosoEmail";
            this.dashboardLinkLabelContosoEmail.TabStop = true;
            // 
            // label3
            // 
            resources.ApplyResources(this.label3, "label3");
            this.label3.Name = "label3";
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.Name = "label2";
            // 
            // panelTop
            // 
            resources.ApplyResources(this.panelTop, "panelTop");
            this.panelTop.Controls.Add(this.tableLayoutPanelTop);
            this.panelTop.Name = "panelTop";
            // 
            // SampleAddinForm
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.panelTop);
            this.Name = "SampleAddinForm";
            this.tableLayoutPanelTop.ResumeLayout(false);
            this.tableLayoutPanelTop.PerformLayout();
            this.tableLayoutPanelSubscription.ResumeLayout(false);
            this.tableLayoutPanelSubscription.PerformLayout();
            this.tableLayoutPanelIntegration.ResumeLayout(false);
            this.tableLayoutPanelIntegration.PerformLayout();
            this.tableLayoutPanelNotIntegration.ResumeLayout(false);
            this.tableLayoutPanelNotIntegration.PerformLayout();
            this.tableLayoutPanelServiceProvider.ResumeLayout(false);
            this.tableLayoutPanelServiceProvider.PerformLayout();
            this.panelTop.ResumeLayout(false);
            this.panelTop.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelSigninAccount;
        private System.Windows.Forms.Label labelSigninAccountDescription;
        private System.Windows.Forms.Label labelServerIsIntegratedDescription;
        private System.Windows.Forms.Label labelServerIntegratedTitle;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelTop;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelSubscription;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelServiceProvider;
        private System.Windows.Forms.Label labelServiceProvider;
        private System.Windows.Forms.Label labelServiceName;
        private System.Windows.Forms.Label labelCompany;
        private System.Windows.Forms.Label labelCompanyName;
        private System.Windows.Forms.Label labelSupport;
        private System.Windows.Forms.Label labelPhoneNumber;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelIntegration;
        private System.Windows.Forms.Label labelTotalMailboxes;
        private System.Windows.Forms.Label labelEmailDomains;
        private System.Windows.Forms.Label labelSubscription;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelNotIntegration;
        private System.Windows.Forms.Label labelServerNotIntegrationTitle;
        private System.Windows.Forms.Label labelServerNotIntegratedDescription;
        private System.Windows.Forms.Label labelUserTab;
        private System.Windows.Forms.Label labelMailNumber;
        private System.Windows.Forms.Label labelFirstEmailDomain;
        private System.Windows.Forms.Panel panelTop;
        private Microsoft.WindowsServerSolutions.Controls.DashboardLinkLabel dashboardLinkLabelContosoURL;
        private Microsoft.WindowsServerSolutions.Controls.DashboardLinkLabel dashboardLinkLabelClickToIntegrate;
        private Microsoft.WindowsServerSolutions.Controls.DashboardLinkLabel dashboardLinkLabelContosoEmail;


    }
}
