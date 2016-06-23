UserActivation

This sample shows how to activate a user. It takes an optional activation server URL as input.  It will use Service Discovery 
to find the activation server if a URL is not provided.  The sample requires that a UserID be provided as input.  See the comments at the beginning of wmain() for a more detailed description.

The usage of this sample is as follows:

Usage:
  UserActivation -U UserID [-A ActivationSvr]
    -U: specifies the UserID.
        example: user@yourdomain.com
    -A: specifies the Activation Server. (optional)
        example: http://localhost/_wmcs/certification
