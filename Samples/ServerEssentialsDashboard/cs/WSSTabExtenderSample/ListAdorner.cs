//
//  <copyright file="ListAdorner.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;
using System.Diagnostics;

namespace WSSTabExtenderSample
{
    public class ListAdorner : ListProviderAdorner
    {
        private IList<ListObject> list;

        public override void RefreshAndListenForUpdates(IList<ListObject> list)
        {
            this.list = list;
        }

        public override void StopListeningForUpdates()
        {
            // Stop listening for updates.
        }
    }
}
