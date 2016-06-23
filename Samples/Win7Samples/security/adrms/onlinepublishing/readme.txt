OnlinePublishing

This sample demonstrates online publishing. It takes an optional licensing server URL as input.  It will use Service Discovery to find the server URL if it is not provided.  The sample requires that a UserID be provided as input.
See the comments at the beginning of wmain() for a more detailed description.

The usage of this sample is as follows:

Usage:
  OnlinePublishing -U UserID [-L LicensingSvr]
    -U: specifies the UserID.
        example: user@yourdomain.com
    -L: specifies the Licensing Server. (optional)
        example: http://localhost/_wmcs/licensing
