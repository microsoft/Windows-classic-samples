//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;
using System.Management.Automation;
using System.Management.Automation.Host;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Host
{
    /// <summary>
    /// A sample implementation of the PSHostUserInterface abstract class for console
    /// applications. Few members are actually implemented. Those that aren't throw a
    /// NotImplementedException.
    /// </summary>
    class MyHostUserInterface : PSHostUserInterface
    {
        public override Dictionary<string, PSObject> Prompt(string caption, string message, System.Collections.ObjectModel.Collection<FieldDescription> descriptions)
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        public override int PromptForChoice(string caption, string message, System.Collections.ObjectModel.Collection<ChoiceDescription> choices, int defaultChoice)
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        public override PSCredential PromptForCredential(string caption, string message, string userName, string targetName)
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        public override PSCredential PromptForCredential(string caption, string message, string userName, string targetName, PSCredentialTypes allowedCredentialTypes, PSCredentialUIOptions options)
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        /// <summary>
        /// Create an instance of the PSRawUserInterface object for this host
        /// application.
        /// </summary>
        private MyRawUserInterface myRawUi = new MyRawUserInterface();
        public override PSHostRawUserInterface RawUI
        {
            get { return myRawUi; }
        }

        public override string ReadLine()
        {
            return Console.ReadLine();
        }

        public override System.Security.SecureString ReadLineAsSecureString()
        {
            throw new NotImplementedException("The method or operation is not implemented.");
        }

        public override void Write(string value)
        {
            System.Console.Write(value); ;
        }

        /// <summary>
        /// Minimal implementation of this method - it ignores the colors.
        /// </summary>
        /// <param name="foregroundColor"></param>
        /// <param name="backgroundColor"></param>
        /// <param name="value"></param>
        public override void Write(ConsoleColor foregroundColor, ConsoleColor backgroundColor, string value)
        {
            // Just ignore the colors...
            System.Console.Write(value);
        }

        public override void WriteDebugLine(string message)
        {
            Console.WriteLine(String.Format(CultureInfo.CurrentCulture, "DEBUG: {0}", message));
        }

        public override void WriteErrorLine(string value)
        {
            Console.WriteLine(String.Format(CultureInfo.CurrentCulture,"ERROR: {0}", value));
        }

        public override void WriteLine()
        {
            System.Console.WriteLine();
        }

        public override void WriteLine(string value)
        {
            System.Console.WriteLine(value);
        }

        public override void WriteLine(ConsoleColor foregroundColor, ConsoleColor backgroundColor, string value)
        {
            // Write to the output stream, ignore the colors
            System.Console.WriteLine(value);
        }

        public override void WriteProgress(long sourceId, ProgressRecord record)
        {
            ; // Do nothing...
        }

        public override void WriteVerboseLine(string message)
        {
            Console.WriteLine(String.Format(CultureInfo.CurrentCulture,"VERBOSE: {0}", message));
        }

        public override void WriteWarningLine(string message)
        {
            Console.WriteLine(String.Format(CultureInfo.CurrentCulture,"WARNING: {0}", message));
        }
    }
}

