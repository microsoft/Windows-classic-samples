

TAPI 2 Dialer.c Sample Application


Overview:
~~~~~~~~~

Dialer.c is a sample TAPI 2 application that illustrates some of the basic TAPI dialing 
features.  The aplication will: (1) initiates/drop calls, (2) handles simple
TAPI request for other applications initiating/dropping calls on their behalf, 
(3) monitor incoming/outgoing calls and keep a call log based on the user's request.

The application will read the speed dial button settings from the Dialer.ini file under the
[Speed Dial Settings] section. The settings for the last dialed numbers and window position 
are also stored in the ini file under the [Last dialed numbers] and the [Preference] 
sections. For more information, please see the Dialer.ini file also located in this directry. 


How to build the sample:
~~~~~~~~~~~~~~~~~~~~~~~~

To run the Dialer sample, set the SDK build environment, then
type "nmake" in the Dialer directory.  This will build dialer.exe.


How to use the sample:
~~~~~~~~~~~~~~~~~~~~~~
After the sample is built, run dialer.exe and a dailog box will appear
giving the user a choice to enter a phone number or dial a number
stored in the speed dial buttons. Menu options allow the user 
to choose among the devices currently installed on your computer to 
establish a dial up connection.  The complete functionality of the 
application will not be illustrated if there are no tapi devices 
installed on the computer.


What functionality does this sample show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The sample illustrates how to make a phone call using the assisted telephony
feature.  The tapi line functions to open  a line, translate 
addresses and make a call are also featured in this sample.  The sample also 
shows how to translate numbers from dialable format to cannonical format.  


What this sample does not show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This sample will not process incoming calls. It does not support 
machine name, email or Ip address type calls.


Additional Notes on how to build the sample: 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
How to build the samples with VC6 or VC5:
-	IMPORTANT: start a new command prompt (cmd.exe), or start a new 
	"Set Win32 Environment" cmd window from Programs - Microsoft Platform SDK .
-	go to the path where you installed VC6 
	(e.g. C:\ Program Files\Microsoft Visual Studio\VC98\Bin) and type VCVARS32.BAT.
-	go to the path where you installed SDK 
	(e.g. C:\Program Files\Microsoft Platform SDK) and type SetEnv.Bat.
-	check the following environment variables: 
	PATH, LIB, INCLUDE. You can see their current values by typing "SET" 
	at the command prompt. You should see that they contain first the SDK paths 
	and then the VC6 paths.
-	go to the path where the TAPI sample (TAPI2 and TAPI3) 
	is installed and type NMAKE. 

