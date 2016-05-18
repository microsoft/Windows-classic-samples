--------------------------------------------------------------------------

  Copyright (C) 1998-1999 Microsoft Corporation. All rights reserved.

--------------------------------------------------------------------------

TAPI 3.0
VBScript samples:
   1. VBscript callee/caller (callDemo.htm)
   2. IP Multicast conference participation (ConferenceJoin.htm)
   3. IP Multicast Conference management (ConfMgr.htm)

Overview:
   This directory contains 3 htm files, each of them is a sample that shows 
   you how to use VBScripting to access the TAPI 3.0 technology.

   See below a short description for each sample.

Hardware requirements:
   In general, you need a full duplex audio card to be installed on your machine.
   If you want to use the video features of these samples, you'll need a video capture card 
   and a camera as well.
   To make audio modem calls, you'll need a voice modem call (half or full duplex).
   To make Internet point to point calls (H323 calls) or to join Multicast conferences (IPConf calls),
   you need to have your machine part of a network, meaning that your machine should have 
   a valid IP address. If you want to join multicast conferences, multicast has to be enabled 
   on your network.
 

How to use:

To run any of this samples simply load it into IE (point your browser to the htm file 
that you are interested in). You need to use Win2000 with IE installed. 

To use samples 2 and 3 you need to have an ILS server configured in your network.

Sample 3 require presence of a MDHCP server as well in your network - it is needed 
for creating new conferences.


Description:


1. SAMPLE 1 - CallDemo.htm
   Vbscript callee/caller:	
   You could originate your call from any of the TAPI3 addresses available on your 
   computer: modem, H323 Line, etc. 

   Different addresses support different types of "target party" addresses:
   modems support only Phone Number, H323 Lines support Phone Number and
   IP Address and Machine Name etc.

   Fill in the target address and select its type (phone number, ip address, etc). 
   Choose the originating address and call properties: you can setup the call to be 
   audio only or with outgoing video.
   Press Connect to call your party.

   You can hang-up your call at any time by pressing Disconnect

   This sample registers itself on all available tapi3 addresses in order to receive
   incoming calls. You have the possibility to answer/reject the incoming calls.
   
What functionality this sample does show:
   How to create outgoing calls  with asynchronous connect procedure, how to setup
   required terminals. It also provides an example of address media modes determination and
   application priority setting; and it shows how to process incoming calls.

What this sample does not show:

   The sample does not handle correctly audio calls on half-duplex voice modem: 
   there is no possibility in this sample to alternate between "send" and "receive";
   this sample always to try to "send" and "receive" in the same time. So if you 
   use a full-duplex voice modem, everything will work fine. If you use a half-duplex 
   modem, one of the audio streams will fail and only one audio stream will work 
   (usually, the microphone (audio+capture stream) will succeed.

2. SAMPLE 2 - ConferenceJoin.htm
   IP Multicast conference participation:
   Enter the ILS server name and the name of a conference that is published on that ILS server; 
   then press Connect. You will be connected to this conference.

   Press hang-up at any time to leave the conference.

What functionality this sample does show:
   Rend.dll usage (connecting to ILS server, conference info extraction)
   Call management
   Media and terminal management (including preview window)
   Video substreams management

	
What this sample does not show:
   This page does not perform bind to the server, so you are unable to see a list of secured conferences.

Hints:
  1. If you need to associate a window with a particular participant then it's recommended to use 
      ITSubStream = ITParticipantSubStreamControl.SubStreamFromParticipant(ITParticipant)
  2. You could improve performance by processing only those tapi3 events that are meaningful for your app.
  3. Using ActiveX controls techonology could help you customize the windows placement 
   (requires HWND operations which are not possible from VBScripting)
  4. If you are planning to dynamically extend the number of VideoWindows allocated (this page always 
   allocates a fixed number of windows for simplicity): you could keep track of the number of substreams 
   allocated by participants.
   If there is no "free" unused substream it's a good idea to create a new VideoWindow, create a new substream 
   and then select this new window terminal on the substream. TAPI3 will do the rest for you. 
            
3. SAMPLE 3 - Confmgr.htm: 
   IP Multicast Conference management
	Enter the name of an ILS server that exists in your network or select one of 
        the ILS names preconfigured in your system.
	Press Connect and you will see a list of conferences published on this ILS server. You could
	add, delete conferences or modify properties of conferences. Some of these operations require 
	knowledge about the structure of conference description info - therefore use Advanced options 
	and SDP blob editor with caution!
	(You could end up with conferences that will not be handled correctly by some conferencing tools.)
	For additional info please consult section "IP Multicast Conferencing in TAPI 3.0"  in Platform SDK 
	documentation.

 This sample allows you to:
    Obtain a list of IP Multicast conferences published on a given ILS server 
    Create new IP Multicast conferences 
    Delete IP Multicast conferences 
    Directly edit properties of existing conferences using SDPblob editor

 What functionality this sample does show:
    rend.dll usage  for IP Multicast conference management
    MDHCP usage
    SDPBlob operations
    
What this sample does not show:
    You could not manage ACL's of the conferences. To do it use Phone Dialer 
    (Phone Dialer is shipped with Windows 2000 and you can find it in the Start Menu:
    Start - Programs - Accessories - Communications - PhoneDialer).



