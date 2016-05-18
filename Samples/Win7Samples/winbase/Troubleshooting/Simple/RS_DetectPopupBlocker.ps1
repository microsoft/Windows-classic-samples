# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation. All rights reserved

Write-DiagProgress -activity "Enabling Pop-up blocker..."

$PopupMgr = Get-ItemProperty "Registry::HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\New Windows" "PopupMgr"

if($PopupMgr.PopupMgr -eq "no") { 
    Set-ItemProperty -Path "Registry::HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\New Windows" -Name PopupMgr -Value "yes"
}

if($PopupMgr.PopupMgr -eq "0") {
    Set-ItemProperty -Path "Registry::HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\New Windows" -Name PopupMgr -Value "1"
}