SBActivation

This sample shows how to Activate a machine and user using a server lockbox with the DRMSetGlobalOptions API.  This sample
will not work with a V1 client.  See the comments at the beginning of wmain() for a more detailed description.

The usage of this sample is as follows:

Usage:
  SBActivation -U UserID [-A ActivationSvr]
    -U: specifies the UserID.
        example: user@yourdomain.com
    -A: specifies the Activation Server. (optional)
        example: http://localhost/_wmcs/certification
