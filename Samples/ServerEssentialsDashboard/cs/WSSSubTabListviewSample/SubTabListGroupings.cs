//
//  <copyright file="SubTabListGroupings.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel;

namespace WSSSubTabListviewSample
{
    static class SubTabListGroupings
    {
        public static ListGroupingCollection<MyBusinessObject> CreateGroupings()
        {
            var groupings = new ListGroupingCollection<MyBusinessObject>();

            var grouping = groupings.Add(
                "Network Name",
                GetObjectListGroup
                );

            return (groupings);
        }

        private static ListGroup<MyBusinessObject> GetObjectListGroup(MyBusinessObject businessObj)
        {
            var group = new ListGroup<MyBusinessObject>(businessObj.NetworkName);
            return (group);
        }
    }
}
