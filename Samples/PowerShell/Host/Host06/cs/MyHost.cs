// <copyright file="MyHost.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Host
{
    using System;
    using System.Globalization;
    using System.Management.Automation.Host;
    using System.Management.Automation.Runspaces;

    /// <summary>
    /// A sample implementation of the PSHost abstract class for console
    /// applications. Not all members are implemented. Those that are not 
    /// implemented throw a NotImplementedException exception.
    /// </summary>
    internal class MyHost : PSHost, IHostSupportsInteractiveSession
    {
        public MyHost(PSListenerConsoleSample program)
        {
            this.program = program;
        }

        /// <summary>
        /// A reference to the runspace used to start an interactive session.
        /// </summary>
        public Runspace pushedRunspace = null;

        /// <summary>
        /// A reference to the listener.
        /// </summary>
        private PSListenerConsoleSample program;

        /// <summary>
        /// The culture information of the thread that created this object.
        /// </summary>
        private CultureInfo originalCultureInfo = System.Threading.Thread.CurrentThread.CurrentCulture;

        /// <summary>
        /// The UI culture information of the thread that created this object.
        /// </summary>
        private CultureInfo originalUICultureInfo = System.Threading.Thread.CurrentThread.CurrentUICulture;

        /// <summary>
        /// The identifier of this instance of the host implementation.
        /// </summary>
        private static Guid instanceId = Guid.NewGuid();

        /// <summary>
        /// A reference to the implementation of the PSHostUserInterface
        /// class for this application.
        /// </summary>
        private MyHostUserInterface myHostUserInterface = new MyHostUserInterface();

        /// <summary>
        /// Gets the culture information to use. This implementation takes a 
        /// snapshot of the culture information of the thread that created 
        /// this object.
        /// </summary>
        public override CultureInfo CurrentCulture
        {
            get { return this.originalCultureInfo; }
        }

        /// <summary>
        /// Gets the UI culture information to use. This implementation takes 
        /// snapshot of the UI culture information of the thread that created 
        /// this object.
        /// </summary>
        public override CultureInfo CurrentUICulture
        {
            get { return this.originalUICultureInfo; }
        }

        /// <summary>
        /// Gets the identifier of this instance of the host implementation. 
        /// This implementation always returns the GUID allocated at 
        /// instantiation time.
        /// </summary>
        public override Guid InstanceId
        {
            get { return instanceId; }
        }

        /// <summary>
        /// Gets the name of the host implementation. This string may be used 
        /// by script writers to identify when this host is being used.
        /// </summary>
        public override string Name
        {
            get { return "MySampleConsoleHostImplementation"; }
        }

        /// <summary>
        /// Gets an instance of the implementation of the PSHostUserInterface class 
        /// for this application. This instance is allocated once at startup time
        /// and returned every time thereafter.
        /// </summary>
        public override PSHostUserInterface UI
        {
            get { return this.myHostUserInterface; }
        }

        /// <summary>
        /// Gets the version object for this host application. Typically 
        /// this should match the version resource in the application.
        /// </summary>
        public override Version Version
        {
            get { return new Version(1, 0, 0, 0); }
        }
        
        #region IHostSupportsInteractiveSession Properties

        /// <summary>
        /// Gets a value indicating whether a request 
        /// to open a PSSession has been made.
        /// </summary>
        public bool IsRunspacePushed
        {
            get { return this.pushedRunspace != null; }
        }

        /// <summary>
        /// Gets or sets the runspace used by the PSSession.
        /// </summary>
        public Runspace Runspace
        {
            get { return this.program.myRunSpace; }
            internal set { this.program.myRunSpace = value; }
        }
        #endregion IHostSupportsInteractiveSession Properties

        /// <summary>
        /// Instructs the host to interrupt the currently running pipeline 
        /// and start a new nested input loop. Not implemented by this example class. 
        /// The call fails with an exception.
        /// </summary>
        public override void EnterNestedPrompt()
        {
            throw new NotImplementedException("Cannot suspend the shell, EnterNestedPrompt() method is not implemented by MyHost.");
        }

        /// <summary>
        /// Instructs the host to exit the currently running input loop. Not 
        /// implemented by this example class. The call fails with an 
        /// exception.
        /// </summary>
        public override void ExitNestedPrompt()
        {
            throw new NotImplementedException("The ExitNestedPrompt() method is not implemented by MyHost.");
        }

        /// <summary>
        /// Notifies the host that the Windows PowerShell runtime is about to 
        /// execute a legacy command-line application. Typically it is used 
        /// to restore state that was changed by a child process after the 
        /// child exits. This implementation does nothing and simply returns.
        /// </summary>
        public override void NotifyBeginApplication()
        {
            return;  // Do nothing.
        }

        /// <summary>
        /// Notifies the host that the Windows PowerShell engine has 
        /// completed the execution of a legacy command. Typically it 
        /// is used to restore state that was changed by a child process 
        /// after the child exits. This implementation does nothing and 
        /// simply returns.
        /// </summary>
        public override void NotifyEndApplication()
        {
           return; // Do nothing.
        }

        /// <summary>
        /// Indicate to the host application that exit has
        /// been requested. Pass the exit code that the host
        /// application should use when exiting the process.
        /// </summary>
        /// <param name="exitCode">The exit code that the 
        /// host application should use.</param>
        public override void SetShouldExit(int exitCode)
        {
            this.program.ShouldExit = true;
            this.program.ExitCode = exitCode;
        }

        #region IHostSupportsInteractiveSession Methods

        /// <summary>
        /// Requests to close a PSSession.
        /// </summary>
        public void PopRunspace()
        {
            Runspace = this.pushedRunspace;
            this.pushedRunspace = null;
        }

        /// <summary>
        /// Requests to open a PSSession.
        /// </summary>
        /// <param name="runspace">Runspace to use.</param>
        public void PushRunspace(Runspace runspace)
        {
            this.pushedRunspace = Runspace;
            Runspace = runspace;
        }

        #endregion IHostSupportsInteractiveSession Methods
    }
}

