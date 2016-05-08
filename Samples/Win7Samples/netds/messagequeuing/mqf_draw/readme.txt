------------------------------------------------------------------------------

	Copyright (C) Microsoft Corporation. All rights reserved.

------------------------------------------------------------------------------

Sample: mqf_DRAW

Purpose: 
This application is a port of the "standard" multi_dest_draw MSMQ sample application to C using
the MSMQ SDK.  
The application basically allows the user to send/receive line drawings to/from other draw 
applications.
Note that the other implementation of this "line drawing" protocol in VB inter-operates with
this sample.

A private text based format is used to encode line-drawing information. 

Requirements:

MSMQ 3.0 or later must be installed 


Overview:
When a mqf_DRAW application is started, the user is prompted to specify his "login" name -- 
this can be any string and is used to create a local queue by that name.

If the user is working on a DS enabled computer, he will be asked to choose whether he 
would like to open a public or a private local receiving queue.  
After creating and opening the local queue an asynchronous message handler is invoked to
monitor that queue: arriving messages are interpreted as line drawings and displayed on the
form. Than the user can choose the remote queues he wants to be connected too. The queues 
can be either public or private. On the public case the user is asked to enter a remote 
queue label. On the private case the user is asked to enter a remote queue name and the remote
computer name. Clicking on the "Add Queue" button adds the queue to the queue's list. 

If the user is working on a DS disabled computer, local private queue will be used 
for receiving messages. And the user will be able to connect to private queues only.

After clicking the "Attach" button everything drawn will be sent to the remote queues.
The picture on the form allows the user to draw lines on its client area by dragging and
clicking the left mouse button. Clicking the right button erase the screen.
These mouse movements are captured and translated into a series of line coordinates.
The coordinates are echoed on the form using standard C Line commands and also sent to the
added queues. Likewise text can be entered.


 


