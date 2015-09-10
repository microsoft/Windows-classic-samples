//
//  <copyright file="ObjectModel.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace ChatObjectModel
{
    public class ObjectModel : INotifyPropertyChanged
    {
        private string m_userName = "<none>";
        public string UserName
        {
            get { return m_userName; }
            set
            {
                if (m_userName != value)
                {
                    m_userName = value;
                    RaisePropertyChanged("UserName");
                }
            }
        }

        private bool m_connected;
        public ObjectModel()
        {
            //This Initialize method is needed to link to Windows Server references that are not GACed
            Microsoft.WindowsServerSolutions.Common.WindowsServerSolutionsEnvironment.Initialize();

            m_backend = new ObjectModelImplementation(this);
        }
        public bool Connected
        {
            get { return m_connected; }
            internal set
            {
                if (m_connected != value)
                {
                    m_connected = value;
                    RaisePropertyChanged("Connected");
                }
            }
        }

        private ObjectModelImplementation m_backend;

        public void Connect(string userName)
        {
            m_userName = userName;
            m_backend.Connect(m_userName);
        }

        public void SendChat(string text)
        {
            m_backend.SendChat(text);
        }

         public EventHandler<ChatReceivedEventArgs> ResponseReceived;

        private void RaiseResponseReceived(string user, string text)
        {
            EventHandler<ChatReceivedEventArgs> changed = ResponseReceived;
            if (changed != null)
                changed(this, new ChatReceivedEventArgs(user, text));
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler changed = PropertyChanged;
            if (changed != null) changed(this, new PropertyChangedEventArgs(propertyName));
        }

        internal void ReceiveResponse(string user, string text)
        {
            RaiseResponseReceived(user, text);
        }

        internal void ConnectionCompleted()
        {
            Connected = true;
        }

        internal void Disconnected()
        {
            Connected = false;
        }
    }
}
