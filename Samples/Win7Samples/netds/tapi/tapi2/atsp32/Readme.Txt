
TAPI 2 atsp.c Sample Application


Overview:
~~~~~~~~~
Atsp.c is a sample TAPI application that shows how to write a 
telephony service provider (TSP) for a modem.  TSP's are used 
by the TAPI applications to interface with telephony devices. Call control 
is established in the tapi application through the TSP interface (TSPI).  
This sample will not run by itself, but can  be used with a full TAPI 
application. The functionality can be tested in the TAPI TB20 or TB3X 
browser. For information on the TAPI browser, see the tools section of
the Platform SDK (PSDK). 

This sample illustrates the basic TSPI required to initiate outgoing modem 
and interactive voice calls. 


How to build the sample:
~~~~~~~~~~~~~~~~~~~~~~~~
To run the atsp sample, set the SDK build environment, then
type "nmake" in the atsp directory.  This will build atsp32.tsp.


How to use the sample:
~~~~~~~~~~~~~~~~~~~~~~
After the sample is built, the TAPI browser can be used to test the TSP.
See documentation for the browser in the Tools section of the PSDK.


What functionality does this sample show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This sample illustrates many of the TSPI line functions such 
as TSPI_LineClose,  TSPI_LineGetCallInfo, TSPI_LineGetDevCaps and 
TSPI_LineMakeCall.  The TSPI  provider functions are also shown for the 
TSPI_providerInit, TSPI_providerEnumDevices, TSPI_providerInstall, and 
theTSPI_providerConfig functions.  The lines configuration and the 
devices configured are retrieved from the register in this example.   

In Addition this TSP is linked to the Sample MSP (SampMsp) in the 
Tapi3 sample directory tree. In ATSP is the implementation of 
TSPI_lineReceiveMSPData, TSPI_lineCreateMSPInstance, 
TSPI_lineCloseMSPInstance and TSPI_lineMSPIdentify



Additional Notes on how to build the sample: 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
How to build the samples with VC6 or VC5:
-	IMPORTANT: start a new command prompt (cmd.exe), or start 
	a new "Set Win32 Environment" cmd window from Programs – Microsoft 
	Platform SDK .
-	go to the path where you installed VC6 
	(e.g. C:\ Program Files\Microsoft Visual Studio\VC98\Bin) 
	and type VCVARS32.BAT.
-	go to the path where you installed SDK 
	(e.g. C:\Program Files\Microsoft Platform SDK) and type SetEnv.Bat.
-	check the following environment variables: 
	PATH, LIB, INCLUDE. You can see their current values by typing "SET" 
	at the command prompt. You should see that they contain first the 
	SDK paths and then the VC6 paths.
-	go to the path where the TAPI sample (TAPI2 and TAPI3) 
	is installed and type NMAKE. 

