option explicit

 

dim classMan

set classMan = CreateObject( "Fsrm.FsrmClassificationManager")

 

dim module

' CreateModuleDefinition takes a parameter of FsrmPipelineModuleType

' Values for this enum are:

'    FsrmPipelineModuleType_Storage      = 1,

'    FsrmPipelineModuleType_Classifier   = 2,

set module = classMan.CreateModuleDefinition(2)

 

module.ModuleClsid = "{0F5F3806-2DAD-4050-9D30-50C35FB7ED0E}"

module.Name = "Sample Classifier"

module.Enabled = true

module.Description = "Classifies files that match a string in the file's name and/or the file's content"

module.Company = "Microsoft Corporation"

module.Version = "1.0.0.0"

module.NeedsFileContent = true

module.Parameters = Array("StaticModuleName=Sample Classifier", "ParameterDescription=Specify the criteria to match files against. Valid parameter names are:" & vbCrLf & "" & vbCrLf & "FileNameContains: Match the string against the file's name." & vbCrLf & "" & vbCrLf & "FileContentContains: Match the string against the contents of the file." & vbCrLf & "" & vbCrLf & "Any one of the two parameters need to match for the rule to apply.")

' The Account property is of type FsrmAccountType

' Values for this enum are:

' FsrmAccountType_NetworkService = 1,

' FsrmAccountType_LocalService = 2,

' FsrmAccountType_LocalSystem = 3,

' FsrmAccountType_InProc = 4,

' FsrmAccountType_External = 5,

module.Account = 5

module.Commit()
