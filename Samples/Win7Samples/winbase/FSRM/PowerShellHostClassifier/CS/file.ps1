[Reflection.Assembly]::LoadFile("c:\Windows\System32\srmlib.dll")
#[Microsoft.Storage.IFsrmPropertyBag]$b = $propertyBag
foreach($rule in $rules){
	if($FileName -eq "3.txt"){
		$rule.HasMatched = $true
	}
}

#$b.Name