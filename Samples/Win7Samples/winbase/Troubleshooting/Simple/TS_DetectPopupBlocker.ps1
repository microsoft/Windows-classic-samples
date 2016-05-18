# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation. All rights reserved

#Write a progress message to the user
Write-DiagProgress -Activity "Checking Pop-up Blocker..."

#Get the pop-up blocker settings from the registry
$PopupMgr = Get-ItemProperty "Registry::HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\New Windows" "PopupMgr"

[bool]$Detected = ($PopupMgr.PopupMgr -eq "no") -or ($PopupMgr.PopupMgr -eq "0")
   
Update-DiagRootcause -Id DetectPopupBlocker -Detected $Detected    
