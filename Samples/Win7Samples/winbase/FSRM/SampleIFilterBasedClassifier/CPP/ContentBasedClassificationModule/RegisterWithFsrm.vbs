option explicit

 

dim classMan

set classMan = CreateObject( "Fsrm.FsrmClassificationManager")

 

dim module

' CreateModuleDefinition takes a parameter of FsrmPipelineModuleType

' Values for this enum are:

'    FsrmPipelineModuleType_Storage      = 1,

'    FsrmPipelineModuleType_Classifier   = 2,

set module = classMan.CreateModuleDefinition(2)

 

module.ModuleClsid = "{7853D011-8F9D-4831-BA8A-E5EE99B7CC65}"

module.Name = "Sample IFilter Based Classifier"

module.Enabled = true

module.Description = "Classifies files that match a string in the file's content obtained through the file's associated iFilter"

module.Company = "Microsoft Corporation"

module.Version = "1.0.0.0"

module.NeedsFileContent = true

module.Parameters = Array("StaticModuleName=Sample IFilter Based Classifier", "ParameterDescription=Specify the criteria to match files against. Enter the strings to search in either the key or value box above. More than one search string can be specified. If any of the strings are found in the content of the file obtained through the associated iFilter, the rule will be applied.")


' The Account property is of type FsrmAccountType

' Values for this enum are:

' FsrmAccountType_NetworkService = 1,

' FsrmAccountType_LocalService = 2,

' FsrmAccountType_LocalSystem = 3,

' FsrmAccountType_InProc = 4,

' FsrmAccountType_External = 5,

module.Account = 5

module.Commit()
