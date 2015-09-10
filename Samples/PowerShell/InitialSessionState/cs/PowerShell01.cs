// <copyright file="PowerShell01.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Sample
{
    using System;
    using System.Collections.ObjectModel;
    using System.Management.Automation;
    using System.Management.Automation.Runspaces;

    /// <summary>
    /// This class contains the Main entry point for the application.
    /// </summary>
    internal class PowerShell01
    {
        /// <summary>
        /// The runspace used to run commands.
        /// </summary>
        private Runspace runspace;

        /// <summary>
        /// Return the first index of the entry in <paramref name="entries"/> 
        /// with the name <paramref name="name"/>. Return -1 if it is not found.
        /// </summary>
        /// <typeparam name="T">Type of ConstrainedSessionStateEntry</typeparam>
        /// <param name="entries">Collection of entries to search for <paramref name="name"/> in.</param>
        /// <param name="name">Named of the entry we are looking for</param>
        /// <returns>
        /// The first index of the entry in <paramref name="entries"/> with the 
        /// name <paramref name="name"/>, or return -1 if it is not found.
        /// </returns>
        private static int GetIndexOfEntry<T>(
            InitialSessionStateEntryCollection<T> entries, 
            string name) where T : ConstrainedSessionStateEntry
        {
            int foundIndex = 0;
            foreach (T entry in entries)
            {
                if (entry.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                {
                    return foundIndex;
                }

                foundIndex++;
            }

            return -1;
        }

        /// <summary>
        /// Execute commands to demonstrate ways to constrain the runspace.
        /// </summary>
        /// <param name="args">This parameter is unused.</param>
        private static void Main(string[] args)
        {
            new PowerShell01().RunCommands();
        }

        /// <summary>
        /// Run a script to display the results and errors.
        /// </summary>
        /// <param name="script">Script to be run.</param>
        /// <param name="scriptComment">Comment to be printed about the script.</param>
        private void RunScript(string script, string scriptComment)
        {
            Console.WriteLine("Running '{0}'\n{1}.\n\nPowerShell Output:", script, scriptComment);
            
            // Using the PowerShell API, create a pipeline, add the script to the pipeline, and 
            // specify the runspace where the pipeline is invoked.
            PowerShell powerShellCommand = PowerShell.Create();
            powerShellCommand.AddScript(script);
            powerShellCommand.Runspace = this.runspace;

            try
            {
                Collection<PSObject> results = powerShellCommand.Invoke();

                // Display the results.
                foreach (PSObject result in results)
                {
                    Console.WriteLine(result);
                }

                // Display any non-terminating errors.
                foreach (ErrorRecord error in powerShellCommand.Streams.Error)
                {
                    Console.WriteLine("PowerShell Error: {0}", error);
                }
            }
            catch (RuntimeException ex)
            {
                Console.WriteLine("PowerShell Error: {0}", ex.Message);
                Console.WriteLine();
            }

            Console.WriteLine("\n-----------------------------\n");
        }

        /// <summary>
        /// Run some commands to demonstrate the script capabilities.
        /// </summary>
        private void RunCommands()
        {
            this.runspace = RunspaceFactory.CreateRunspace(InitialSessionState.CreateDefault());
            this.runspace.Open();
            this.RunScript("$a=0;$a", "Assigning to a variable will work for a default InitialSessionState");
            this.runspace.Close();

            this.runspace = RunspaceFactory.CreateRunspace(InitialSessionState.CreateDefault());
            this.runspace.InitialSessionState.LanguageMode = PSLanguageMode.RestrictedLanguage;
            this.runspace.Open();
            this.RunScript("$a=0;$a", "Assigning to a variable will not work in RestrictedLanguage LanguageMode");
            this.runspace.Close();

            this.runspace = RunspaceFactory.CreateRunspace(InitialSessionState.CreateDefault());
            this.runspace.InitialSessionState.LanguageMode = PSLanguageMode.NoLanguage;
            this.runspace.Open();
            this.RunScript("10/2", "A script will not work in NoLanguage LanguageMode");
            this.runspace.Close();

            this.runspace = RunspaceFactory.CreateRunspace(InitialSessionState.CreateDefault());
            this.runspace.Open();
            string scriptComment = "get-childitem with a default InitialSessionState will work since the standard \n" +
                "PowerShell cmdlets are included in the default InitialSessionState";
            this.RunScript("get-childitem", scriptComment);
            this.runspace.Close();

            InitialSessionState defaultSessionState = InitialSessionState.CreateDefault();
            defaultSessionState.Commands.Add(new SessionStateAliasEntry("dir2", "get-childitem"));
            this.runspace = RunspaceFactory.CreateRunspace(defaultSessionState);
            this.runspace.Open();
            this.RunScript("dir2", "An alias, like dir2, can be added to InitialSessionState");
            this.runspace.Close();

            defaultSessionState = InitialSessionState.CreateDefault();
            int commandIndex = GetIndexOfEntry(defaultSessionState.Commands, "get-childitem");
            defaultSessionState.Commands.RemoveItem(commandIndex);
            this.runspace = RunspaceFactory.CreateRunspace(defaultSessionState);
            this.runspace.Open();
            scriptComment = "get-childitem was removed from the list of commands so it\nwill no longer be found";
            this.RunScript("get-childitem", scriptComment);
            this.runspace.Close();

            defaultSessionState = InitialSessionState.CreateDefault();
            defaultSessionState.Providers.Clear();
            this.runspace = RunspaceFactory.CreateRunspace(defaultSessionState);
            this.runspace.Open();
            this.RunScript("get-childitem", "There are no providers so get-childitem will not work");
            this.runspace.Close();

            // Marks a command as private, and then defines a proxy command
            // that uses the private command.  One reason to define a proxy for a command is
            // to remove a parameter of the original command. 
            // For a more complete sample of a proxy command, see the Runspace11 sample.
            defaultSessionState = InitialSessionState.CreateDefault();
            commandIndex = GetIndexOfEntry(defaultSessionState.Commands, "get-childitem");
            defaultSessionState.Commands[commandIndex].Visibility = SessionStateEntryVisibility.Private;
            CommandMetadata getChildItemMetadata = new CommandMetadata(
                typeof(Microsoft.PowerShell.Commands.GetChildItemCommand));
            getChildItemMetadata.Parameters.Remove("Recurse");
            string getChildItemBody = ProxyCommand.Create(getChildItemMetadata);
            defaultSessionState.Commands.Add(new SessionStateFunctionEntry("get-childitem2", getChildItemBody));
            this.runspace = RunspaceFactory.CreateRunspace(defaultSessionState);
            this.runspace.Open();
            this.RunScript("get-childitem", "get-childitem is private so it will not be available");
            scriptComment = "get-childitem2 is is a proxy to get-childitem. \n" +
                "It works even when get-childitem is private.";
            this.RunScript("get-childitem2", scriptComment);
            scriptComment = "This will fail. Unlike get-childitem, get-childitem2 does not have -Recurse";
            this.RunScript("get-childitem2 -Recurse", scriptComment);

            InitialSessionState cleanSessionState = InitialSessionState.Create();
            this.runspace = RunspaceFactory.CreateRunspace(cleanSessionState);
            this.runspace.Open();
            scriptComment = "A script will not work because \n" +
                "InitialSessionState.Create() will have the default LanguageMode of NoLanguage";
            this.RunScript("10/2", scriptComment);
            this.runspace.Close();

            cleanSessionState = InitialSessionState.Create();
            cleanSessionState.LanguageMode = PSLanguageMode.FullLanguage;
            this.runspace = RunspaceFactory.CreateRunspace(cleanSessionState);
            this.runspace.Open();
            scriptComment = "get-childitem, standard cmdlets and providers are not present \n" +
                "in an InitialSessionState returned from InitialSessionState.Create()";
            this.RunScript("get-childitem", scriptComment);
            this.runspace.Close();

            cleanSessionState = InitialSessionState.Create();
            cleanSessionState.Commands.Add(
                new SessionStateCmdletEntry(
                    "Get-ChildItem", 
                    typeof(Microsoft.PowerShell.Commands.GetChildItemCommand), 
                    null));
            cleanSessionState.Providers.Add(
                new SessionStateProviderEntry(
                    "FileSystem", 
                    typeof(Microsoft.PowerShell.Commands.FileSystemProvider),
                    null));
            cleanSessionState.LanguageMode = PSLanguageMode.FullLanguage;
            this.runspace = RunspaceFactory.CreateRunspace(cleanSessionState);
            this.runspace.Open();
            scriptComment = "get-childitem and the FileSystem provider were explicitly added\n" +
                "so get-childitem will work";
            this.RunScript("get-childitem", scriptComment);
            this.runspace.Close();

            Console.Write("Done...");
            Console.ReadLine();
        }
    }
}
