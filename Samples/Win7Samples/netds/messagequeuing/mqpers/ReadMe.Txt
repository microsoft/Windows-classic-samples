------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

Sample: mqpers

Purpose:
This application demonstrates how to create a persistent COM object in VC++ and ATL
which is then sent/received as an MSMQ message.

Requirements:
MSMQ 1.0 or greater, VC6 or greater, ATL3.0 or greater.

Overview:
Message Queuing supports sending/receiving COM objects that implement the
IPersistStream or IPersistStorage interfaces. This sample shows how to take
advantage of this feature using Visual C++ and ATL. Simple persistent Line and Point
objects That are able to serialize and deserialize their state are defined.

The sender side uses the GraphObj.dll to generate the specified object, and send the
object to a defined queue. If the queue doesn't exist it will be created on the
local computer.

The receiving side waits for a message and prints out the object state. 


Build:
To build the mqpers sample run nmake . This will compile the three sub projects that
compose the sample.


Run:

1. run the receiver side (Rcv.exe, located under the "Rcv\ReleaseU" folder) and wait
   for incoming objects.
2. run the sender side (Snd.exe, located under the "Snd\ReleaseU" folder) and choose
   an object to send.


scheme:

                                  )
 +++++++++++                      )
 +         +===() IPersistStream  )
 + CLine   +                      )
 +         +                      )
 +++++++++++                      )         +     ===========
                                  )          +    -         -
                                  )           +   - Sender  -
                                  )----------- +  -         -
                                  )           +   ===========
 +++++++++++                      )          +         |
 +         +===() IPersistStream  )         +          |
 + CPoint  +                      )                    |
 +         +                      )                    |
 +++++++++++                      )                    |		      
                                  )                    |
                                                       |
                                                       |
                                                       | MSMQ (Send)
                                                       |
                                                       |
                                                       |
                                                       |
                                                      ###
                                                       #
                                                  +++++++++++
                                                  +         +
                                                  +  Queue  +
                                                  +         +
                                                  +++++++++++
                                                       |
                                                       |
                                                       | MSMQ (Receive)
                                                       |
                                                       |
                                                      ###
                                                       #
                                                 =============
                                                 -           -
                                                 -  Receiver -
                                                 -           -
                                                 =============




