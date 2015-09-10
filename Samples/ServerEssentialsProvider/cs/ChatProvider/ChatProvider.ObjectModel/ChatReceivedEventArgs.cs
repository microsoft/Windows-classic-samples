//
//  <copyright file="ChatReceivedEventArgs.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ChatObjectModel
{
    public class ChatReceivedEventArgs : EventArgs
    {
        public string Text { get; set; }
        public string User { get; set; }
        public ChatReceivedEventArgs(string user, string text)
        {
            User = user;
            Text = text;
        }
    }
}