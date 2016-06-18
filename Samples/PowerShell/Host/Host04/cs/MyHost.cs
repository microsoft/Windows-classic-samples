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
 
    /// <summary>
    /// A sample implementation of the PSHost abstract class for console
    /// applications. Not all members are implemented. Those that aren't throw a
    /// NotImplementedException.
    /// </summary>
    internal class MyHost : PSHost
    {
        /// <summary>
        /// The identifier of this instance of the host implementation.
        /// </summary>
        private static Guid instanceId = Guid.NewGuid();

        /// <summary>
        /// A reference to the PSHost implementation.
        /// </summary>
        private PSListenerConsoleSample program;

        /// <summary>
        /// The culture info of the thread that created
        /// this object.
        /// </summary>
        private CultureInfo originalCultureInfo = System.Threading.Thread.CurrentThread.CurrentCulture;

        /// <summary>
        /// The UI culture info of the thread that created
        /// this object.
        /// </summary>
        private CultureInfo originalUICultureInfo = System.Threading.Thread.CurrentThread.CurrentUICulture;

        /// <summary>
        /// A reference to the implementation of the PSHostUserInterface
        /// class for this application.
        /// </summary>
        private MyHostUserInterface myHostUserInterface = new MyHostUserInterface();

        /// <summary>
        /// Initializes a new instance of the MyHost class. Keep a reference 
        /// to the host application object so that it can be informed when 
        /// to exit.
        /// </summary>
        /// <param name="program">A reference to the host application object.</param>
        public MyHost(PSListenerConsoleSample program)
        {
            this.program = program;
        }

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
        /// Gets the version object for this application. Typically this should match the version
        /// resource in the application.
        /// </summary>
        public override Version Version
        {
            get { return new Version(1, 0, 0, 0); }
        }

        /// <summary>
        /// Gets an instance of the implementation of the PSHostUserInterface
        /// class for this application. This instance is allocated once at startup time
        /// and returned every time thereafter.
        /// </summary>
        public override PSHostUserInterface UI
        {
            get { return this.myHostUserInterface; }
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
        /// Gets an appropriate string to identify you host implementation.
        /// Keep in mind that this string may be used by script writers to identify
        /// when your host is being used.
        /// </summary>
        public override string Name
        {
            get { return "MySampleConsoleHostImplementation"; }
        }
       
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
        /// <param name="exitCode">The exit code that the host application should use.</param>
        public override void SetShouldExit(int exitCode)
        {
            this.program.ShouldExit = true;
            this.program.ExitCode = exitCode;
        }
    }
}

