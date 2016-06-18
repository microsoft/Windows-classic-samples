AcquireClientLicensor

This sample shows how to acquire a client licensor certificate. It takes an optional activation and certification server URL as input.  It will use Service Discovery to find the server URLs if the URLs are not provided.  The sample requires that a UserID be provided as input.  See the comments at the beginning of wmain() for a more detailed description.

The usage of this sample is as follows:

Usage:
  AcquireClientLicensor -U UserID [-A ActivationSvr] [-L LicensingSvr]
    -U: specifies the UserID.
        example: user@yourdomain.com
    -A: specifies the Activation Server. (optional)
        example: http://localhost/_wmcs/certification
    -L: specifies the Licensing Server. (optional)
        example: http://localhost/_wmcs/licensing
