------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

This sample demonstrates how to use the MSMQ ActiveX components with the #import 
compiler directive new to Visual C++ 5.0. This program also demonstrates how to         
perform asynchronous receive using MSMQ ActiveX Events in C++.

This sample program functions identically to the VB_Draw and C_Draw samples included 
with the MSMQ product and will inter-operate with those two samples.

In order to handle ActiveX Event notification a new class, CMSMQEventHandler, has been
defined.  This class can be used in other projects by deriving a new class from 
CMSMQEventHandler and overriding the Arrived and ArrivedError methods.

Aside from this new class the majority of code in this project was taken or adapted 
from the existing C_Draw and VB_Draw samples.

==========================================================================
				INTRODUCTION
==========================================================================

The #import compiler directive makes COM objects easier to use with C++.  
This sample program functions identically to the VB_Draw and C_Draw samples 
included with the MSMQ product, and will interoperate with those two samples.  

In order to handle ActiveX Event notification, a new class, CMSMQEventHandler, 
has been defined.  This class can be used in other projects by deriving a new 
class from CMSMQEventHandler and overriding the Arrived and ArrivedError methods.

This sample can run both on DS enabled and DS disabled computers.
When connecting two DS enabled computers, standard connection mode can be used, meaning that
public queues are created and that a query on the DS is made. But if at least one of the computers is 
DS disabled, a direct connection must be established. This means that only private queues are created. 
In that case, no query on the DS can be done, so the user has to specify the name of the computer to connect 
with, as well as the queue name.

Aside from this new class the majority of code in this project was taken or          
adapted from the existing MSMQ SDK C_Draw and VB_Draw samples.


Requirements:

MSMQ 1.0 or later must be installed 


==========================================================================
				HOW TO USE
==========================================================================

NOTE: You can run multiple instances of this sample on one or more machines. 
The MSMQ Client must be installed on all machines used in the procedure below:

To see MSMQ functionality using Imp_Draw:

1. Run Imp_Draw.

2. Type your name in the login dialog.

   The name you specify is used to create a queue on your computer.

3. Run Imp_Draw on a second computer, specifying a different   
   name.
   Both computers must be in the same site.

4. Only on a DS enabled Computer:        
   Choose connection configuration (i.e. connecting with a DS enabled or a DS disabled computer).

   On a DS disabled Computer you cannot choose connection configuration. Only direct connection can be established.

   Follow the instruction according to the configuration you chose.

5. Standard mode:
   Type the name of the queue you wish to send messages to.

   Direct mode:
   Type the name of the queue and the name of the computer you wish to send messages to.  

6. Click the attach button.

Please note that you can’t connect between instances from different configuration.         
Both instances of Imp_Draw now have their own queue for receiving messages, and have opened the other 
instance's queue for send. You can either type text or draw on the form (by holding down the left mouse button).
As you type text or draw, each character or mouse movement is sent between the two instances of Imp_Draw.

You can close either instance of Imp_Draw and continue
sending messages. When Imp_Draw is restarted and you use the
same name, Imp_Draw receives all undelivered messages. If you
change the delivery mode from Express to Recoverable, you can close
one instance of Imp_Draw, restart the computer, restart
Imp_Draw, and receive messages that were sent while the
computer was rebooting.

Tip
To clear the Imp_Draw display drawing area, right-click
within the drawing area.

==========================================================================
				HOW IT WORKS
==========================================================================

	The main dialog is handled by the CImp_DrawDlg class, in the 
Imp_DrawDlg.cpp file.  It initializes the dialog, handles queue events, opens
the friend queue and sends mouse events and keystrokes to that friend queue.  

	When the dialog box is initialized, it prompts for a username, connects
to the local queue with that name, and registers to receive events from that
queue.  It will create the user's queue if necessary.  Events are handled 
via the CMSMQEventEvents class, which knows how to register for events if it's
given an event object.  That event object is then linked to the appropriate 
queue using the queue's EnableNotification method.

	Drawing within the draw area of the main dialog is handled by CDrawArea 
in the DRAWAREA.CPP file, which forwards keystroke and mouse events to the
main dialog so they can be sent to the friend queue if need be.

	When the main dialog's Attach button is pressed, the program will read
the name of the friend queue from the edit box and try to connect to that
queue to send mouse events and keystrokes.