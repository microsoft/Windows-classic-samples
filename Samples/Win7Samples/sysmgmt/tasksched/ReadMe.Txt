Overview:
    SA.EXE is sample code that demonstrates the new scheduling agent API
provided with installation of the Task Scheduler package;  complete
documentation describing the interfaces is available elsewhere in the 
SDK.  The sample mimics the functionality of the AT command 
through the Task Scheduler APIs, as opposed to the NetSchedule API 
used by AT.  This sample implements only a small subset of the AT 
command set, but should be easily extensible.  This sample also runs on
Win9x platforms.

Usage:
        sa                                                - Show All Tasks
        sa /?                                             - Display Help
        sa JobName.job /DELETE                            - delete job
        sa JobName.job Time Command [UserName [Password]] - submit a job

Job Time is in 24 hour format (such as 15:30) and is the next instance 
of this specific time within 24 hours.

Command is the name of the application to run.  Path names may be included.
However, spaces are not allowed in the path name, so an application residing
in C:\program files\foo.exe should be specified as C:\progra~1\foo.exe,
using the old-style 8.3 filenames.  Should any command line parameters
be required, enclose the entire Command string in quotes:
    "c:\progra~1\foo.exe /bar 5"

Username and password are required to run the specified task under Windows NT.
Under Windows NT, if they are not specified as part of the command line, 
a prompt for the missing data will be displayed.  Under Windows 95, 
both fields are ignored, if specified.  Regardless of specification,
under Windows 95, no prompt will be displayed.

Error Reporting:
    The program performs error checking within the code over the parameters
passed in.  Any errors are returned as the exit value of "main", which may
be read by checking the environmental value %ERRORLEVEL% under Windows NT,
or the corresponding variable in Windows 95.   

Additional Notes:
    The program makes use of the following interfaces and methods:
    
    1. ITaskScheduler::Enum
       ITaskScheduler::Activate
       ITaskScheduler::Delete
       ITaskScheduler::NewTask

    2. IEnumTasks::Next

    3. ITask::GetTriggerCount
       ITask::GetTriggerString
       ITask::SetAccountInformation
       ITask::SetApplicationName
       ITask::SetParameters
       ITask::SetComment
       ITask::CreateTrigger

    4. ITaskTrigger::SetTrigger

    5. IScheduledWorkItem::SetFlags

    In addition, various methods on IUnknown and IPersistFile are called.
It is assumed that a user of this code has familiarity with those 
interfaces and calling conventions.  

    The output of the program is one of four possibilities:

    First, and most obviously, is the presentation of a help file.  This
case occurs from command line arguments requesting help, or from input
parameter lists that are longer than the expected range. Output is to
stdout.

    Second, the program can enumerate all task objects in the tasks folder.
Output is provided to stdout, and consists of a line containing
"Job: filename.job" followed by indented lines containing strings of the
type "Trigger: text representation of trigger values".  Empty task folders
cause no output.

    Third, the program can delete jobs from the job folder.  This action
will fail in a secure environment (Windows NT on an NTFS partition) if
the caller does not have permissions to remove the file.  Deletion
of tasks is not limited to those that were created from this program.
If the deletion succeeds, no output is displayed.

    Finally, the program can add a task from the command line.  If the
scheduling agent is to recognize the task, the filename (jobname.job) 
given MUST have the extension ".job" on it.  Otherwise, it will fail
to be enumerated from this tool, the task scheduler, or anywhere else.
In addition, that task will fail to run if ".job" is not the file
extension.  If the addition succeeds, no output is displayed.

    The task type that is created has a single trigger of type "ONCE"
and is limited to running within the next 24 hours.  Also, like AT,
these jobs will be deleted upon completion of their run.  For
creation of other types of triggers, please consult the documentation
for the TASK_TRIGGER structure.

