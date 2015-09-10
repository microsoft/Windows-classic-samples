//
//  <copyright file="SubTabListColumns.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSSubTabListviewSample
{
    static class SubTabListColumns
    {
        public static ListColumnCollection<MyBusinessObject> CreateColumns()
        {
            var columns = new ListColumnCollection<MyBusinessObject>();

            var column = columns.Add("ComputerName", "ComputerName");
            column.IsRequired = true;

            column = columns.Add("AdminName", "AdminName");
            column.IsRequired = true;

            column = columns.Add("CompanyName", "CompanyName");
            column.IsRequired = true;

            return(columns);
        }
    }
}
