//-----------------------------------------------------------------------
// <copyright file="SessionConfiguration.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.BasicPlugins
{
    using System.IO;
    using System.Linq;
    using System.Management.Automation;
    using System.Management.Automation.Remoting;
    using System.Management.Automation.Runspaces;
    using System.Web;

    /// <summary>
    /// PSSessionConfigruation implementation
    /// Management OData serivce uses PSSessionConfiguration interface to get PowerShell session configuration for a user
    /// This implementation gives same PowerShell session configuration for all users.
    /// It adds all the PowerShell modules (*.psm1) and PowerShell scripts (*-*.ps1) files into the PowerShell session configuration.
    /// </summary>
    public class SessionConfiguration : PSSessionConfiguration
    {
        /// <summary>
        /// Gets application private data
        /// </summary>
        /// <param name="senderInfo">Sender information</param>
        /// <returns>Always returns null</returns>
        public override PSPrimitiveDictionary GetApplicationPrivateData(PSSenderInfo senderInfo)
        {
            return null;
        }

        /// <summary>
        /// Gets custom initial session state
        /// </summary>
        /// <param name="senderInfo">Sender information</param>
        /// <returns>Custom initial Session state</returns>
        public override InitialSessionState GetInitialSessionState(PSSenderInfo senderInfo)
        {
            var initialSessionState = InitialSessionState.CreateDefault();

            var dir = HttpContext.Current != null ? HttpContext.Current.Server.MapPath(".") : Directory.GetCurrentDirectory();

            initialSessionState.ImportPSModule(Directory.EnumerateFiles(dir, "*.psm1")
                .Select(s => Path.Combine(dir, Path.GetFileNameWithoutExtension(s))).ToArray());

            foreach (var cmdletFileName in Directory.EnumerateFiles(dir, "*-*.ps1"))
            {
                initialSessionState.Commands.Add(
                    new SessionStateFunctionEntry(Path.GetFileNameWithoutExtension(cmdletFileName), File.ReadAllText(Path.Combine(dir, cmdletFileName))));
            }

            return initialSessionState;
        }
    }
}
