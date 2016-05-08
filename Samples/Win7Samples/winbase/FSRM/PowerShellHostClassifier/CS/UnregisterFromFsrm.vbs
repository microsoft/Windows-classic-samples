option explicit

 

dim classMan

set classMan = CreateObject( "Fsrm.FsrmClassificationManager")

 
Dim module
Dim modules

set modules = classMan.EnumModuleDefinitions(2, 0)

for each module in modules

	if (module.Name = "Powershell Host Classifier")
		module.delete
		module.commit
	endif
next
