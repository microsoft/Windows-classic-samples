========================================================================
    CONSOLE APPLICATION : ExpressW Project Overview
========================================================================

This is Express Writer sample application. This application exercises
two scenarios supported by Express Writer.


1. Create new Express Writer and register it in the system

This scenario allows you to perform full Express Writer creation and
registration on the target machine. Express Writer creation on the
machine gives the opportunity to adapt Writer's schema to the storage
location on the customer machine.

For example if FooApplication stores its data in a directory selected
by an administrator during installation, mechanisms exercised by this
scenario can be used to protect that storage location with the Express
Writer. Due to the configurable nature of the storage location,
Express Writer metadata cannot be pre-created and simply registered
on the customer machine.

This code can be used by your application to configure its writer
during installation or on the first execution assuming that your
storage varies between customer machines.


2. Create new Express Writer and save it to a file, then load it from
   a file and register in the system

This scenario allows developer to perform full Express Writer creation
offline (during development) and later perform registration on the
target machine by reading the pre-created metadata file. This metadata
file can be delivered alongside the application binaries. This code
can be used in two cases. First, when storage location is the same for
all of the client machines. Second, as the metadata for the default
installation directory.

This scenario requires two steps. First Express Writer metadata must be
created by the developer. It should cover the storage location uniformly
across all the deployment scenarios. This metadata should be delivered
with the application. Then using simple code executed on the customer
machine, metadata can be registered in the system.

This code can be used by your application to configure its writer during
installation or on the first execution assuming that your storage
location is the same on each machine where your application is being
deployed. Later, if the storage locations changes, application can
reconfigure its storage using code from the previous sample.


Sample code covers steps necessary to unregister your Express Writer.
It is very important to perform this step when a user uninstalls your
application.
