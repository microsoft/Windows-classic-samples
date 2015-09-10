//
//  <copyright file="SubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using System.Diagnostics.CodeAnalysis;

namespace WSSSubTabListviewSample
{
    public class SubTabPage : Page
    {
        public SubTabPage()
            : base(new Guid("b108468d-2f8a-4e3b-a15f-30b16af21c66"),
                   "WSS ListView SubTab Sample",
                   "WSS Dashboard Addin Sample consisting of a custom ListView SubTab ")
        {
        }

        protected override PageContent CreateContent()
        {
            return PageContent.Create<MyBusinessObject>(
                new SubTabListProvider(),
                SubTabListColumns.CreateColumns(),
                SubTabListGroupings.CreateGroupings(),
                SubTabTasks.CreateTasks(),
                SubTabDetails.GetDetails
                );
        }

        protected override HelpTopicInfo CreateHelpTopicInfo()
        {
            // Replace with a custom URL or CHM/HTML file path.
            return new HelpTopicInfo(new Uri("http://www.microsoft.com"));
        }
    }
}
