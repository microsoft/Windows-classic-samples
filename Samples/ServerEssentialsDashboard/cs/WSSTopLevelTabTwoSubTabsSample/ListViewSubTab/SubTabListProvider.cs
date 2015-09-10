//
//  <copyright file="SubTabListProvider.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace ListViewSubTab
{
    public class SubTabListProvider : ListProvider<MyBusinessObject>
    {
        protected override void RefreshAndListenForUpdates(IList<MyBusinessObject> list)
        {
            // fake data
            var bObject = new MyBusinessObject();
            bObject.AdminName = "User";
            bObject.CompanyName = "Microsoft";
            bObject.ComputerName = "Computer";
            bObject.NetworkName = "MyMachine.Local";

            list.Add(bObject);

            bObject = new MyBusinessObject();
            bObject.AdminName = "User2";
            bObject.CompanyName = "Microsoft";
            bObject.ComputerName = "Computer2";
            bObject.NetworkName = "MyMachine.Local";

            list.Add(bObject);

            bObject = new MyBusinessObject();
            bObject.AdminName = "User2";
            bObject.CompanyName = "Microsoft";
            bObject.ComputerName = "Computer3";
            bObject.NetworkName = "MyMachine.Local2";

            list.Add(bObject);

            bObject = new MyBusinessObject();
            bObject.AdminName = "User2";
            bObject.CompanyName = "Microsoft";
            bObject.ComputerName = "Computer4";
            bObject.NetworkName = "MyMachine.Local2";

            list.Add(bObject);
        }

        protected override void StopListeningForUpdates()
        {
            // not listening for updates; nothing to do
        }

        protected override string GetObjectDisplayName(MyBusinessObject businessObj)
        {
            // return a string that represents the friendly/displayable name of businessObj
            return (businessObj.ComputerName);
        }

        protected override string GetObjectId(MyBusinessObject businessObj)
        {
            // return a string that uniquely identifies the businessObj
            return businessObj.ComputerName + "." + businessObj.NetworkName;
        }
    }
}
