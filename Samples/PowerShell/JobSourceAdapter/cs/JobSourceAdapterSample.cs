// <copyright file="JobSourceAdapterSample.cs" company="Microsoft Corporation">
// Copyright (c) 2012 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.IO;
using System.Threading;

namespace JobSourceAdapterSample
{
    /// <summary>
    /// Simple cmdlet to create a FileCopyJob.
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "FileCopyJob")]
    [OutputType(typeof(Job2))]
    public sealed class StartFileCopyJobCommand : PSCmdlet
    {
        #region Parameters

        [Parameter(Position = 0, Mandatory = true)]
        [ValidateNotNullOrEmpty]
        public string Name { get; set; }

        [Parameter(Position = 1, Mandatory = true)]
        [ValidateNotNullOrEmpty]
        public string SourcePath { get; set; }

        [Parameter(Position = 2, Mandatory = true)]
        [ValidateNotNullOrEmpty]
        public string DestinationPath { get; set; }

        #endregion

        #region Overrides

        protected override void ProcessRecord()
        {
            // Get full path to source.
            ProviderInfo provider;
            string fullSourcePath = GetResolvedProviderPathFromPSPath(SourcePath, out provider).FirstOrDefault();
            if (string.IsNullOrEmpty(fullSourcePath))
            {
                throw new ArgumentException(SourcePath);
            }

            // Get full path for destination file (which may not exist).
            string fullDestPath = null;
            string destFile = Path.GetFileName(DestinationPath);
            string path = GetResolvedProviderPathFromPSPath(
                Path.GetDirectoryName(DestinationPath), out provider).FirstOrDefault();
            if (path != null)
            {
                fullDestPath = Path.Combine(path, destFile);
            }
            if (string.IsNullOrEmpty(fullDestPath))
            {
                throw new ArgumentException(DestinationPath);
            }

            // Create job source adapter.
            FileCopyJobSourceAdapter jobSourceAdapter = new FileCopyJobSourceAdapter();

            // Create FileCopyJob parameters (source and destination paths).
            Dictionary<string, object> copyJobParameters = new Dictionary<string, object>();
            copyJobParameters.Add(FileCopyJobSourceAdapter.SourcePathProperty, fullSourcePath);
            copyJobParameters.Add(FileCopyJobSourceAdapter.DestinationPathProperty, fullDestPath);

            // Create job specification.
            JobInvocationInfo copyJobSpecification = new JobInvocationInfo(
                new JobDefinition(typeof(FileCopyJobSourceAdapter), string.Empty, Name),
                copyJobParameters);
            copyJobSpecification.Name = Name;

            // Create file copy job from job source adapter and start it.
            Job2 fileCopyJob = jobSourceAdapter.NewJob(copyJobSpecification);
            fileCopyJob.StartJob();

            WriteObject(fileCopyJob);
        }

        #endregion
    }

    /// <summary>
    /// Sample FileCopyJob class derived from Job2.
    /// This sample job runs until stopped and monitors the provided
    /// source file for changes and copies it to the destination path
    /// on change.
    /// </summary>
    public sealed class FileCopyJob : Job2
    {
        #region Private members

        private const string FileCopyJobTypeName = "FileCopyJob";
        private FileSystemWatcher _fileWatcher;
        private string _sourcePath;
        private string _destinationPath;

        #endregion

        #region Constructor

        private FileCopyJob()
        { }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="jobName">Job name</param>
        /// <param name="sourcePath">Source file path</param>
        /// <param name="destinationPath">Copy to destination file path</param>
        public FileCopyJob(
            string jobName,
            string sourcePath,
            string destinationPath)
        {
            if (string.IsNullOrEmpty(sourcePath))
            {
                throw new ArgumentException("sourcePath");
            }
            if (string.IsNullOrEmpty(destinationPath))
            {
                throw new ArgumentException("destPath");
            }

            _sourcePath = sourcePath;
            _destinationPath = destinationPath;

            this.Name = !string.IsNullOrEmpty(jobName) ? jobName : "FileCopyJob";
            this.PSJobTypeName = FileCopyJobTypeName;

            // File watcher.
            _fileWatcher = new FileSystemWatcher(Path.GetDirectoryName(_sourcePath));
            _fileWatcher.Filter = Path.GetFileName(_sourcePath);
            _fileWatcher.EnableRaisingEvents = false;
            _fileWatcher.NotifyFilter = NotifyFilters.LastWrite;
            _fileWatcher.Changed += new FileSystemEventHandler(HandleFileWatcherChanged);

            // Job state changed callback.
            this.StateChanged += new EventHandler<JobStateEventArgs>(HandleJobStateChanged);
        }

        #endregion

        #region Public properties

        public string SourcePath
        {
            get { return _sourcePath; }
        }

        public string DestinationPath
        {
            get { return _destinationPath; }
        }

        #endregion

        #region Public methods

        public override void StartJob()
        {
            if (this.JobStateInfo.State != JobState.NotStarted)
            {
                throw new InvalidOperationException("Cannot start job.");
            }

            SetJobState(JobState.Running);
            _fileWatcher.EnableRaisingEvents = true;
        }

        public override void StartJobAsync()
        {
            StartJob();
            OnStartJobCompleted(new System.ComponentModel.AsyncCompletedEventArgs(null, false, null));
        }

        public override void StopJob()
        {
            if (!IsFinishedState(this.JobStateInfo.State))
            {
                SetJobState(JobState.Stopped);
            }
        }

        public override void StopJobAsync()
        {
            StopJob();
            OnStopJobCompleted(new System.ComponentModel.AsyncCompletedEventArgs(null, false, null));
        }

        public override void StopJob(bool force, string reason)
        {
            StopJob();
        }

        public override void StopJobAsync(bool force, string reason)
        {
            StopJobAsync();
        }

        public override void SuspendJob()
        {
            switch (this.JobStateInfo.State)
            {
                case JobState.Suspended:
                    break;

                case JobState.Running:
                    _fileWatcher.EnableRaisingEvents = false;
                    SetJobState(JobState.Suspended);
                    break;

                default:
                    throw new InvalidOperationException("Cannot suspend job.");
            }
        }

        public override void SuspendJobAsync()
        {
            SuspendJob();
            OnSuspendJobCompleted(new System.ComponentModel.AsyncCompletedEventArgs(null, false, null));
        }

        public override void SuspendJob(bool force, string reason)
        {
            SuspendJob();
        }

        public override void SuspendJobAsync(bool force, string reason)
        {
            SuspendJobAsync();
        }

        public override void ResumeJob()
        {
            switch (this.JobStateInfo.State)
            {
                case JobState.Running:
                    break;

                case JobState.Suspended:
                    SetJobState(JobState.Running);
                    _fileWatcher.EnableRaisingEvents = true;
                    break;

                default:
                    throw new InvalidOperationException("Cannot resume job.");
            }
        }

        public override void ResumeJobAsync()
        {
            ResumeJob();
            OnResumeJobCompleted(new System.ComponentModel.AsyncCompletedEventArgs(null, false, null));
        }

        public override void UnblockJob()
        {
            throw new NotImplementedException();
        }

        public override void UnblockJobAsync()
        {
            throw new NotImplementedException();
        }

        public override bool HasMoreData
        {
            get
            {
                return (Output.Count > 0 ||
                        Error.Count > 0);
            }
        }

        public override string Location
        {
            get { return "localhost"; }
        }

        public override string StatusMessage
        {
            get { return string.Empty; }
        }

        #endregion

        #region IDispose

        protected override void Dispose(bool disposing)
        {
            if (!IsFinishedState(this.JobStateInfo.State))
            {
                SetJobState(JobState.Stopped);
            }

            base.Dispose(disposing);
        }

        #endregion

        #region Private methods

        private void HandleFileWatcherChanged(object sender, FileSystemEventArgs e)
        {
            if (IsFinishedState(this.JobStateInfo.State))
            {
                return;
            }

            Exception failed = null;
            try
            {
                File.Copy(_sourcePath, _destinationPath, true);
            }
            catch (IOException ex)
            {
                failed = ex;
            }
            catch (UnauthorizedAccessException ex)
            {
                failed = ex;
            }

            if (failed != null && Error.IsOpen)
            {
                try
                {
                    Error.Add(new ErrorRecord(failed, "FileCopyFailed", ErrorCategory.WriteError, this));
                }
                catch (PSInvalidOperationException)
                {
                    // Can't write to closed buffer.
                }
            }
        }

        private void HandleJobStateChanged(object sender, JobStateEventArgs e)
        {
            if (IsFinishedState(e.JobStateInfo.State))
            {
                // Job transitioned to finished state.
                this.StateChanged -= new EventHandler<JobStateEventArgs>(HandleJobStateChanged);
                DisposeFileWatcher();
            }
        }

        private void DisposeFileWatcher()
        {
            if (_fileWatcher != null)
            {
                _fileWatcher.Changed -= new FileSystemEventHandler(HandleFileWatcherChanged);
                _fileWatcher.Dispose();
                _fileWatcher = null;
            }
        }

        private bool IsFinishedState(JobState state)
        {
            return (state == JobState.Completed ||
                    state == JobState.Stopped ||
                    state == JobState.Failed);
        }

        #endregion
    }

    /// <summary>
    /// Sample JobSourceAdapter for a file copy job.
    /// Creates new CopyFileJob jobs.
    /// Maintains repository for CopyFileJobs.
    /// </summary>
    public sealed class FileCopyJobSourceAdapter : JobSourceAdapter
    {
        #region Private members

        private const string AdapterTypeName = "FileCopyJobSourceAdapter";

        private static List<Job2> JobRepository = new List<Job2>();

        #endregion

        #region Public strings

        // FileCopy job properties.
        public const string SourcePathProperty = "SourcePath";
        public const string DestinationPathProperty = "DestinationPath";

        #endregion

        #region Constructor

        public FileCopyJobSourceAdapter()
        {
            this.Name = AdapterTypeName;
        }

        #endregion

        #region Public methods

        public override Job2 NewJob(JobInvocationInfo specification)
        {
            if (specification == null)
            {
                throw new NullReferenceException("specification");
            }

            if (specification.Parameters.Count != 1)
            {
                throw new ArgumentException("JobInvocationInfo specification parameters not specified.");
            }

            // Retrieve source and destination path information from specification
            // parameters.
            string sourcePath = null;
            string destinationPath = null;
            CommandParameterCollection parameters = specification.Parameters[0];
            foreach (var item in parameters)
            {
                if (item.Name.Equals(SourcePathProperty, StringComparison.OrdinalIgnoreCase))
                {
                    sourcePath = item.Value as string;
                }
                else if (item.Name.Equals(DestinationPathProperty, StringComparison.OrdinalIgnoreCase))
                {
                    destinationPath = item.Value as string;
                }
            }

            // Create FileCopyJob
            FileCopyJob rtnJob = new FileCopyJob(specification.Name, sourcePath, destinationPath);
            lock (JobRepository)
            {
                JobRepository.Add(rtnJob);
            }
            return rtnJob;
        }

        public override void RemoveJob(Job2 job)
        {
            lock (JobRepository)
            {
                if (JobRepository.Contains(job))
                {
                    JobRepository.Remove(job);
                }
            }

            job.Dispose();
        }

        public override IList<Job2> GetJobs()
        {
            lock (JobRepository)
            {
                return JobRepository.ToArray<Job2>();
            }
        }

        public override Job2 GetJobByInstanceId(Guid instanceId, bool recurse)
        {
            lock (JobRepository)
            {
                foreach (var job in JobRepository)
                {
                    if (job.InstanceId == instanceId)
                    {
                        return job;
                    }
                }
            }

            return null;
        }

        public override Job2 GetJobBySessionId(int id, bool recurse)
        {
            lock (JobRepository)
            {
                foreach (var job in JobRepository)
                {
                    if (job.Id == id)
                    {
                        return job;
                    }
                }
            }

            return null;
        }

        public override IList<Job2> GetJobsByName(string name, bool recurse)
        {
            List<Job2> rtnJobs = new List<Job2>();
            WildcardPattern namePattern = new WildcardPattern(name, WildcardOptions.IgnoreCase);
            lock (JobRepository)
            {
                foreach (var job in JobRepository)
                {
                    if (namePattern.IsMatch(job.Name))
                    {
                        rtnJobs.Add(job);
                    }
                }
            }

            return rtnJobs;
        }

        public override IList<Job2> GetJobsByState(JobState state, bool recurse)
        {
            List<Job2> rtnJobs = new List<Job2>();
            lock (JobRepository)
            {
                foreach (var job in JobRepository)
                {
                    if (job.JobStateInfo.State == state)
                    {
                        rtnJobs.Add(job);
                    }
                }
            }

            return rtnJobs;
        }

        public override IList<Job2> GetJobsByCommand(string command, bool recurse)
        {
            throw new NotImplementedException();
        }

        public override IList<Job2> GetJobsByFilter(Dictionary<string, object> filter, bool recurse)
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
