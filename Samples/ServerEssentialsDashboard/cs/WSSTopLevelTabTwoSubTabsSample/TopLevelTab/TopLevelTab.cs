//
//  <copyright file="TopLevelTab.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using System.Drawing;
using ListViewSubTab;

namespace TopLevelTab
{
    [ContainsCustomControl] // Required attribute for custom tabs
    public class TopLevelTabPageProvider : PageProvider
    {
        public TopLevelTabPageProvider()
            : base(new Guid("08999c4f-187b-483d-bddc-aca7023f11ea"),  // Put your fixed, static guid here
                   "WSS Winform Tab with 2 Sub-Tabs Sample",
                   "WSS Dashboard Addin Sample for a top level tab containing a Winform User Control Sub-Tab and a custom ListView Sub-Tab")
        {
        }

        protected override Icon CreateImage()
        {
            //return SystemIcons.Information;
            return WSSTopLevelTwoSubTabsSample.GenericIcon;
        }

        protected override object CreatePages()
        {
            return (new object[] { new ListViewSubTabPage(), new CustomSubTabPage() });
        }
    }
}