// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System.Collections.Generic;
using Microsoft.Feeds.Interop;

namespace Microsoft.Samples.RssPlatform.ScreenSaver
{
    internal static class CommonFeedListUtils
    {
        public static IEnumerable<IFeed> CommonFeedList(IFeedFolder folder)
        {
            Queue<IFeedFolder> queue = new Queue<IFeedFolder>();
            queue.Enqueue(folder);
            while (queue.Count > 0)
            {
                IFeedFolder currentFolder = queue.Dequeue();
                foreach (IFeedFolder subfolder in (IFeedsEnum)currentFolder.Subfolders)
                    queue.Enqueue(subfolder);

                foreach (IFeed feed in (IFeedsEnum)currentFolder.Feeds)
                {
                    System.Windows.Forms.Application.DoEvents();
                    yield return feed;
                }
            }
        }

        public static IEnumerable<IFeed> LastWriteSince(IFeedFolder folder, System.DateTime lastWriteTime)
        {
            foreach (IFeed feed in CommonFeedList(folder))
            {
                if (feed.LastWriteTime.ToLocalTime() > lastWriteTime)
                    yield return feed;
            }
        }

        /*** Similar functionality can be implemented in Visual Studio .NET 2003 as follows.
        public static System.Collections.IEnumerable CommonFeedList(IFeedFolder folder)
        {
            System.Collections.Queue queue = new System.Collections.Queue();
            System.Collections.ArrayList list = new System.Collections.ArrayList();
            queue.Enqueue(folder);
            while (queue.Count > 0)
            {
                IFeedFolder currentFolder = (IFeedFolder) queue.Dequeue();
                foreach (IFeedFolder subfolder in currentFolder.Subfolders)
                    queue.Enqueue(subfolder);

                foreach (IFeed feed in currentFolder.Feeds)
                    list.Add(feed);
            }
            return list;
        } */
    }
}
