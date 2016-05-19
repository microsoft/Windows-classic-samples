================================================================================
                WIN32 Console Application : DRMUpdate
================================================================================

This sample demonstrates how to perform individualization on a client computer
by using the Windows Media DRM Extended APIs. 



Usage: DRMUpdate <LOG_FILE> [-force]

  <LOG_FILE> = Name of a text file to which status will be logged. If the 
               specified file exists it will be overwritten.

  -force     = If set, this flag configures the DRM subsystem to perform 
               individualization regardless of the current security version on 
               the client computer. If not set, the subsystem will determine 
               whether individualization is required