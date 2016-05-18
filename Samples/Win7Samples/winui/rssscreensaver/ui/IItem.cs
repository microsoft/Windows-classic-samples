// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Samples.RssPlatform.ScreenSaver.UI
{
    /// <summary>
    /// A generealization of an item with a <c>Description</c> and <c>Title</c>.
    /// Any implmentation of IItem can be rendered using the ItemListView and ItemDescriptionView types.
    /// </summary>
    public interface IItem
    {
        string Description { get; }
        string Title { get; }
    }
}
