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
    public class TopLevelTabPageProvider : PageProvider
    {
        public TopLevelTabPageProvider()
            : base(new Guid("0dc6c1d7-8094-42cc-876b-fc7d7f4cb23c"),  // Put your fixed, static GUID here
                  "WSS ListView Tab Sample",
                  "WSS Dashboard Addin Sample consisting of a custom ListView SubTab ")  // description for the add-in
        {
        }
        protected override System.Drawing.Icon CreateImage()
        {
            //return SystemIcons.Shield;
            return WSSTopLevelTabListView.GenericIcon;
        }

        protected override object CreatePages()
        {
            // Use this method to create subtab pages

            object pageSample = new ListViewSubTabPage();
            object[] pages = new object[] { pageSample };
            return pages;
        }
    }
}