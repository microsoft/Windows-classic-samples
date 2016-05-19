option explicit

 

dim classMan

set classMan = CreateObject( "Fsrm.FsrmClassificationManager")

 

dim module

' CreateModuleDefinition takes a parameter of FsrmPipelineModuleType

' Values for this enum are:

'    FsrmPipelineModuleType_Storage      = 1,

'    FsrmPipelineModuleType_Classifier   = 2,

set module = classMan.CreateModuleDefinition(2)

 

module.ModuleClsid = "{4d292ba9-366b-42ff-ab6b-274953475166}"

module.Name = "Managed Sample Classifier"

module.Enabled = true

module.Company = "Microsoft Corporation"

module.Version = "1.0.0.0"

module.NeedsFileContent = true

module.Description = "Classifies files whose content matches the regular expression defined in the rule."

module.Parameters = Array("StaticModuleName=Managed Sample Classifier", "Encoding=UTF8", "ParameterDescription=Specify the criteria to match files against. Valid parameter is:" & vbCrLf & "" & vbCrLf & "RegularExpression:  Match this regular expression using .Net syntax.See http://msdn.microsoft.com/en-us/library/ae5bf541.aspx for the complete specification. For example, '\d\d\d' will match any three-digit number." & vbCrLf & "" & vbCrLf & "Only one parameter is accepted.")


' The Account property is of type FsrmAccountType

' Values for this enum are:

' FsrmAccountType_NetworkService = 1,

' FsrmAccountType_LocalService = 2,

' FsrmAccountType_LocalSystem = 3,

' FsrmAccountType_InProc = 4,

' FsrmAccountType_External = 5,

module.Account = 5

module.Commit()
