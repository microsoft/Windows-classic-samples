//
//  <copyright file="DistributionGroupAdorner.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using Contoso.EmailService;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;
using Microsoft.WindowsServerSolutions.Common.ProviderFramework;
using Microsoft.WindowsServerSolutions.HostedEmail;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Contoso.HostedEmail.DistributionGroup
{
    public class DistributionGroupAdorner : FormContentAdorner, IHostedEmailExtension
    {
        private DistributionGroupTabContent content = null;
        public DistributionGroupAdorner()
            : base(new Guid("B00F3F8D-176B-4A85-A1C9-3022A6E5B9BC"), Resources.DGAdorner_Name, Resources.DGAdorner_Des)
        {
        }

        public override ICollection<AddinPageContent> CreatePages(FormPropertyBag propertyBag)
        {
            List<AddinPageContent> list = new List<AddinPageContent>();
            content = new DistributionGroupTabContent(propertyBag)
            {
                Title = Resources.DGTab_Name,
                HelpLink = null,
                HelpLinkText = null
            };
            list.Add(content);
            return list;
        }

        public override ICollection<Task> CreatePostExecutionTasks(FormPropertyBag propertyBag)
        {
            return null;
        }

        public override ICollection<Task> CreatePreExecutionTasks(FormPropertyBag propertyBag)
        {
            if (content == null) return null;
            List<Task> tasks = new List<Task>();
            tasks.Add(new Task(() => content.UpdateDistributionGroups()));
            return tasks;
        }

        #region IHostedEmailExtension

        public Guid GetAddinId()
        {
            return Constants.AdaptorId;
        }

        #endregion
    }

    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1001:TypesThatOwnDisposableFieldsShouldBeDisposable", Justification = "TabControl will be disposed by the framework.")]
    public class DistributionGroupTabContent : AddinPageContent
    {
        DistributionGroupControl tabControl = null;
        public DistributionGroupTabContent(FormPropertyBag propertyBag)
            : base(propertyBag)
        {
            tabControl = new DistributionGroupControl(base.PropertyBag);
            tabControl.Dock = DockStyle.Fill;
            tabControl.AutoScroll = false;
            // ATTENTION: 
            // the reason of loading data in page initiailzing coz that we need the UserName in propertyBag which will be 
            // filled in with data after the controls are created. Therefore, we cannot get data we want in this ctor.
            this.PageInitializing += (object sender, EventArgs e) => { this.tabControl.StartLoadingData(); };
            this.PageValidating += (object sender, CancelEventArgs e) =>
                {
                    // Putting the data here coz this method is called in UI thread
                    base.PropertyBag.AddinData[Constants.ExtendedParam_DGs] = tabControl.GetDistributionGroupsString();
                };
        }

        public override Control CreateControl()
        {
            return tabControl;
        }

        public override event EventHandler PropertyChanged
        {
            add
            {
                tabControl.PropertyChanged += value;
            }
            remove
            {
                tabControl.PropertyChanged -= value;
            }
        }

        internal void UpdateDistributionGroups()
        {
            tabControl.UpdateDistributionGroups();
        }
    }

}
