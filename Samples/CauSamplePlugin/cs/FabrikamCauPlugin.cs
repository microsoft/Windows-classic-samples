//-----------------------------------------------------------------------
// <copyright file="FabrikamCauPlugin.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//-----------------------------------------------------------------------
using System;
using System.Collections;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Management;
using System.Management.Automation;
using System.Runtime.Serialization;
using System.Security;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.ClusterAwareUpdating;

namespace Microsoft.Samples.ClusterAwareUpdating.CauSamplePlugin
{
    //
    // This code defines a CAU plugin that will run an arbitrary (administrator-specified)
    // command on each cluster node. It has one required argument, "Command", which is the
    // command that will be run. For example:
    //
    //    Invoke-CauRun -ClusterName MyTestCluster -CauPluginName FabrikamCauPlugin -CauPluginArguments @{ "Command"="cmd.exe /c echo Hello." } -Verbose
    //
    // When the plugin performs a scan, it will detect a single "update" applicable to
    // each node, which is the command to run. Staging doesn't do anything except report
    // that it is ready to install the update (run the command). When the plugin
    // "installs" the update, it runs the command on the target machine using WMI, and
    // waits for it to exit. If the remote process exits with a non-zero error code, the
    // update is considered failed.
    //
    // Because this plugin uses WMI, it requires that your firewalls on both the
    // orchestrator machine and the cluster nodes have the appropriate rules enabled. An
    // easy one to overlook is that the orchestrator must allow inbound WMI connections in
    // order to receive events from the cluster nodes when the commands finish. The plugin
    // will attempt to test its ability to use WMI with each node.
    //

    // This attribute has the name of the plugin that gets registered when you use
    // Register-CauPlugin.
    [ClusterUpdatingPlugin( "FabrikamCauPlugin" )]
    internal sealed class FabrikamCauPlugin : IClusterUpdatingPlugin
    {
        public FabrikamCauPlugin(string clusterName,
                                 PSCredential credential,
                                 Hashtable arguments,
                                 CancellationToken cancelToken)
        {
            m_clusterName = clusterName;
            m_credential = credential;
            m_arguments = arguments;
            m_cancelToken = cancelToken;
        }

        private string m_clusterName;
        private PSCredential m_credential;
        private Hashtable m_arguments;
        private CancellationToken m_cancelToken;


        public IClusterNodeUpdater CreateUpdater( string machineName,
                                                  Guid runId )
        {
            string command = m_arguments[ "Command" ] as string;
            if( String.IsNullOrEmpty( command ) )
            {
                string msg = "The Fabrikam CAU Plugin requires an Command argument which consists of a command to be run on each cluster node.";
                throw new ClusterUpdateException( msg,
                                                  "MissingCommandArg",
                                                  ErrorCategory.InvalidArgument );
            }

            return new FabrikamUpdater( machineName, m_credential, m_cancelToken, runId, command );
        }

        public void Dispose()
        {
            // Code to clean up the plugin as a whole goes here.
        }
    } // end class FabrikamCauPlugin


    [Serializable]
    public class FabrikamException : Exception
    {
        public FabrikamException() : base() { }
        public FabrikamException( string message ) : base( message ) { }
        public FabrikamException( string message, Exception inner ) : base( message, inner ) { }
        protected FabrikamException( SerializationInfo info, StreamingContext context ) : base( info, context ) { }
    } // end class FabrikamException


    //
    // This class represents an object that is used to scan, stage, and update a
    // specific cluster node.
    //
    internal sealed class FabrikamUpdater : IClusterNodeUpdater
    {
        private string m_machine;
        private PSCredential m_cred;
        private CancellationToken m_cancelToken;
        private Guid m_runId;
        private string m_command;
        private UpdateInfo m_update;


        public FabrikamUpdater( string machine,
                                PSCredential cred,
                                CancellationToken cancelToken,
                                Guid runId,
                                string command )
        {
            if( String.IsNullOrEmpty( machine ) )
                throw new ArgumentException( "You must supply a machine name.", "machine" );

            // If cred is null, that means just use the security context of the user
            // running this process.

            if( String.IsNullOrEmpty( command ) )
                throw new ArgumentException( "You must supply a command to run.", "command" );

            m_machine = machine;
            m_cred = cred;
            m_cancelToken = cancelToken;
            m_runId = runId;
            m_command = command;

            m_update = new UpdateInfo( m_machine,
                                       m_command,
                                       "Fabrikam CAU Command Update",
                                       "This update will run the command specified by the administrator on each cluster node. The command is: " + m_command );

            _TestWmiConnection();
        } // end constructor


        public async Task ScanAsync( IScanCallback scanCallback )
        {
            // Note that we only allow ClusterUpdateExceptions out; the CAU cmdlet will
            // not handle other exception types, so if any other exception type flies out
            // of here, it will cause an abnormal termination of the CAU cmdlet. (with the
            // exception of OperationCanceledException)
            try
            {
                await Task.Factory.StartNew( () => _DoScan( scanCallback ), m_cancelToken );
            }
            catch( FabrikamException fe )
            {
                throw new ClusterUpdateException( String.Format( CultureInfo.CurrentCulture,
                                                                 "Scanning node \"{0}\" failed: {1}",
                                                                 m_machine,
                                                                 fe.Message ),
                                                  "ScanFailed",
                                                  ErrorCategory.NotSpecified,
                                                  fe );
            }
        } // end ScanAsync()


        // Our "scan" needs to detect the "update" just one time, because after installing
        // updates, Invoke-CauRun will call ScanAsync again to look for "second-order"
        // updates (updates that only become applicable after installing the first set of
        // updates).
        private bool m_alreadyReported;

        private void _DoScan( IScanCallback scanCallback )
        {
            if( !m_alreadyReported )
            {
                scanCallback.ReportApplicableUpdate( m_update );
                m_alreadyReported = true;
            }
        } // end _DoScan()


        public async Task StageAsync( IStageCallback stageCallback )
        {
            try
            {
                await Task.Factory.StartNew( () => _DoStaging( stageCallback ), m_cancelToken );
            }
            catch( FabrikamException fe )
            {
                // Note that if an "update" fails to stage, it is reported through the
                // stageCallback.ReportStagingResult method. Only a more serious problem
                // representing a complete infrastructure failure should be reported by
                // throwing a ClusterUpdateException.
                throw new ClusterUpdateException( String.Format( CultureInfo.CurrentCulture,
                                                                 "Staging node \"{0}\" failed: {1}",
                                                                 m_machine,
                                                                 fe.Message ),
                                                  "StageFailed",
                                                  ErrorCategory.NotSpecified,
                                                  fe );
            }
        } // end StageAsync()


        private void _DoStaging( IStageCallback stageCallback )
        {
            // Nothing to do except report that we're ready to run the command.
            // All timestamps should be UTC.
            UpdateStagingResult usr = new UpdateStagingResult( m_update, ResultCode.Succeeded, 0, DateTime.UtcNow );
            stageCallback.ReportStagingResult( usr );
        } // end _DoStaging()


        public async Task InstallAsync( IInstallCallback installCallback )
        {
            try
            {
                await Task.Factory.StartNew( () => _DoInstall( installCallback ),
                                             m_cancelToken,
                                             TaskCreationOptions.LongRunning,
                                             TaskScheduler.Default );
            }
            catch( FabrikamException fe )
            {
                // Note that if an "update" fails to install, it is reported through the
                // installCallback.ReportInstallResult method. Only a more serious problem
                // representing a complete infrastructure failure should be reported by
                // throwing a ClusterUpdateException.
                throw new ClusterUpdateException( String.Format( CultureInfo.CurrentCulture,
                                                                 "Installing node \"{0}\" failed: {1}",
                                                                 m_machine,
                                                                 fe.Message ),
                                                  "InstallFailed",
                                                  ErrorCategory.NotSpecified,
                                                  fe );
            }
        } // end InstallAsync()


        private void _DoInstall( IInstallCallback installCallback )
        {
            installCallback.WriteVerbose( String.Format( CultureInfo.CurrentCulture,
                                                         "Attempting to run this command on {0}: {1}",
                                                         m_machine,
                                                         m_command ) );
            Exception e = null;
            int errorCode = 0;
            UpdateInstallResult uir = null;
            try
            {
                errorCode = _RunCommand( m_command, false, installCallback );
            }
            catch( Win32Exception w32e ) { e = w32e; }
            catch( ManagementException me ) { e = me; }
            catch( OperationCanceledException )
            {
                uir = new UpdateInstallResult( m_update,
                                               ResultCode.Canceled,
                                               0,                 // error code
                                               false,             // reboot required
                                               false,             // long reboot hint
                                               DateTime.UtcNow ); // All timestamps should be UTC.
                installCallback.ReportInstallResult( uir );
                throw;
            }

            if( null != e )
            {
                throw new FabrikamException( String.Format( CultureInfo.CurrentCulture,
                                                            "Failed to run command: {0}: {1}",
                                                            e.GetType().Name,
                                                            e.Message ),
                                             e );
            }

            bool rebootRequired = false;
            bool succeeded = false;
            const int ERROR_SUCCESS_REBOOT_REQUIRED  = 3010;
            const int ERROR_SUCCESS_RESTART_REQUIRED = 3011;
            const int ERROR_FAIL_REBOOT_REQUIRED     = 3017;

            if( 0 == errorCode )
            {
                succeeded = true;
            }
            else if( (ERROR_SUCCESS_REBOOT_REQUIRED == errorCode) ||
                     (ERROR_SUCCESS_RESTART_REQUIRED == errorCode) )
            {
                succeeded = true;
                rebootRequired = true;
            }
            else if( 0 != errorCode )
            {
                succeeded = false;
                if( ERROR_FAIL_REBOOT_REQUIRED == errorCode )
                {
                    rebootRequired = true;
                }
            }

            uir = new UpdateInstallResult( m_update,
                                           succeeded ? ResultCode.Succeeded : ResultCode.Failed,
                                           errorCode,
                                           rebootRequired,
                                           false,
                                           DateTime.UtcNow ); // All timestamps should be UTC.
            installCallback.ReportInstallResult( uir );
        } // end _DoInstall()


        // This can throw a Win32Exception, a ManagementException, or an
        // OperationCanceledException.
        private int _RunCommand( string command, bool instaKill, ICauPluginCallbackBase callback )
        {
            int exitCode = -1;
            int processId = 0;
            string username = null;
            SecureString password = null;
            if( null != m_cred )
            {
                username = m_cred.UserName;
                password = m_cred.Password;
            }

            object gate = new object();

            ConnectionOptions co = new ConnectionOptions( null,        // locale
                                                          username,
                                                          password,
                                                          null,        // authority
                                                          ImpersonationLevel.Impersonate,
                                                          AuthenticationLevel.PacketPrivacy,
                                                          true,        // enable privileges
                                                          null,        // context
                                                          TimeSpan.Zero );
            string path = String.Format( CultureInfo.InvariantCulture, @"\\{0}\root\cimv2", m_machine );
            ManagementScope scope = new ManagementScope( path, co );
            ObjectGetOptions objGetOptions = new ObjectGetOptions();

            scope.Connect();

            // We start the query before launching the process to prevent any possibility
            // of a timing or PID recycling problem.
            WqlEventQuery query = new WqlEventQuery( "Win32_ProcessStopTrace" );
            EventWatcherOptions watcherOptions = new EventWatcherOptions( null, TimeSpan.MaxValue, 1 );
            using( ManagementEventWatcher watcher = new ManagementEventWatcher( scope, query, watcherOptions ) )
            {
                watcher.EventArrived += (sender, arg) =>
                {
                    int stoppedProcId = (int) (uint) arg.NewEvent[ "ProcessID" ];
                    if( stoppedProcId == processId )
                    {
                        exitCode = (int) (uint) arg.NewEvent[ "ExitStatus" ];

                        if( null != callback )
                        {
                            callback.WriteVerbose( String.Format( CultureInfo.CurrentCulture,
                                                                  "Process ID {0} on {1} exited with code: {2}",
                                                                  processId,
                                                                  m_machine,
                                                                  exitCode ) );
                        }

                        lock( gate )
                        {
                            Monitor.PulseAll( gate );
                        }
                    }
                };

                watcher.Start();

                int timeout = Timeout.Infinite;
                ManagementPath mPath = new ManagementPath( "Win32_Process" );
                using( ManagementClass mc = new ManagementClass( scope, mPath, objGetOptions ) )
                using( ManagementBaseObject inParams = mc.GetMethodParameters( "Create" ) )
                {
                    inParams[ "CommandLine" ] = command;
                    using( ManagementBaseObject outParams = mc.InvokeMethod( "Create", inParams, null ) )
                    {
                        int err = (int) (uint) outParams[ "returnValue" ];
                        if( 0 != err )
                        {
                            throw new Win32Exception( err );
                        }
                        processId = (int) (uint) outParams[ "processId" ];
                        Debug.Assert( processId > 0 );
                    }
                }

                if( null != callback )
                {
                    callback.WriteVerbose( String.Format( CultureInfo.CurrentCulture,
                                                          "Process launched on {0} with ID: {1}",
                                                          m_machine,
                                                          processId ) );
                }

                // If instaKill is true, we are trying to test our ability to receive
                // Win32_ProcessStopTrace events, so kill the process immediately.
                if( instaKill )
                {
                    SelectQuery killQuery = new SelectQuery( "SELECT * FROM Win32_Process WHERE ProcessId = " + processId );
                    using( ManagementObjectSearcher searcher = new ManagementObjectSearcher( scope, killQuery ) )
                    {
                        foreach( ManagementObject obj in searcher.Get() )
                        {
                            obj.InvokeMethod( "Terminate", new object[] { (uint) c_TestWmiExitCode } );
                            obj.Dispose();
                        }
                    }

                    // If we don't receive the Win32_ProcessStopTrace event within 10
                    // seconds, we'll assume it's not coming. In that case, there's
                    // probably a firewall problem blocking WMI.
                    timeout = 10000;
                }

                // Wait until the process exits (or we get canceled).
                lock( gate )
                {
                    // If we get canceled, we stop waiting for the remote process to
                    // finish and just leave it running.
                    using( m_cancelToken.Register( () => { lock( gate ) { Monitor.PulseAll( gate ); } } ) )
                    {
                        if( !Monitor.Wait( gate, timeout ) )
                        {
                            Debug.Assert( instaKill, "This call should not time out unless we are in the instaKill scenario." );
                            throw new ClusterUpdateException( "The Fabrikam CAU plugin is able to create a process on cluster node \"{0}\" using WMI, but is not able to receive process lifetime events. Check the firewalls on both this computer and the target node and ensure that the appropriate WMI rules are enabled.",
                                                              "WmiTestReceiveEventFailed",
                                                              ErrorCategory.ResourceUnavailable );
                        }
                    }
                }

                watcher.Stop();
                m_cancelToken.ThrowIfCancellationRequested();
            }

            return exitCode;
        } // end _RunCommand()


        private const int c_TestWmiExitCode = 7;

        //
        // This method tests our ability to create a process on the target node and
        // monitor its exit. If we can't, it will throw a ClusterUpdateException.
        //
        private void _TestWmiConnection()
        {
            Exception e = null;
            try
            {
                int exitCode = _RunCommand( "cmd.exe", true, null );
                Debug.Assert( c_TestWmiExitCode == exitCode );
            }
            catch( Win32Exception w32e ) { e = w32e; }
            catch( ManagementException me ) { e = me; }

            if( null != e )
            {
                throw new ClusterUpdateException( String.Format( CultureInfo.CurrentCulture,
                                                                 "The Fabrikam CAU plugin was unable to create a process on the cluster node \"{0}\". Check that the appropriate firewall rules are enabled on both this computer and the target node. The error received was a {1}: {2}",
                                                                 m_machine,
                                                                 e.GetType().Name,
                                                                 e.Message ),
                                                  "WmiTestCreateProcessFailed",
                                                  ErrorCategory.OpenError,
                                                  e );
            }
        } // end _TestWmiConnection()


        public void Dispose()
        {
            // Cleanup code goes here.
        }
    } // end class FabrikamUpdater
}

