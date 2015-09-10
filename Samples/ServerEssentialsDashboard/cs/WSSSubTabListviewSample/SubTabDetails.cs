//
//  <copyright file="SubTabDetails.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSSubTabListviewSample
{
    static class SubTabDetails
    {
        public static DetailColumnCollection GetDetails(MyBusinessObject businessObj)
        {
            var columns = new DetailColumnCollection();

            var column = new DetailColumn();
            var group = new DetailGroup("Computer Information");
            group.Add("Computer Name", businessObj.ComputerName);
            group.Add("Network Name", businessObj.NetworkName);

            column.Add(group);
            columns.Add(column);

            return (columns);
        }
    }
}
