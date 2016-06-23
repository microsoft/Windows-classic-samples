AcquireTemplates

This sample shows how to acquire templates. It takes an optional licensing server URL as input.  It will use Service Discovery to find the server URLs if the URLs are not provided.  The sample requires that a UserID be provided as input.  See the comments at the beginning of wmain() for a more detailed description.

The usage of this sample is as follows:

Usage:
  AcquireTemplates -U UserID [-L LicensingSvr]
    -U: specifies the UserID.
        example: user@yourdomain.com
    -L: specifies the Licensing Server. (optional)
        example: http://localhost/_wmcs/licensing


*** NOTE: Requires Windows Server 2008 or Windows Vista SP1.