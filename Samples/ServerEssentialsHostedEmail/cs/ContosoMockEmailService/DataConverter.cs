//
//  <copyright file="DataConverter.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using Microsoft.WindowsServerSolutions.HostedEmail;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;

namespace Contoso.EmailService
{
    internal static class DataConverter
    {
        public static HostedEmailAdaptorErrorCode ToHEAE_ErrorCode(this ErrorCodeEnum error)
        {
            switch (error)
            {
                case ErrorCodeEnum.AccountAlreadyExists: return HostedEmailAdaptorErrorCode.AccountAlreadyExists;
                case ErrorCodeEnum.AccountNotExists: return HostedEmailAdaptorErrorCode.AccountNotExists;
                case ErrorCodeEnum.AuthenticationFailure: return HostedEmailAdaptorErrorCode.AuthenticationFailure;
                case ErrorCodeEnum.Custom: return HostedEmailAdaptorErrorCode.Custom;
                case ErrorCodeEnum.InsufficientPermission: return HostedEmailAdaptorErrorCode.InsufficientPermission;
                case ErrorCodeEnum.InvalidEmailAddress: return HostedEmailAdaptorErrorCode.InvalidEmailAddress;
                case ErrorCodeEnum.ServiceNotReachable: return HostedEmailAdaptorErrorCode.ServiceNotReachable;
                default: return HostedEmailAdaptorErrorCode.Custom;
            }
        }

        public static AddinErrorRecord ToAddinErrorRecord(this ErrorRecord errorRecord)
        {
            if (errorRecord == null) return null;
            return new AddinErrorRecord()
            {
                HelpLink = errorRecord.HelpLink,
                Message = errorRecord.ErrorMessage,
                Title = errorRecord.ErrorTitle,
            };
        }
    }
}
