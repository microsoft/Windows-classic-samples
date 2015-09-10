//
//  <copyright file="IProviderCallback.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System.ServiceModel;

namespace ChatObjectModel
{
    [ServiceContract]
    public interface IProviderCallback
    {
        [OperationContract(IsOneWay = true)]
        void ResponseReceived(string user, string text);
    }
}
