MachineActivation

This sample shows how to Activate a machine. For backwards compatibility with V1 clients, it takes an optional activation server URL as input.  It will use Service Discovery to find the activation server if a URL is not provided.  See detailed comments at the beginning of wmain().

The usage of this sample is as follows:

Usage:
  MachineActivation [-A ActivationSvr]
    -A: specifies the Activation Server. (optional)
        example: http://localhost/_wmcs/certification
