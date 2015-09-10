//
//  <copyright file="IProvider.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System.ServiceModel;
using Microsoft.WindowsServerSolutions.Common.ProviderFramework;

namespace ChatObjectModel
{
    [ProviderEndpointBehavior(CredentialType.User,ConnectionSetting.AllowRemoteAccess)]
    [ServiceContract(CallbackContract = typeof(IProviderCallback))]
    public interface IProvider
    {
        [OperationContract(IsOneWay = true)]
        void SetUserName(string name);

        [OperationContract(IsOneWay = true)]
        void SendChat(string text);
    }
}