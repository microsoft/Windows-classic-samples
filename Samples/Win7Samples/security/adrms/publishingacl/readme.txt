PublishingACL

This sample shows how to create an issuance license. It takes a required path to a directory as input.  It will use the ACLs on that directory to put users and rights into an issuance license.  The directory provided as input must have at least one user and/or group in its ACL list.  See the comments at the beginning of wmain() for a more detailed description.  

Note: To retrieve an object's DACL, you must be the object's owner or have READ_CONTROL access to the object. To run this sample, you must have READ_CONTROL on the path you provide on the command line.

The usage of this sample is as follows:

Usage:
  PublishingACL -P Path
    -P: Path to the directory to use for the issuance license's rights.
        example: c:\\myDir

On a Vista or Server 2008 machine, you must run this sample elevated.
