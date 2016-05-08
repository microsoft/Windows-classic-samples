
The RdcSdkTestServer sample:

This sample is a DCOM server to allow remote users to request signatures
for any accessible files.  This server uses the MSRDC to generate
signatures, and then allows clients to read the signature and file data.

This sample Server has several limitations. A few of the limitations are as
follows:

1.  The generation of signatures is synchronous - the client has to wait for
    signatures to be generated.  A multithreaded client can make several calls
    into the server for different files, but each client thread will wait for
    the corresponding server thread to complete signature generation.

2.  It does not try to detect and resolve conflicts - such that if several
    clients request the same file, only one of the clients will succeed.

3.  It stores the signature files as additional files in the same directory as
    the file begin transferred.  For example, if the client
    requests "C:\temp\foo.txt", additional files will be created:
    "C:\temp\foo.txt_1.sig", "C:\temp\foo.txt_2.sig", "C:\temp\foo.txt_3.sig".
    These files are always deleted by this sample server.

4.  The generated signatures files may not be secured as well as the
    source file.  This sample doesn't attempt to match or surpass the security
    settings of the file being transferred.


The RdcSdkTestClient sample:

This sample works with the RdcSdkTestServer DCOM server and the MSRDC
in-proc COM server to transfer files from a remote machine to the local
machine.  The RdcSdkTestServer and MSRDC server must be installed on both
machines first (using regsvr32.exe).  The client will connect to the
RdcSdkTestServer on the remote machine, and on the local machine.  The
client uses RdcSdkTestServer to generate signatures, and read the source
signature files, the seed signature files, the source file and the seed
file.  The client then uses the MSRDC server directly to compare the sets
of signatures and to create the target file.

This sample Client has several limitations. A few of the limitations are as
follows:

1.  It requires that complete path names (not UNC) be used for both the remote
    and local files.

2.  Primitive similarity is supported.  Each time the client transfers a file,
    its similarity traits are added to "RdcSampleTraitsTable", and the
    fileindex is saved in a file on disk -- "RdcSampleFilenameTable".

    The SimpleFilenameTable implementation is not robust.  If this file gets
    corrupted, it must be deleted.

3.  It can only transfer one file at a time.

4.  It doesn't verify the correctness of the target file.  It is expected
    that a complete application making use of RDC would checksum the entire source
    and target files to ensure that they match.  This check serves to catch any
    problems that may have been missed if cached signatures files have gotten out
    of sync with the files, and application bugs.
