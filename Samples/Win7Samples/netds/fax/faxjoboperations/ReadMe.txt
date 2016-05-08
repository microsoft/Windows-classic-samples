========================================================================
   SAMPLE :  Cancel Fax 
========================================================================

Description:
-----------------

This sample demonstrates how to cancel a fax in the Outgoing queue. The program connects to a fax 
server, and  blocks and pauses its outgoing queue. This prevents the addition of new faxes to the 
queue (blocked) and the sending of faxes from the queue (paused). This allows you to manage the 
cancellation process on a static outgoing queue. When 'q' is entered the queue is unblocked and unpaused. 
If you terminate the program in any other way, the queue will remain blocked and paused.
To unblock the queue, run the FaxJobOperations and exit it with option 'q'.

If you cancel an individual fax from a broadcast, that fax has the status Canceled, but remains in the outgoing 
queue, and the Count property is not modified.


PreCondition:
-------------------

This is supported for Windows Vista SKU.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

The program can be run as FaxJobOperations.exe  /s <FaxServer>

If /s paramater is not given then the default Fax Server is the local server. 

It will first print the list of jobs with their job ids, subject, submission id and sender name. Then the job id can be entered to
cancel the job.