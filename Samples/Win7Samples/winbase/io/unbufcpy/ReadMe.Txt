Fast Copy Routines


The UNBUFCPY sample consists of two parts: UNBUFCP1 and UNBUFCP2.

The UNBUFCP1 sample shows a fast copy routine that uses I/O completion
ports. It is intended to demonstrate using a single thread to complete I/O
requests from multiple file handles.

Only one thread and one I/O completion port is used. The file handles for
the source and destination files are both associated with the same port. The
thread starts by posting a number of overlapped reads from the source file.
It then waits on the I/O completion port. Whenever a read completes, it
immediately turns it into a write to the destination file. Whenever a write
completes, it immediately posts the next read from the source file.

The UNBUFCP2 sample shows a fast copy routine that uses I/O completion
ports. It is intended to demonstrate how to complete I/O in a different
thread than the thread that started the I/O.

Two threads are used. The first thread posts overlapped reads from the
source file. These reads complete to an I/O completion port the second
thread is waiting on. The second thread sees the I/O completion and posts
an overlapped write to the destination file. The write completes to another
I/O completion port that the first thread is waiting on. The first thread
sees the I/O completion and posts another overlapped read.
