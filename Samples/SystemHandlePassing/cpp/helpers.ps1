param([switch] $register, [switch] $clean, [System.IO.FileInfo]$path)

function registerSparkle($proxypath, $serverpath){
    Write-Host "Registering Sparkle Finisher Proxy and Server..."
    Write-Host "Elevation is required and will prompt twice if necessary."
    
    Write-Host "Self-Register Proxy with regsvr32.exe"
    Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s",$proxypath -Verb RunAs -Wait

    Write-Host "Import class and interface definitions for Server with reg.exe"
    Start-Process -FilePath "reg.exe" -ArgumentList "ADD","HKCR\CLSID\{EA27C73A-48C2-4714-9D20-A9D2C4F6AED3}\LocalServer32","/ve","/t","REG_SZ","/d",$serverpath -Verb RunAs -Wait   
}

function cleanSparkle($proxypath, $serverpath){
    Write-Host "Cleaning up Sparkle Finisher registrations..."
    Write-Host "Elevation is required and will prompt twice if necessary."

    Write-Host "Self-Unregister Proxy with regsvr32.exe"
    Start-Process -FilePath "regsvr32.exe" -ArgumentList "/s","/u",$proxypath -Verb RunAs -Wait

    Write-Host "Delete class and interface definitions for Server with reg.exe"
    Start-Process -FilePath "reg.exe" -ArgumentList "delete","HKCR\CLSID\{EA27C73A-48C2-4714-9D20-A9D2C4F6AED3}","/f" -Verb RunAs -Wait

    Write-Host "Done!"
}

function help()
{
    Write-Host "Use `-register` to register the sparkle server and proxy. Use `-clean` to clean up registrations."
    Write-Host "Use `-path` to pass the output path to the generated binaries from building the project. (Example: x64\release if building from the solution directory)"
}

# Must choose one or the other flag and provide a path
if (($register -xor $clean) -and $path)
{
    # Quick check the path for existance
    if (-Not (Test-Path -Path $path)){
        Write-Host "Folder does not exist."
        help
    }
    # Quick check it's a folder, not a file.
    elseif (-Not (Test-Path -Path $path -PathType Container)){
        Write-Host "Path must be the folder where binaries were built."
        help
    }
    else {
        # Set current working directory into System.IO or it might get confused
        [System.IO.Directory]::SetCurrentDirectory($pwd)

        # Canonicalize path and append proxy/server binary names
        $proxystub = [System.IO.Path]::GetFullPath((Join-Path -Path $path -ChildPath "ProxyStub.dll"))
        $serverexe = [System.IO.Path]::GetFullPath((Join-Path -Path $path -ChildPath "Server.exe"))

        # Check if they exist
        if (-Not(Test-Path -Path $proxystub)) {
            Write-Host "Cannot find ProxyStub.dll. Did you build it or give an incorrect output path?"
            help
        }
        elseif (-Not(Test-Path -Path $serverexe)) {
            Write-Host "Cannot find Server.exe. Did you build it or give an incorrect output path?"
            help
        }
        else {
            # Call Register or Clean
            if ($register){
                registerSparkle $proxystub $serverexe
            }
            else{
                cleanSparkle $proxystub $serverexe
            }
        }
    }   
}
else
{
    help
}
