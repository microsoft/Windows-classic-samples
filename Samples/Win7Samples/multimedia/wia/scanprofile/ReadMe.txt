========================================================================
   SAMPLE : ScanProfile
========================================================================

Description:
-------------
Use the new IScanProfile and IScanProfileMgr interfaces exposed by WIA. 
These 2 interfaces help in managing Scan profiles.

Scan Profiles provide the user a means to save preferred scan settings for future use. Scan Profiles are linked to a specific 
Windows User Account and scan device. Each Windows User Account can have multiple Scan Profiles for each scan device. Scan Profiles 
are saved as XML files for each user under that users My Scanned Documents folder which has a sub-folder called Scan Profiles. 
Scan profiles essentially consist of values for the scan setting options available on the Scan Settings Dialog.

Details:
--------
The sample provides both programmatic control over Scan Profiles and through a GUI (Scan Profile dialog).

The sample creates an instance of IScanProfileMgr. This interface can be used to get IScanProfile. Using these 2 interfaces,
it is easy to programmatically do various operations on Scan Profiles. 

After that the sample uses the API IScanProfileUI::ScanProfileDialog() to launch the Scan Dialog.

The sample uses the following API's:

1)The sample calls IScanProfileMgr::CreateProfile() to create a new scan profile.
2)The sample calls IScanProfileMgr::GetProfilesForDeviceID() to return the IScanProfile interfaces for all available profiles 
for a specific deviceID in the system.
3)The sample calls IScanProfile::GetGUID() to get the GUID of the profile. Each profile has an automatically created GUID in it, which uniquely 
identifies it.
4)The sample calls IScanProfile::GetdeviceID() to get the DeviceID of the device, with which the profile is associated. 
5)The sample calls IScanProfile::GetName() and IScanProfile::SetName() to get and set the name of the profile respectively. 
6)The sample calls IScanProfileMgr::GetNumProfilesforDeviceID() to get the nunber of profiles for a specific device ID. 
7)The sample calls IScanProfile::GetProperty() to return the values of the list of properties in a profile.
8)The sample calls IScanProfile::SetProperty() to set properties in a profile.
9)The sample calls IScanProfileMgr::SetDefault() to set the current profile
as the default. Hence in push scanning this profile will be used.
10) The sample calls IScanProfileUI::ScanProfileDialog() to launch the Scan Dialog.

Note: When the sample is run , after some messages on the console containing
information about scan profiles , immediately Scan Dialog pops up for the user
to change the profile settings.

PreCondition:
-------------------
This sample is supported for Windows Vista.
For this sample to produce some result, atleast one WIA device should be installed on the system.




