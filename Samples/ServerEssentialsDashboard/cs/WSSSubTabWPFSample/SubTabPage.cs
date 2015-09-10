//
//  <copyright file="SubTabPage.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;
using System.Diagnostics.CodeAnalysis;

namespace WSSSubTabWPFSample
{
    public class SubTabPage : ControlRendererPage
    {
        public SubTabPage()
            : base(new Guid("1f4d75aa-2131-4a38-939d-3ef19a7b9458"), // Put your fixed, static GUID here
                   "WSS WPF SubTab Sample",
                   "WSS Dashboard Addin Sample consisting of a SubTab containing a WPF User Control")
        {
        }

        protected override ControlRendererPageContent CreateContent()
        {
            return ControlRendererPageContent.Create(new MyControlHost());
        }

        protected override HelpTopicInfo CreateHelpTopicInfo()
        {
            // Replace with a custom URL or CHM/HTML file path.
            return new HelpTopicInfo(new Uri("http://servername/help"));
        }
    }
}
