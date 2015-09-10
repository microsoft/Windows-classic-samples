//
//  <copyright file="AccountManagerExceptions.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Contoso.EmailService
{
    [Serializable]
    public class AccountExistsException : Exception
    {
        public AccountExistsException() { }

        public AccountExistsException(string message) : base(message) { }

        public AccountExistsException(string message, Exception inner) : base(message, inner) { }
    }

    [Serializable]
    public class AccountNotExistsException : Exception
    {
        public AccountNotExistsException() { }

        public AccountNotExistsException(string message) : base(message) { }

        public AccountNotExistsException(string message, Exception inner) : base(message, inner) { }
    }

    [Serializable]
    public class InvalidEmailAddressException : Exception
    {
        public InvalidEmailAddressException() { }

        public InvalidEmailAddressException(string message) : base(message) { }

        public InvalidEmailAddressException(string message, Exception inner) : base(message, inner) { }
    }
}
