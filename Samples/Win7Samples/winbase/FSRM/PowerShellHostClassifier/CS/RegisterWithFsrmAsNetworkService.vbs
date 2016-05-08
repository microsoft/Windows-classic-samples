option explicit

 

dim classMan

set classMan = CreateObject( "Fsrm.FsrmClassificationManager")

 

dim module

' CreateModuleDefinition takes a parameter of FsrmPipelineModuleType

' Values for this enum are:

'    FsrmPipelineModuleType_Storage      = 1,

'    FsrmPipelineModuleType_Classifier   = 2,

set module = classMan.CreateModuleDefinition(2)

 

module.ModuleClsid = "{2573306a-2519-4e82-9617-5f99c6137c51}"

module.Name = "Powershell Host Classifier"

module.Company = "Microsoft Corporation"

module.Version = "1.0.0.0"

module.Enabled = true

module.NeedsFileContent = true

module.NeedsExplicitValue = false

module.Description = "Classifies files using a supplied powershell script. Which must be supplied via the Additional Classification Parameters, click Advanced"

module.Parameters = Array("StaticModuleName=Powershell Host Classifier", "ParameterDescription=Must specify " &vbCrLf & "  Name: ScriptFileName and Value: the full path to the script file" & vbCrLf & "This along with any aditional values will be accessible via the $rule.Parameters variable in the script.")


' The Account property is of type FsrmAccountType

' Values for this enum are:

' FsrmAccountType_NetworkService = 1,

' FsrmAccountType_LocalService = 2,

' FsrmAccountType_LocalSystem = 3,

' FsrmAccountType_InProc = 4,

' FsrmAccountType_External = 5,

module.Account = 1

module.Commit()
