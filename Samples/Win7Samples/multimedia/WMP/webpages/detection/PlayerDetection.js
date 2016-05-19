//    Copyright (c) Microsoft Corporation.  All rights reserved.

//-----------------------------------------------------------------
// PlayerDetection.js                                              
//                                                                 
// Purpose:                                                        
//   Javascript library of functions used by Sample.htm.  Each
//   function is commented as to its purpose.            
//-----------------------------------------------------------------


//------------------------
//-- Global Vars 
//------------------------
var WMPVer = "unknown";       // Set to WMP version string detected
var fHasWMP = false;          // True if either WMP v6.4 or v7+ found
var fHasWMP64 = false;        // True if WMP v6.4 found
var fHasWMP7 = false;         // True if WMP v7+ (8, 9) found

//-----------------------------------------------
// v7+ onPlayStateChange state options array
//-----------------------------------------------
var psArray = new Array(12);
psArray[0] = "Undefined - Windows Media Player is in an undefined state.";
psArray[1] = "Stopped - Playback of the current media clip is stopped."; 
psArray[2] = "Paused - Playback of the current media clip is paused. When media is paused, resuming playback begins from the same location.";
psArray[3] = "Playing - The current media clip is playing."; 
psArray[4] = "ScanForward - The current media clip is fast forwarding.";
psArray[5] = "ScanReverse - The current media clip is fast rewinding."; 
psArray[6] = "Buffering - The current media clip is getting additional data from the server.";
psArray[7] = "Waiting - Connection is established, however the server is not sending bits. Waiting for session to begin.";
psArray[8] = "MediaEnded - Media has completed playback and is at its end.";  
psArray[9] = "Transitioning - Preparing new media."; 
psArray[10] = "Ready - Ready to begin playing."; 
psArray[11] = "Reconnecting - Reconnecting to stream.";


//-----------------------------------------------
// v6.4 onPlayStateChange state options array
//-----------------------------------------------
var ps64Array = new Array(9);
ps64Array[0] = "mpStopped - Playback is stopped.";
ps64Array[1] = "mpPaused - Playback is paused."; 
ps64Array[2] = "mpPlaying - Stream is playing."; 
ps64Array[3] = "mpWaiting - Waiting for stream to begin."; 
ps64Array[4] = "mpScanForward - Stream is scanning forward."; 
ps64Array[5] = "mpScanReverse - Stream is scanning in reverse."; 
ps64Array[6] = "mpSkipForward - Skipping to next."; 
ps64Array[7] = "mpSkipReverse - Skipping to previous."; 
ps64Array[8] = "mpClosed - Stream is not open."; 


//-----------------------------------------------
// Name:  GetBrowser
// Purpose: Get Browser Information
//-----------------------------------------------
function GetBrowser()
{
    var agt=navigator.userAgent.toLowerCase();
    if( ((agt.indexOf("msie") != -1) && (agt.indexOf("opera") == -1)) )
        return "IE";
    else if( ((agt.indexOf('mozilla')!=-1) && (agt.indexOf('spoofer')==-1)
            && (agt.indexOf('compatible') == -1) && (agt.indexOf('opera')==-1)
            && (agt.indexOf('webtv')==-1) && (agt.indexOf('hotjava')==-1)) )
        return "Netscape";
    else
        return "unknown";
}


//------------------------------------------------
// Name:  GetPlayerMajorVer
// Purpose: Get Media Player Major Version Number
//------------------------------------------------
function GetPlayerMajorVer()
{
    var strVer = new String(WMPVer);
    s = strVer.split(".");
    return s[0];
}


//-------------------------------------------------
// Name:  GetPlayerMinorVer
// Purpose: Get Media Player Minor Version Number.
//-------------------------------------------------
function GetPlayerMinorVer()
{
    var strVer = new String(WMPVer);
    s = strVer.split(".");
    if (s[1])
        return s[1];
    else
        return("unknown");
}


//----------------------------------------------------------------------
// Name:  set_uiMode
// Purpose:  Wrapper function for setting the uiMode of the Media Player 
//  control.  Supports uiMode of "none", "mini" and "full".
//-----------------------------------------------------------------------
function set_uiMode(uiMode)
{ 
    // If WMP 6.4 then set ui mode related properties
    if (fHasWMP64) 
    { 
        if (uiMode=="none") 
        {
            MediaPlayer.ShowControls=false;
            MediaPlayer.ShowTracker=false;
            MediaPlayer.EnableTracker=false;
            MediaPlayer.ShowPositionControls=false;
            MediaPlayer.EnablePositionControls=false;
            MediaPlayer.ShowStatusBar=false;
        } 
        if (uiMode=="mini") 
        {
            MediaPlayer.ShowControls=true;
            MediaPlayer.ShowTracker=false;
            MediaPlayer.EnableTracker=false;
            MediaPlayer.ShowPositionControls=false;
            MediaPlayer.EnablePositionControls=false;
            MediaPlayer.ShowStatusBar=true;
        }  
        if (uiMode=="full") 
        {
            MediaPlayer.ShowControls=true;
            MediaPlayer.ShowTracker=true;
            MediaPlayer.EnableTracker=true;
            MediaPlayer.ShowPositionControls=true;
            MediaPlayer.EnablePositionControls=true;
            MediaPlayer.ShowStatusBar=true;
        }
    }
    else
    {
        MediaPlayer.uiMode=uiMode;
    }
}


//---------------------------------------------------------------------------
// Name: get_uiMode
// Purpose: Wrapper function for getting current uiMode of the Media Player.
//---------------------------------------------------------------------------
function get_uiMode()
{
    // Note: Per WMP SDK, uiMode is a string:  "none", "mini", "full"
    // If accessing the old 6.4 properties
    if (fHasWMP64) 
    {                
        if (MediaPlayer.ShowControls==false &&
            MediaPlayer.ShowTracker==false &&
            MediaPlayer.EnableTracker==false && 
            MediaPlayer.ShowPositionControls==false && 
            MediaPlayer.EnablePositionControls==false )
        return("none");

        if (MediaPlayer.ShowControls==true &&
            MediaPlayer.ShowTracker==false && 
            MediaPlayer.EnableTracker==false &&
            MediaPlayer.ShowPositionControls==false &&
            MediaPlayer.EnablePositionControls==false)
        return("mini");   

        if (MediaPlayer.ShowControls==true &&
            MediaPlayer.ShowTracker==true &&
            MediaPlayer.EnableTracker==true &&
            MediaPlayer.ShowPositionControls==true &&
            MediaPlayer.EnablePositionControls==true)
        return("full");   

        return("unknown");
    }
    else
    {
        return(MediaPlayer.uiMode);
    }
}


//------------------------------------------------------------------
// Name: ToggleMode
// Purpose:  Using get_uiMode and set_uiMode this  function cycles 
//           through the UI modes of the player control when called.
//-------------------------------------------------------------------
function ToggleMode()
{
   var Mode = get_uiMode();
   if (Mode=="none") set_uiMode("mini");
   if (Mode=="mini") set_uiMode("full");
   if (Mode=="full") set_uiMode("none");   
}


//----------------------------------------------------------------------
// Name: GetPlayerState
// Purpose:  Returns a string describing the NewState number specified.
//----------------------------------------------------------------------
function GetPlayerState(NewState)
{
    if (fHasWMP64)
    {
        if (NewState>=0 && NewState<=8)
            return ps64Array[NewState];
        else
            return "mpUndefined - Windows Media Player is in an undefined state.";
    }    
    else
    {
        if (NewState>=0 && NewState<=11)
            return psArray[NewState];
        else
            return "Undefined - Windows Media Player is in an undefined state.";
    }   
}

//---------------------------------------------------------------------
// Name:  AddDownloadLink
// Purpose:  Add WMP download link if an older version of WMP 
//           detected.
//---------------------------------------------------------------------
function AddDownloadLink(MajorVer)
{
    if (!(MajorVer) || MajorVer=="unknown" || MajorVer<9)
    { 
        document.write('<tr><td>You do not have the latest Windows Media Player version.<BR><A HREF="http://windowsmedia.com/download"><IMG SRC="http://www.microsoft.com/windows/windowsmedia/images/logos/getwm/mp11_88x31_static.gif" WIDTH="88" HEIGHT="31" BORDER="0" ALT="Get Windows Media Player" VSPACE="7"></A></td></tr>');
    } 
}

// SIG // Begin signature block
// SIG // MIIRUgYJKoZIhvcNAQcCoIIRQzCCET8CAQExCzAJBgUr
// SIG // DgMCGgUAMGcGCisGAQQBgjcCAQSgWTBXMDIGCisGAQQB
// SIG // gjcCAR4wJAIBAQQQEODJBs441BGiowAQS9NQkAIBAAIB
// SIG // AAIBAAIBAAIBADAhMAkGBSsOAwIaBQAEFCPRYtK96aVp
// SIG // iDZRvJD5HYPigNjcoIIO6jCCBBMwggNAoAMCAQICEGoL
// SIG // mU/AACKrEdsCQnwC074wCQYFKw4DAh0FADB1MSswKQYD
// SIG // VQQLEyJDb3B5cmlnaHQgKGMpIDE5OTkgTWljcm9zb2Z0
// SIG // IENvcnAuMR4wHAYDVQQLExVNaWNyb3NvZnQgQ29ycG9y
// SIG // YXRpb24xJjAkBgNVBAMTHU1pY3Jvc29mdCBUZXN0IFJv
// SIG // b3QgQXV0aG9yaXR5MB4XDTA2MDYyMjIyNTczMVoXDTEx
// SIG // MDYyMTA3MDAwMFowcTELMAkGA1UEBhMCVVMxEzARBgNV
// SIG // BAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1vbmQx
// SIG // HjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEb
// SIG // MBkGA1UEAxMSTWljcm9zb2Z0IFRlc3QgUENBMIIBIjAN
// SIG // BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAj/Pz33qn
// SIG // cihhfpDzgWdPPEKAs8NyTe9/EGW4StfGTaxnm6+j/cTt
// SIG // fDRsVXNecQkcoKI69WVT1NzP8zOjWjMsV81IIbelJDAx
// SIG // UzWp2tnbdH9MLnhnzdvJ7bGPt67/eW+sIwZUDiNDN3jd
// SIG // Pk4KbdAq9sZ+W5J0DbMTD1yxcbQQ/LEgCAgueW5f0nI0
// SIG // rpI6gbAyrM5DWTCmwfyu+MzofYZrXK7r3pX6Kjl1BlxB
// SIG // OlHcVzVOksssnXuk3Jrp/iGcYR87pEx/UrGFOWR9kYlv
// SIG // nhRCs7yi2moXhyTmG9V8fY+q3ALJoV7d/YEqnybDNkHT
// SIG // z/xzDRx0KDjypQrF0Q+7077QkwIDAQABo4HrMIHoMIGo
// SIG // BgNVHQEEgaAwgZ2AEMBjRdejAX15xXp6XyjbQ9ahdzB1
// SIG // MSswKQYDVQQLEyJDb3B5cmlnaHQgKGMpIDE5OTkgTWlj
// SIG // cm9zb2Z0IENvcnAuMR4wHAYDVQQLExVNaWNyb3NvZnQg
// SIG // Q29ycG9yYXRpb24xJjAkBgNVBAMTHU1pY3Jvc29mdCBU
// SIG // ZXN0IFJvb3QgQXV0aG9yaXR5ghBf6k/S8h1DELboVD7Y
// SIG // lSYYMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFFSl
// SIG // IUygrm+cYE4Pzt1G1ddh1hesMAsGA1UdDwQEAwIBhjAJ
// SIG // BgUrDgMCHQUAA4HBACzODwWw7h9lGeKjJ7yc936jJard
// SIG // LMfrxQKBMZfJTb9MWDDIJ9WniM6epQ7vmTWM9Q4cLMy2
// SIG // kMGgdc3mffQLETF6g/v+aEzFG5tUqingK125JFP57MGc
// SIG // JYMlQGO3KUIcedPC8cyj+oYwi6tbSpDLRCCQ7MAFS15r
// SIG // 4Dnxn783pZ5nSXh1o+NrSz5mbGusDIj0ujHBCqblI96+
// SIG // Rk7oVQ2DI3oQkSmGQf+BrmRXoJfB3YuXXFc+F88beLHS
// SIG // F0S8oJhPjzCCBPowggPioAMCAQICCmEHsOIAAAAAAAkw
// SIG // DQYJKoZIhvcNAQEFBQAwcTELMAkGA1UEBhMCVVMxEzAR
// SIG // BgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1v
// SIG // bmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlv
// SIG // bjEbMBkGA1UEAxMSTWljcm9zb2Z0IFRlc3QgUENBMB4X
// SIG // DTA3MTAwOTIxMDcyM1oXDTA5MDQwOTIxMTcyM1owgawx
// SIG // CzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9u
// SIG // MRAwDgYDVQQHEwdSZWRtb25kMRIwEAYDVQQKEwlNaWNy
// SIG // b3NvZnQxGzAZBgNVBAsTEkNvcnBvcmF0ZSBTZWN1cml0
// SIG // eTEiMCAGA1UEAxMZTWljcm9zb2Z0IFdpbmRvd3MgUmVk
// SIG // bW9uZDEhMB8GCSqGSIb3DQEJARYScGtpdEBtaWNyb3Nv
// SIG // ZnQuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB
// SIG // CgKCAQEAtkUpDJ1Ml+d3dn3Q8Z7hP6oFCKG01lYP9UAx
// SIG // K/JFotT5rufRy/tLFI8nrekfRphcQ+bls3i3gmvgpPbL
// SIG // toqG1/uONsBIR2QbNawzPit1zCepteN5WADYfnuV6W67
// SIG // y7gzqNbvXHbhuSRxqV5J7xoJvfZA4CfpKfpAAgBXPJMy
// SIG // hr9xPVNtWBB4xU/qcu6pXO/vcD6+4VrL+U/zuK08wpCV
// SIG // VR5Rgei1nIq6F6AO//d37kNzfW7dVoD0ECQ4tnHl6f6H
// SIG // D8N+gFUSpayV15Grn21H8xH7wv7Zk3afuPS3ItsxxU+S
// SIG // KtrT9gfKI5GqldHW+tItxQIX6hdfSIjB/idNIkhegQID
// SIG // AQABo4IBVjCCAVIwDwYDVR0TAQH/BAUwAwEB/zAdBgNV
// SIG // HQ4EFgQUU5QbMWKAaw9CxJmgNlrtApN0dk0wCwYDVR0P
// SIG // BAQDAgGGMBIGCSsGAQQBgjcVAQQFAgMFAAUwIwYJKwYB
// SIG // BAGCNxUCBBYEFJ8H6Woi8ML7A3nxeuG76UTQ5uvuMBkG
// SIG // CSsGAQQBgjcUAgQMHgoAUwB1AGIAQwBBMB8GA1UdIwQY
// SIG // MBaAFFSlIUygrm+cYE4Pzt1G1ddh1hesMEwGA1UdHwRF
// SIG // MEMwQaA/oD2GO2h0dHA6Ly9jcmwubWljcm9zb2Z0LmNv
// SIG // bS9wa2kvY3JsL3Byb2R1Y3RzL2xlZ2FjeXRlc3RwY2Eu
// SIG // Y3JsMFAGCCsGAQUFBwEBBEQwQjBABggrBgEFBQcwAoY0
// SIG // aHR0cDovL3d3dy5taWNyb3NvZnQuY29tL3BraS9jZXJ0
// SIG // cy9MZWdhY3lUZXN0UENBLmNydDANBgkqhkiG9w0BAQUF
// SIG // AAOCAQEASZvKu6ib0s+TYu9xuJovCkfeCDUKDWSSR8Fk
// SIG // pBDpG3ssy4zzRiKqY/V2KFEZ8bhxu0qExdyGZw/lqbxA
// SIG // IikNgZRKNM3q73fmT2ZXuJHv2WjQ1yy06pukP0A2/nv3
// SIG // hQ03OFku1vGgPmZX1bC5YhoqCi4RfztYKCnuO9Ne7d1n
// SIG // rjMUjBudadPGMVnDBKjKLkW8WsyYyrMM0xntgXLVJ/lC
// SIG // lOEMMhJGdfNtUMHcif4T4zLQAF2SKhk9/5J8goip2A3E
// SIG // u6OTuCtsMu7zoQ252OGCjQhwDY7YnkF+7pBHXuLamOxr
// SIG // CeaxrG/2uy2wmZRG9KStT+IFAjioLSmAifxj69w4hTCC
// SIG // BdEwggS5oAMCAQICCixQSHoABQAUqsswDQYJKoZIhvcN
// SIG // AQEFBQAwgawxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpX
// SIG // YXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMRIwEAYD
// SIG // VQQKEwlNaWNyb3NvZnQxGzAZBgNVBAsTEkNvcnBvcmF0
// SIG // ZSBTZWN1cml0eTEiMCAGA1UEAxMZTWljcm9zb2Z0IFdp
// SIG // bmRvd3MgUmVkbW9uZDEhMB8GCSqGSIb3DQEJARYScGtp
// SIG // dEBtaWNyb3NvZnQuY29tMB4XDTA4MTIyMDAyMjAxMFoX
// SIG // DTA5MDQwOTIxMTcyM1owFTETMBEGA1UEAxMKVlMgQmxk
// SIG // IExhYjCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA
// SIG // wvUHQg/XAjRhvR6CIO0Ztig7RfdTF5LeF4lO/kS+csNZ
// SIG // v/pQuijwYneJIypVRcqHMUAY9E5akdp7frRz5jhuKFUp
// SIG // kgRg0o14qc8/CE6yJNEvWUrLne0ZbDci/WMKxmRkb9NE
// SIG // eYpjQOci6qLtmc5qGVD/sHjiw0WUHVZAE5aiM1ECAwEA
// SIG // AaOCAw0wggMJMAsGA1UdDwQEAwIHgDAdBgNVHQ4EFgQU
// SIG // ZuosvMhERz9xwjwvXxVlG6zaIlAwPQYJKwYBBAGCNxUH
// SIG // BDAwLgYmKwYBBAGCNxUI3reIc4T9oIR5h+2VvQyM/9H6
// SIG // OI+bw7xiiq3xvicCAWQCAQgwHwYDVR0jBBgwFoAUU5Qb
// SIG // MWKAaw9CxJmgNlrtApN0dk0wggEiBgNVHR8EggEZMIIB
// SIG // FTCCARGgggENoIIBCYaBzWxkYXA6Ly8vQ049TWljcm9z
// SIG // b2Z0JTIwV2luZG93cyUyMFJlZG1vbmQoNSksQ049Q1JM
// SIG // LENOPUNEUCxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNl
// SIG // cyxDTj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERD
// SIG // PWNvcnAsREM9bWljcm9zb2Z0LERDPWNvbT9jZXJ0aWZp
// SIG // Y2F0ZVJldm9jYXRpb25MaXN0P2Jhc2U/b2JqZWN0Q2xh
// SIG // c3M9Y1JMRGlzdHJpYnV0aW9uUG9pbnSGN2h0dHA6Ly9j
// SIG // b3JwcGtpL2NybC9NaWNyb3NvZnQlMjBXaW5kb3dzJTIw
// SIG // UmVkbW9uZCg1KS5jcmwwgcwGCCsGAQUFBwEBBIG/MIG8
// SIG // MHUGCCsGAQUFBzAChmlodHRwOi8vcmVkaXRnY2FiMDgv
// SIG // Q2VydEVucm9sbC9SRURJVEdDQUIwOC5yZWRtb25kLmNv
// SIG // cnAubWljcm9zb2Z0LmNvbV9NaWNyb3NvZnQlMjBXaW5k
// SIG // b3dzJTIwUmVkbW9uZCg1KS5jcnQwQwYIKwYBBQUHMAKG
// SIG // N2h0dHA6Ly9jb3JwcGtpL2FpYS9NaWNyb3NvZnQlMjBX
// SIG // aW5kb3dzJTIwUmVkbW9uZCg1KS5jcnQwHwYDVR0lBBgw
// SIG // FgYKKwYBBAGCNwoDBgYIKwYBBQUHAwMwKQYJKwYBBAGC
// SIG // NxUKBBwwGjAMBgorBgEEAYI3CgMGMAoGCCsGAQUFBwMD
// SIG // MDoGA1UdEQQzMDGgLwYKKwYBBAGCNxQCA6AhDB9kbGFi
// SIG // QHJlZG1vbmQuY29ycC5taWNyb3NvZnQuY29tMA0GCSqG
// SIG // SIb3DQEBBQUAA4IBAQAangKN+4Z8zmfZ/mLV1D9Bmg5W
// SIG // 4CsPvl59l9QH1x6qjhsz/vYWzAz5r08ilVm7YjT1s169
// SIG // rERUPKRcY8lW2k20baC9Z3gTvo0zW/cgvF6ZTRlCjL2h
// SIG // 66T4ojJkJIWOXsEnE5d3Buqan/gnJqL+BWLbEA2b8HeZ
// SIG // cHyM8k/mPrwFpB1m6DuGAyP+mvWal0tfuuGj5ScWYCIs
// SIG // hQ2oBtF6yxtto/rLp0BEsGrgdVVkuD+aSd9mOUeAoE+j
// SIG // ON9dKOQuAi7Gbmxp2SP4lZodAqa1uPsXGjSy3mhguyiA
// SIG // KNai136dyVUa7Vk4qYehdJD6Q/In29uNTW90sMJT+DRp
// SIG // AiclA9P/MYIB1DCCAdACAQEwgbswgawxCzAJBgNVBAYT
// SIG // AlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9uMRAwDgYDVQQH
// SIG // EwdSZWRtb25kMRIwEAYDVQQKEwlNaWNyb3NvZnQxGzAZ
// SIG // BgNVBAsTEkNvcnBvcmF0ZSBTZWN1cml0eTEiMCAGA1UE
// SIG // AxMZTWljcm9zb2Z0IFdpbmRvd3MgUmVkbW9uZDEhMB8G
// SIG // CSqGSIb3DQEJARYScGtpdEBtaWNyb3NvZnQuY29tAgos
// SIG // UEh6AAUAFKrLMAkGBSsOAwIaBQCgcDAQBgorBgEEAYI3
// SIG // AgEMMQIwADAZBgkqhkiG9w0BCQMxDAYKKwYBBAGCNwIB
// SIG // BDAcBgorBgEEAYI3AgELMQ4wDAYKKwYBBAGCNwIBFTAj
// SIG // BgkqhkiG9w0BCQQxFgQUiB1/W92rdstS+czS8R1XKef6
// SIG // 2bwwDQYJKoZIhvcNAQEBBQAEgYBgfPI37l/5M46TAWQr
// SIG // 2ClFmDIgU59KwhSo+P7JTFQZAxgFFW4Hzby6XbRXyNQy
// SIG // fzv6Zf0qOZp4K221fupRGwFKEpgju3nFIHzPI+xtdH9d
// SIG // m+pQZ9PzfDBKXZiC5OSd+408oTrFHep1MmLBwhTynaB/
// SIG // EC1FaaCT6ftMcwErr93L2w==
// SIG // End signature block
