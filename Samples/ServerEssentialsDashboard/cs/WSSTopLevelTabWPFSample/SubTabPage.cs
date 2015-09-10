//
//  <copyright file="SubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSTopLevelTabWPFSample
{
    public class SubTabPage : ControlRendererPage
    {
        public SubTabPage()
            : base(new Guid("6f1f3f4c-7b89-46c2-b511-239ae5a427c9"), // Put your fixed, static guid here
                    "WSS WPF SubTab Sample",
                   "WSS Dashboard Addin Sample consisting of a SubTab containing a WPF User Control")
        {
        }

        protected override ControlRendererPageContent CreateContent()
        {
            return ControlRendererPageContent.Create(new MyControlHost());
        }
    }
}
