//
//  <copyright file="ObjectModelImplementation.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
using System.Text;
using Microsoft.WindowsServerSolutions.Common.ProviderFramework;

namespace ChatObjectModel
{
    public class ObjectModelImplementation : IProviderCallback
    {
        private ObjectModel m_receiver;
        private ProviderConnector<IProvider> m_connector;

        public ObjectModelImplementation(ObjectModel receiver)
        {
            m_receiver = receiver;
            m_connector = ConnectorFactory.GetServerConnector<IProvider>("provider", this);
            m_connector.ConnectionOpened += m_connector_ConnectionOpened;
            m_connector.ConnectionClosed += m_connector_ConnectionClosed;
        }

        private void m_connector_ConnectionClosed(object sender, ProviderConnectionClosedArgs<IProvider> e)
        {
            m_receiver.Disconnected();
        }

        private string m_name;

        public void Connect(string name)
        {
            m_name = name;
            m_connector.Connect(); //Changed clifford Fix in 
        }

        public void ResponseReceived(string user, string text)
        {
            m_receiver.ReceiveResponse(user, text);
        }

        void m_connector_ConnectionOpened(
          object sender, ProviderConnectionOpenedArgs<IProvider> e)
        {
            try
            {
                m_connector.Connection.SetUserName(m_name);
                m_receiver.ConnectionCompleted();
            }
            catch (CommunicationException)
            {
                // the closed event will do the appropriate cleanup
            }
            catch (TimeoutException)
            {
                // the closed event will do the appropriate cleanup
            }
        }

        public void SendChat(string text)
        {
            try
            {
                m_connector.Connection.SendChat(text);
            }
            catch (CommunicationException)
            {
                // the handler will disconnect
            }
            catch (TimeoutException)
            {
                // Timeouts in a local system generally represent an error case. 
            }
        }
    }
}

   
