####################################################################
# This implements a Virtual Machine System. This uses a XML based store.
# In the system, there are multiple physical machines. Each physical 
# machine can have multiple virutal machines in it. There are multiple
# systems (like domain)
####################################################################
Add-Type @'
using System.Collections.Generic;
using System;

public class VM
{
    public string MachineName {get; set; }
    public Guid Id {get; set; }
    public string OS {get; set; }

    public object PhysicalMachine {get; set; }
    public object System {get; set; }
}

public class VmSystem
{
    public VmSystem()
    {
        VMs = new List<VM>();
    }

    public string Name {get; set; }    
    public Guid Id {get; set; }  
    
    public List<VM> VMs {get; private set; }
}

public class VmPhysicalMachine
{
    public VmPhysicalMachine()
    {
        VMs = new List<VM>();
    }

    public string Name {get; set; }
    public List<VM> VMs {get; private set; }
}
'@

$PhysicalMachines = new-object System.Collections.ArrayList
$Systems = new-object System.Collections.ArrayList

# Update this path, if HostVMSystem file is moved to some other folder
$SystemFileName =  $Env:SystemDrive + "\inetpub\wwwroot\Modata\HostVMSystem.xml"

Function LoadSystemFile($systemFileName)
{
    [System.Xml.XmlDocument] $doc = new-object System.Xml.XmlDocument
    $doc.load($systemFileName)

    $systemsNode = $doc.SelectNodes("/VMSystem/Systems/System")

    foreach ($systemNode in $systemsNode)
    {
        $system = new-object VmSystem
        $system.Name = $systemNode.SelectSingleNode("Name").get_InnerXml()
        $system.Id = [System.Guid]::Parse($systemNode.SelectSingleNode("Id").get_InnerXml())

        [void]$Systems.Add($system)
    }

    $physicalMachineNodes = $doc.SelectNodes("/VMSystem/PhysicalMachines/PhysicalMachine")

    foreach ($physicalMachineNode in $physicalMachineNodes)
    {
        $physicalMachine = new-object VmPhysicalMachine
        $physicalMachine.Name = $physicalMachineNode.SelectSingleNode("Name").get_InnerXml()

        $vmNodes = $physicalMachineNode.SelectNodes("VMs/Vm");
        foreach ($vmNode in $vmNodes)
        {
            $vm = new-object VM

            $vm.Id = [System.Guid]::Parse($vmNode.SelectSingleNode("VmId").get_InnerXml())
            $vm.OS = $vmNode.SelectSingleNode("OperatingSystem").get_InnerXml()
            $vm.MachineName = $physicalMachine.Name

            $systemIdNode = $vmNode.SelectSingleNode("SystemId");
            if ($systemIdNode -ne $null)
            {
                $system = GetSystem([System.Guid]::Parse($systemIdNode.get_InnerXml()));

                $vm.System = $system
                if ($system -ne $null)
                {
                    $system.VMs.Add($vm)
                }
            }

            $vm.PhysicalMachine = $physicalMachine

            $physicalMachine.VMs.Add($vm);
        }

        [void]$PhysicalMachines.Add($physicalMachine)
    }
}

Function WriteToSystemFile([string]$systemFileName)
{
    $settings = New-Object system.Xml.XmlWriterSettings  
    $settings.Indent = $true  
    $settings.OmitXmlDeclaration = $false  
    $settings.NewLineOnAttributes = $true  

    $writer = [system.xml.XmlWriter]::Create($systemFileName, $settings)

    $writer.WriteStartElement("VMSystem")

    $writer.WriteStartElement("PhysicalMachines")
    
    foreach ($physicalMachine in $PhysicalMachines)
    {
        $writer.WriteStartElement("PhysicalMachine")
        $writer.WriteElementString("Name", $physicalMachine.Name)

        $writer.WriteStartElement("VMs")
        
        foreach ($vm in $physicalMachine.VMs)
        {
            $writer.WriteStartElement("Vm")

            $writer.WriteElementString("VmId", $vm.Id)
            $writer.WriteElementString("OperatingSystem", $vm.OS)
            if ($vm.System -ne $null)
            {
                $writer.WriteElementString("SystemId", $vm.System.Id)
            }
          
            $writer.WriteEndElement()
        }
        
        $writer.WriteEndElement()

        # PhysicalMachine
        $writer.WriteEndElement()
    }

    # PhysicalMachines
    $writer.WriteEndElement()

    $writer.WriteStartElement("Systems")

    foreach ($system in $Systems)
    {
        $writer.WriteStartElement("System")
        $writer.WriteElementString("Name", $system.Name)
        $writer.WriteElementString("Id", $system.Id)

        # System
        $writer.WriteEndElement()
    }

    # Systems
    $writer.WriteEndElement()
    
    # VMSystem
    $writer.WriteEndElement()

    $writer.Flush()
    $writer.Close()
}

Function Init()
{
    if (($PhysicalMachines.Count -eq 0) -or ($Systems.Count -eq 0))
    {
        LoadSystemFile $SystemFileName
    }
}

Function ClearSystem()
{
    $PhysicalMachines.Clear()
    $Systems.Clear()
}

Function GetSystem($systemId)
{
    foreach ($system in $Systems)
    {
        if ($systemId -eq $system.Id)        
        {
            return $system;
        }
    }

    return $null;
}

##############################################
# Get-PhysicalMachine cmdlet implementation
##############################################
Function Get-PhysicalMachine($machineName)
{
    Init

    if ($machineName -eq $null)
    {
        return $PhysicalMachines;
    }
    else
    {
        foreach ($physicalMachine in $PhysicalMachines)
        {
            if ($physicalMachine.Name -eq $machineName)
            {
                return $physicalMachine;
            }
        }

        return $null;
    }
}

##############################################
# Get-System cmdlet implementation 
##############################################
Function Get-System($systemName, $systemId)
{
    Init

    if ($systemId -ne $null)
    {
        $systemId = [System.Guid]::Parse($systemId)
    }

    if (($systemName -eq $null) -and ($systemId -eq $null))
    {
        return $Systems;
    }
    else
    {
        foreach ($system in $Systems)
        {
            if (($system.Name -eq $systemName) -and ($system.Id -eq $systemId))
            {
                return $system;
            }
        }

        return $null;
    }
}

##############################################
# Get-VM cmdlet implementation
##############################################
Function Get-VM($machineName, $vmId)
{
    Init

    if ($vmId -ne $null)
    {
        $vmId = [System.Guid]::Parse($vmId)
    }

    if (($machineName -eq $null) -and ($vmId -eq $null))
    {
        $allVms = new-object System.Collections.ArrayList

        foreach ($physicalMachine in $PhysicalMachines)
        {
            foreach ($vm in $physicalMachine.VMs)
            {
                [void]$allVms.Add($vm);
            }
        }

        return $allVms;
    }
    else
    {
        $physicalMachine = Get-PhysicalMachine $machineName
        if ($physicalMachine -ne $null)
        {
            foreach ($vm in $physicalMachine.VMs)
            {
                if ($vm.Id -eq $vmId)
                {
                    return $vm;
                }
            }
        }

        return $null;
    }
}

##############################################
# New-VM cmdlet implementation. 
##############################################
Function New-VM($machineName, $OS)
{
    $machine = Get-PhysicalMachine $machineName
    if ($machine -eq $null)
    {
        write-error("Cannot find physical machine " + $machineName)
        return $null
    }

    $vm = new-object VM
    $vm.OS = $OS
    $vm.MachineName = $machineName
    $vm.Id = [System.Guid]::NewGuid()

    $vm.PhysicalMachine = $machine

    [void]$machine.VMs.Add($vm)
    WriteToSystemFile $SystemFileName

    return $vm
}

##############################################
# Remove-VM cmdlet implementation
##############################################
Function Remove-VM($machineName, $vmId)
{
    $physicalMachine = Get-PhysicalMachine $machineName
    if ($physicalMachine -eq $null)
    {
       write-error("Cannot find physical machine " + $machineName)        
       return $null
    }

    $removedVm = $null
    foreach ($vm in $physicalMachine.VMs)
    {
        if ($vm.Id -eq $vmId)
        {
            [void]$physicalMachine.VMs.Remove($vm)
            
            $removedVm = $vm
            break
        }
    }

    foreach ($system in $Systems)
    {
        if ($system.VMs.Contains($removedVm))
        {
            [void]$system.VMs.Remove($removedVm)
        }
    }

    WriteToSystemFile $SystemFileName

    return $removedVm
}

##############################################
# Set-VM cmdlet implementation
# Changes properties of a VM
##############################################
Function Set-VM($machineName, $vmId, $OS)
{
    $vm = Get-VM $machineName $vmId
    if ($vm -eq $null)
    {
        write-error("Cannot find vm (id = " + $vmId + ", Machine name " + $machineName + ")")        
        return $null
    }
    
    if ($OS -ne $null)
    {
        $vm.OS = $OS
    }

    return $vm
}

##############################################
# Get-VMForSystem cmdlet implementation
# Gets collection of virtual machines for a system
##############################################
Function Get-VMForSystem($systemName, $systemId)
{
    Init 

    if (($systemId -eq $null) -or ($systemName -eq $null))
    {
        write-error("Cannot accept null values for systemName and systemId");
        return $null;
    }

    $system  = Get-System $systemName $systemId
    if ($system -eq $null)
    {
        write-error("Cannot find system (systemName " + $systemName + ", systemId " + $systemId + ")");
        return $null
    }

    return $system.VMs;
}

##############################################
# Add-VMToSystem cmdlet implementation
# Adds a virtual machine to a system
##############################################
Function Add-VMToSystem($systemName, $systemId, $machineName, $vmId)
{
    Init

    if (($systemId -eq $null) -or ($systemName -eq $null) -or ($machineName -eq $null) -or ($vmId -eq $null))
    {
        write-error("Null input values passed ");
	return
    }

    $system  = Get-System $systemName $systemId
    if ($system -eq $null)
    {
        write-error("Cannot find System for system name " + $systemName + " and system id " + $systemId);
        return
    }

    $vm  = Get-VM $machineName $vmId
    if ($vm -eq $null)
    {
        write-error("Cannot find VM for machine name " + $machineName + " and vm id " + $vmId);
        return
    }

    $oldSystem = $vm.System;
    if ($oldSystem -ne $null)
    {
        $oldSystem.VMs.Remove($vm)
    }

    $system.VMs.Add($vm)
    $vm.System = $system

    # Update the file
    WriteToSystemFile $SystemFileName
}

##############################################
# Remove-VMFromSystem cmdlet implementation
# Removes a virtual machine from a system
##############################################
Function Remove-VMFromSystem($systemName, $systemId, $machineName, $vmId)
{
    Init

    if (($systemId -eq $null) -or ($systemName -eq $null) -or ($machineName -eq $null) -or ($vmId -eq $null))
    {
        return;
    }

    $system  = Get-System $systemName $systemId
    if ($system -eq $null)
    {
        write-error("Cannot find system (systemName " + $systemName + ", systemId " + $systemId + ")");
        return;
    }

    $vm  = Get-VM $machineName $vmId
    if ($vm -eq $null)
    {
        write-error("Cannot find vm (vmId " + $vmId + ", machine name " + $machineName + ")");
        return;
    }

    [void]$system.VMs.Remove($vm)
    $vm.System = $null

    # Update the file
    WriteToSystemFile $SystemFileName
}

Export-ModuleMember Get-*
Export-ModuleMember New-*
Export-ModuleMember Remove-*
Export-ModuleMember Set-*
Export-ModuleMember Add-*
