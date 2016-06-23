//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Text;
using System.Management.Automation;
using System.Management.Automation.Host;
using System.Management.Automation.Runspaces;

namespace Microsoft.Samples.PowerShell.Host
{
    /// <summary>
    /// Sample implementation of the PSHostUserInterface class.
    /// This is more complete than the previous samples however
    /// there are still members that are unimplemented by this class.
    /// Members that aren't implemented usually throw a
    ///    NotImplementedException
    /// though some just do nothing and silently return. Members
    /// that map simply onto the existing .NET console APIs are
    /// supported. Also a basic implementation of the prompt
    /// API provided. The credential and secure string methods are
    /// not supported.
    /// </summary>
    class MyHostUserInterface : PSHostUserInterface
    {
        public override Dictionary<string, PSObject> Prompt(
        string caption, string message,
        Collection<FieldDescription> descriptions)
        {
            Write(ConsoleColor.Blue, ConsoleColor.Black,
                caption + "\n" + message + " ");
            Dictionary<string, PSObject> results =
                new Dictionary<string, PSObject>();
            foreach (FieldDescription fd in descriptions)
            {
                string[] label = GetHotkeyAndLabel(fd.Label);
                WriteLine(label[1]);
                string userData = Console.ReadLine();
                if (userData == null)
                    return null;
                results[fd.Name] = PSObject.AsPSObject(userData);
            }
            return results;
        }

        public override int PromptForChoice(
            string caption, string message,
            Collection<ChoiceDescription> choices, int defaultChoice)
        {
            // Write the caption and message strings in Blue.
            WriteLine(ConsoleColor.Blue, ConsoleColor.Black,
                caption + "\n" + message + "\n");

            // Convert the choice collection into something that's a
            // little easier to work with
            // See the BuildHotkeysAndPlainLabels method for details.
            Dictionary<string, PSObject> results = 
                new Dictionary<string, PSObject>();
            string[,] promptData = BuildHotkeysAndPlainLabels(choices);

            // Format the overall choice prompt string to display...
            StringBuilder sb = new StringBuilder();
            for (int element=0; element < choices.Count; element++)
            {
                sb.Append(String.Format(CultureInfo.CurrentCulture,"|{0}> {1} ",
                    promptData[0,element], promptData[1, element]));
            }
            sb.Append(String.Format(CultureInfo.CurrentCulture,"[Default is ({0}]",
                promptData[0,defaultChoice]));

            // loop reading prompts until a match is made, the default is
            // chosen or the loop is interrupted with ctrl-C.
            while (true)
            {
                WriteLine(ConsoleColor.Cyan, ConsoleColor.Black, sb.ToString());
                string data = Console.ReadLine().Trim().ToUpper(CultureInfo.CurrentCulture);

                // if the choice string was empty, use the default selection
                if (data.Length == 0)
                    return defaultChoice;

                // see if the selection matched and return the
                // corresponding index if it did...
                for (int i = 0; i < choices.Count; i++)
                {
                    if (promptData[0, i] == data)
                        return i;
                }
                WriteErrorLine("Invalid choice: " + data);
            }
        }

        /// <summary>
        /// Parse a string containing a hotkey character.
        /// 
        /// Take a string of the form
        ///    Yes to &amp;all
        /// and returns a two-dimensional array split out as
        ///    "A", "Yes to all".
        /// </summary>
        /// <param name="input">The string to process</param>
        /// <returns>
        /// A two dimensional array containing the parsed components.
        /// </returns>
        private static string[] GetHotkeyAndLabel(string input)
        {
            string[] result = new string[] {String.Empty, String.Empty };
            string[] fragments = input.Split('&');
            if (fragments.Length == 2)
            {
                if (fragments[1].Length > 0)
                    result[0] = fragments[1][0].ToString().
                ToUpper(CultureInfo.CurrentCulture);
                result[1] = (fragments[0] + fragments[1]).Trim();
            }
            else
            {
                result[1] = input;
            }
            return result;
        }

        /// <summary>
        /// This is a private worker function splits out the
        /// accelerator keys from the menu and builds a two
        /// dimentional array with the first access containing the
        /// accelerator and the second containing the label string
        /// with the &amp; removed.
        /// </summary>
        /// <param name="choices">The choice collection to process</param>
        /// <returns>
        /// A two dimensional array containing the accelerator characters
        /// and the cleaned-up labels</returns>
        private static string[,] BuildHotkeysAndPlainLabels(
            Collection<ChoiceDescription> choices)
        {
            // we will allocate the result array
            string[,] hotkeysAndPlainLabels = new string[2, choices.Count];

            for (int i = 0; i < choices.Count; ++i)
            {
                string[] hotkeyAndLabel = GetHotkeyAndLabel(choices[i].Label);
                hotkeysAndPlainLabels[0,i] = hotkeyAndLabel[0];
                hotkeysAndPlainLabels[1,i] = hotkeyAndLabel[1];
            }
            return hotkeysAndPlainLabels;
        }

        public override PSCredential PromptForCredential(
            string caption, string message, string userName, string targetName)
        {
            throw new NotImplementedException(
              "The method PromptForCredential() is not implemented by MyHost.");
        }

        public override PSCredential PromptForCredential(
            string caption, string message, string userName,
            string targetName, PSCredentialTypes allowedCredentialTypes,
            PSCredentialUIOptions options)
        {
            throw new NotImplementedException(
              "The method PromptForCredential() is not implemented by MyHost.");
        }

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
            throw new NotImplementedException(
           "The method ReadLineAsSecureString() is not implemented by MyHost.");
        }

        public override void Write(string value)
        {
            Console.Write(value); ;
        }

        public override void Write(ConsoleColor foregroundColor,
            ConsoleColor backgroundColor, string value)
        {
            ConsoleColor oldFg = Console.ForegroundColor;
            ConsoleColor oldBg = Console.BackgroundColor;
            Console.ForegroundColor = foregroundColor;
            Console.BackgroundColor = backgroundColor;
            Console.Write(value);
            Console.ForegroundColor = oldFg;
            Console.BackgroundColor = oldBg;
        }

        public override void WriteLine(ConsoleColor foregroundColor,
            ConsoleColor backgroundColor, string value)
        {
            ConsoleColor oldFg = Console.ForegroundColor;
            ConsoleColor oldBg = Console.BackgroundColor;
            Console.ForegroundColor = foregroundColor;
            Console.BackgroundColor = backgroundColor;
            Console.WriteLine(value);
            Console.ForegroundColor = oldFg;
            Console.BackgroundColor = oldBg;
        }

        public override void WriteDebugLine(string message)
        {
            this.WriteLine(ConsoleColor.DarkYellow, ConsoleColor.Black,
                String.Format(CultureInfo.CurrentCulture, "DEBUG: {0}", message));
        }

        public override void WriteErrorLine(string value)
        {
            this.WriteLine(ConsoleColor.Red, ConsoleColor.Black, value);
        }

        public override void WriteLine()
        {
            Console.WriteLine();
        }

        public override void WriteLine(string value)
        {
            Console.WriteLine(value);
        }

        public override void WriteVerboseLine(string message)
        {
            this.WriteLine(ConsoleColor.Green, ConsoleColor.Black,
                String.Format(CultureInfo.CurrentCulture,"VERBOSE: {0}", message));
        }

        public override void WriteWarningLine(string message)
        {
            this.WriteLine(ConsoleColor.Yellow, ConsoleColor.Black,
                String.Format(CultureInfo.CurrentCulture,"WARNING: {0}", message));
        }

        /// <summary>
        /// Progress is not implemented by this class. Since it's not
        /// required for the cmdlet to work, it is better to do nothing
        /// instead of throwing an exception.
        /// </summary>
        /// <param name="sourceId">See base class</param>
        /// <param name="record">See base class</param>
        public override void WriteProgress(long sourceId, ProgressRecord record)
        {
            ; // Do nothing...
        }
    }
}

