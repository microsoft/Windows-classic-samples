//
//  <copyright file="ProviderService.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
using System.Text;
using ChatObjectModel;

namespace BasicProvider1
{
    [ServiceBehavior(InstanceContextMode = InstanceContextMode.PerSession,
    ConcurrencyMode = ConcurrencyMode.Multiple,
    UseSynchronizationContext = false)]
    public class ProviderService : IProvider
    {
        public ProviderService()
        {
            m_core = s_core;
            m_callback = OperationContext.Current.GetCallbackChannel<IProviderCallback>();
            OperationContext.Current.Channel.Closed += Channel_Closed;

            m_core.AddConnection(this);
        }

        private void Channel_Closed(object sender, EventArgs e)
        {
            m_core.Disconnect(this);
        }

        private string m_userName;
        private ProviderCore m_core;
        static private ProviderCore s_core = new ProviderCore();
        private IProviderCallback m_callback;

        public void SendChat(string text)
        {
            m_core.SendOperation(m_userName, text);
        }

        internal void SendToClient(string name, string text)
        {
            try
            {
                m_callback.ResponseReceived(name, text);
            }
            catch (CommunicationException) { }
            catch (TimeoutException) { }
        }

           public void SetUserName(string name)
        {
            m_userName = name;
        }
    }
}
