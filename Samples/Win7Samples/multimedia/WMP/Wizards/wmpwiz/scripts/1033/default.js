/* (C) 2001 Microsoft Corporation  */

var bCopyOnly = false;

function OnFinish(selProj, selObj)
{
    try
    {
        var strProjectPath = wizard.FindSymbol("PROJECT_PATH");
        var strProjectName = wizard.FindSymbol("PROJECT_NAME");

        var bEmptyProject = wizard.FindSymbol("EMPTY_PROJECT");
        var bVS70 = wizard.FindSymbol("VS70");       
        var bDSP = wizard.FindSymbol("DSPPLUGIN");

        selProj = CreateProject(strProjectName, strProjectPath);

        AddCommonConfig(selProj, strProjectName);
        AddSpecificConfig(selProj, strProjectName, bEmptyProject);
        
        if(bVS70)
        {
            SetFilters(selProj);
        }
        else
        {
            SetupFilters(selProj);
        }
        
        if (!bEmptyProject)
        {
            if(bVS70)
            {
                var InfFile = CreateInfFile();
                AddFilesToProject(selProj, strProjectName, InfFile);
                SetCommonPchSettings(selProj);
                InfFile.Delete();
            }
            else
            {
                AddFilesToProjectWithInfFile(selProj, strProjectName);
                SetCommonPchSettings(selProj);
            }
            
            var projName = strProjectPath + "\\" + strProjectName + ".vcproj";
            selProj.Object.Save();
            
            if(bDSP)
            {
                // Add the proxy/stub project.
                
                strProjectPath += "\\" + strProjectName + "PS";
                
                var strDefFile = strProjectPath + "\\" + strProjectName + "PS.def";
                var strIDLFile = strProjectPath + "\\" + strProjectName + ".idl";
                var str_PFile = strProjectPath + "\\" + strProjectName + "_p.c";
                var str_IFile = strProjectPath + "\\" + strProjectName + "_i.c";
                var strResFile = strProjectPath + "\\" + "resource.h";
                var strRCFile = strProjectPath + "\\" + strProjectName + "PS.rc";
                var strDataFile = strProjectPath + "\\dlldata.c";
                
                strProjectName += "PS";
                wizard.AddSymbol("CLOSE_SOLUTION", false);
                var oPSProj = CreateProject(strProjectName, strProjectPath);
                
                SetPSConfigurations(oPSProj, selProj);
                
                var strTemplatePath = wizard.FindSymbol("TEMPLATES_PATH") + "\\dspplugin\\dsppluginPS";
                
                wizard.RenderTemplate(strTemplatePath + "\\ps.def", strDefFile);
                wizard.RenderTemplate(strTemplatePath + "\\sample.idl", strIDLFile);
                wizard.RenderTemplate(strTemplatePath + "\\resource.h", strResFile);
                wizard.RenderTemplate(strTemplatePath + "\\ps.rc", strRCFile);			
                
                var oHeaderFiles = oPSProj.Object.AddFilter("Header Files");
                oHeaderFiles.AddFile(strResFile);
                var oResourceFiles = oPSProj.Object.AddFilter("Resource Files");
                oResourceFiles.AddFile(strRCFile);
                var oSourceFiles = oPSProj.Object.AddFilter("Source Files");
                oSourceFiles.AddFile(strIDLFile);
                oSourceFiles.AddFile(strDefFile);
                oSourceFiles.AddFile(str_IFile);
                oSourceFiles.AddFile(str_PFile);                                   
                oSourceFiles.AddFile(strDataFile);
                
                oPSProj.Object.Save();
            }
        }
    }
    catch(e)
    {
        if (e.description.length != 0)
            SetErrorInfo(e);
        return e.number
    }
}

function GetTargetName(strName, strProjectName, strResourcePath)
{
    var strFileName = strName;
    
    if (strName.lastIndexOf("\\") >= 0)
    {
        strFileName = strName.substr(strName.lastIndexOf("\\") + 1);
    }
    
    if ((strFileName == "dll.cpp") || (strFileName == "events.cpp"))
    {
        strTarget = strProjectName + strFileName;
    }
    else if ((strFileName == "PropertyPage.cpp") || (strFileName == "PropertyPage.h") || (strFileName == "PropertyPage.rgs"))
    {
        strTarget = strProjectName + "PropPage" + strFileName.substr(12);
    }
    else if (strFileName == "sample_video.cpp")
    {
        strTarget = strProjectName + ".cpp";
    }
    else if (strFileName == "sample_video.h")
    {
        strTarget = strProjectName + ".h";
    }
    else if (strFileName == "ps.def" || strFileName == "ps.rc")
    {
        strTarget = strProjectName + "PS" + strFileName.substr(2);
    }
        else if (strFileName.substr(0, 6) == "sample")
    {
        var strlen = strFileName.length;

        strTarget = strProjectName + strFileName.substr(6, strlen - 6);
    }
    else
    {
        strTarget = strFileName;
    }
    
    return strTarget; 
}


function AddSpecificConfig(proj, strProjectName, bEmptyProject)
{
    var bMFC = wizard.FindSymbol("SUPPORT_MFC");
    var bNeedTypeLibPath = wizard.FindSymbol("VISPLUGIN");
    var bNeedMSDMO = (wizard.FindSymbol("DSPPLUGIN") || wizard.FindSymbol("RENDERINGPLUGIN")) ? true : false;
    var bNeedMfPlat = (wizard.FindSymbol("DSPPLUGIN") && wizard.FindSymbol("DUALMODE")) ? true: false;

    var config = proj.Object.Configurations("Debug|Win32");
    config.CharacterSet = charSetUnicode;
    config.ConfigurationType = typeDynamicLibrary;
    config.useOfATL = useATLStatic;

    var CLTool = config.Tools("VCCLCompilerTool");
    CLTool.PreprocessorDefinitions = "WIN32;_WINDOWS;_USRDLL;_WINNT_WIN32=0x5000;_UNICODE;UNICODE";
    if(wizard.FindSymbol("DSPPLUGIN"))
    {
        CLTool.RuntimeLibrary = rtMultiThreadedDebugDLL;
    }

    var LinkTool = config.Tools("VCLinkerTool");
    LinkTool.LinkDLL = true;
    LinkTool.RegisterOutput = true;
    LinkTool.ModuleDefinitionFile = strProjectName + ".def";
        LinkTool.BaseAddress = 0x0F300000;
    if (bNeedMSDMO)
    {
        oldDeps = LinkTool.AdditionalDependencies;
        LinkTool.AdditionalDependencies = oldDeps + "msdmo.lib";
    }

    if (bNeedTypeLibPath)
    {
        var RCTool = config.Tools("Resource Compiler Tool");

        oldDeps = RCTool.AdditionalIncludeDirectories;
        RCTool.AdditionalIncludeDirectories = oldDeps + "$(IntDir)";
    }

    if (bNeedMfPlat)
    {
        oldDeps = LinkTool.AdditionalDependencies;
        LinkTool.AdditionalDependencies = oldDeps + " mfplat.lib";
    }



    config = proj.Object.Configurations.Item("Release|Win32");
    config.CharacterSet = charSetUnicode;
    config.ConfigurationType = typeDynamicLibrary;
    config.useOfATL = useATLStatic;

    var CLTool = config.Tools("VCCLCompilerTool");
    CLTool.PreprocessorDefinitions = "WIN32;_WINDOWS;_USRDLL;_WINNT_WIN32=0x5000;_UNICODE;UNICODE";
    if(wizard.FindSymbol("DSPPLUGIN"))
    {
        CLTool.RuntimeLibrary = rtMultiThreadedDLL;
    }

    var LinkTool = config.Tools("VCLinkerTool");
    LinkTool.LinkDLL = true;
    LinkTool.RegisterOutput = true;
    LinkTool.ModuleDefinitionFile = strProjectName + ".def";      
    LinkTool.BaseAddress = 0x0F300000;

    if (bNeedMSDMO)
    {
        oldDeps = LinkTool.AdditionalDependencies;
        LinkTool.AdditionalDependencies = oldDeps + "msdmo.lib";
    }

    if (bNeedTypeLibPath)
    {
        var RCTool = config.Tools("Resource Compiler Tool");

        oldDeps = RCTool.AdditionalIncludeDirectories;
        RCTool.AdditionalIncludeDirectories = oldDeps + "$(IntDir)";
    }

    if (bNeedMfPlat)
    {
        oldDeps = LinkTool.AdditionalDependencies;
        LinkTool.AdditionalDependencies = oldDeps + " mfplat.lib";
    }
}


function SetFileProperties(projfile, strName)
{
    return false;
}

function DoOpenFile(strTarget)
{
    return false;
}

function SetPSConfigurations(oProj, oMainProj)
{
    try
    {
        oConfigs = oProj.Object.Configurations;

        for (var nCntr = 1; nCntr <= oConfigs.Count; nCntr++)
        {
            var oConfig = oConfigs(nCntr);
            var bDebug = false;
            if (-1 != oConfig.Name.indexOf("Debug"))
                bDebug = true;

            oConfig.ConfigurationType = typeDynamicLibrary;
            oConfig.CharacterSet = charSetUnicode;
            var oCLTool = oConfig.Tools("VCCLCompilerTool");

            var strDefines = oCLTool.PreprocessorDefinitions;
            if (strDefines != "") strDefines += ";";
            strDefines += GetPlatformDefine(oConfig);
            strDefines += "_WIN32_WINNT=0x0500;_WINDOWS;_USRDLL;REGISTER_PROXY_DLL";
            if (bDebug)
            {
                strDefines += ";_DEBUG";
                oCLTool.RuntimeLibrary = rtMultiThreadedDebugDLL;
                oCLTool.Optimization = "0";
                oCLTool.MinimalRebuild = "true";
                oCLTool.BasicRuntimeChecks = "3";
                oCLTool.UsePrecompiledHeader = "0";
                oCLTool.WarningLevel = "3";
                // oCLTool.Detect64BitPortabilityProblems="true"; deprecated
                oCLTool.DebugInformationFormat="4"
            }
            else
            {
                strDefines += ";NDEBUG";
                oCLTool.RuntimeLibrary = rtMultiThreadedDLL;
                oCLTool.Optimization = optimizeMaxSpeed;
            }
            oConfig.IntermediateDirectory = "$(ConfigurationName)PS";
            oConfig.OutputDirectory = "$(ConfigurationName)PS";
            oCLTool.PreprocessorDefinitions = strDefines;

            var oLinkTool = oConfig.Tools("VCLinkerTool");
            oLinkTool.AdditionalDependencies = "rpcrt4.lib";
            oLinkTool.ModuleDefinitionFile = oProj.Name + ".def";
            oLinkTool.TargetMachine = "1";
            oLinkTool.OptimizeReferences = optNoReferences;
            oLinkTool.EnableCOMDATFolding = optNoFolding;

            if (!bDebug)
            {
                oLinkTool.EnableCOMDATFolding = optFolding;
                oLinkTool.OptimizeReferences = optReferences;
            }

            oLinkTool.RegisterOutput = true;
        }
        
        // Set the build order
        // The PS project must build before the main project.
        var oSolBuild = dte.Solution.SolutionBuild;
        oSolBuild.BuildDependencies(oMainProj.UniqueName).AddProject(oProj.UniqueName);
    }
    catch(e)
    {
        throw e;
    }
}
