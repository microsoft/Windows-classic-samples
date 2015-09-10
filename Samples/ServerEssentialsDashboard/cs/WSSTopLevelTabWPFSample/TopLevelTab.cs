//
//  <copyright file="TopLevelTab.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSTopLevelTabWPFSample
{
    [ContainsCustomControl] // Required attribute for custom tabs
    public class TopLevelTabPageProvider : PageProvider
    {
        public TopLevelTabPageProvider()
            : base(new Guid("f5392463-d07e-4c13-95a8-8943953b2f62"),  // Put your fixed, static guid here
                   "WSS WPF Tab Sample",
                   "WSS Dashboard Addin Sample for a top level tab containing a WPF User Control SubTab")
        {
        }

        protected override Icon CreateImage()
        {
            // return SystemIcons.Information;
            return TopLevelTabResources.GenericIcon;
        }

        protected override object CreatePages()
        {
            // Use this method to instantiate sub-tabs
            object pageSample = new SubTabPage();
            object[] pages = new Object[] { pageSample };
            return pages;
        }
    }
}
