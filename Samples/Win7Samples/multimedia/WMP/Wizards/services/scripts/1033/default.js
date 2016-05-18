// Copyright (C) Microsoft Corporation. All rights reserved. 
function OnFinish(selProj, selObj)
{
    try
    {               
        var strProjectPath = wizard.FindSymbol('PROJECT_PATH');
        var strProjectName = wizard.FindSymbol('PROJECT_NAME');

        // Make the ContentDistributor string safe (esp. no spaces)
        var strContentDistributor = wizard.FindSymbol('CONTENTDISTRIBUTOR');                
        var newContentDistributor = CreateSafeName(strContentDistributor);
        wizard.AddSymbol("CONTENTDISTRIBUTOR", newContentDistributor);

        selProj = CreateCustomProject(strProjectName, strProjectPath);
        AddConfig(selProj, strProjectName);
        AddFilters(selProj);

        var InfFile = CreateCustomInfFile();
        AddFilesToCustomProj(selProj, strProjectName, strProjectPath, InfFile);
        PchSettings(selProj);
        InfFile.Delete();

        selProj.Object.Save();
    }
    catch(e)
    {
        if (e.description.length != 0)
            SetErrorInfo(e);
        return e.number
    }
}

function CreateCustomProject(strProjectName, strProjectPath)
{
    try
    {
        var strProjTemplatePath = wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
        var strProjTemplate = '';
        strProjTemplate = strProjTemplatePath + '\\default.vcproj';

        var Solution = dte.Solution;
        var strSolutionName = "";
        if (wizard.FindSymbol("CLOSE_SOLUTION"))
        {
            Solution.Close();
            strSolutionName = wizard.FindSymbol("VS_SOLUTION_NAME");
            if (strSolutionName.length)
            {
                var strSolutionPath = strProjectPath.substr(0, strProjectPath.length - strProjectName.length);
                Solution.Create(strSolutionPath, strSolutionName);
            }
        }

        var strProjectNameWithExt = '';
        strProjectNameWithExt = strProjectName + '.vcproj';

        var oTarget = wizard.FindSymbol("TARGET");
        var prj;
        if (wizard.FindSymbol("WIZARD_TYPE") == vsWizardAddSubProject)  // vsWizardAddSubProject
        {
            var prjItem = oTarget.AddFromTemplate(strProjTemplate, strProjectNameWithExt);
            prj = prjItem.SubProject;
        }
        else
        {
            prj = oTarget.AddFromTemplate(strProjTemplate, strProjectPath, strProjectNameWithExt);
        }
        return prj;
    }
    catch(e)
    {
        throw e;
    }
}

function AddFilters(proj)
{
    try
    {
        // Add the folders to your project
        var strSrcFilter = wizard.FindSymbol('SOURCE_FILTER');
        var group = proj.Object.AddFilter('Source Files');
        group.Filter = strSrcFilter;

        strSrcFilter = wizard.FindSymbol('HEADER_FILTER');
        group = proj.Object.AddFilter('Header Files');
        group.Filter = strSrcFilter;

        strSrcFilter = wizard.FindSymbol('RESOURCE_FILTER');
        group = proj.Object.AddFilter('Resource Files');
        group.Filter = strSrcFilter;
    }
    catch(e)
    {
        throw e;
    }
}

function AddConfig(proj, strProjectName)
{
    try
    {
      var type1 = wizard.FindSymbol('TYPE1PLUGIN');
      var type2 = wizard.FindSymbol('TYPE2PLUGIN');
    
      if(true == type1)
      {
        var config = proj.Object.Configurations('Debug');
        config.IntermediateDirectory = 'Debug';
        config.OutputDirectory = 'Debug';
        config.CharacterSet = charSetUnicode;
        config.ConfigurationType = typeDynamicLibrary;
        config.useOfATL = useATLStatic;
        

        var CLTool = config.Tools('VCCLCompilerTool');      
        CLTool.RuntimeLibrary = rtMultithreadedDebug; 
        CLTool.Optimization = optimizeDisabled;
        CLTool.DebugInformationFormat = debugEditAndContinue;          
        CLTool.PreprocessorDefinitions = "WIN32;_DEBUG;_WINDOWS;_USRDLL;_WINDLL;_UNICODE;UNICODE";

        var LinkTool = config.Tools('VCLinkerTool');
        LinkTool.LinkDLL = true;
        LinkTool.RegisterOutput = true;
        LinkTool.ModuleDefinitionFile = /*"$(SolutionDir)\\" + */strProjectName + ".def";
        LinkTool.BaseAddress = 0x0F100000;
        LinkTool.GenerateDebugInformation = true;
        LinkTool.ProgramDatabaseFile = /*"$(SolutionDir)\\" + */"Debug\\" + strProjectName + ".pdb";
        LinkTool.AdditionalDependencies = "crypt32.lib";
       


        config = proj.Object.Configurations('Release');
        config.IntermediateDirectory = 'Release';
        config.OutputDirectory = 'Release';
        config.CharacterSet = charSetUnicode;
        config.ConfigurationType = typeDynamicLibrary;
        config.useOfATL = useATLStatic;


        var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.RuntimeLibrary = rtMultithreaded; 
        CLTool.Optimization = optimizeMaxSpeed;
        CLTool.DebugInformationFormat = debugEnabled;   
        CLTool.PreprocessorDefinitions = "WIN32;NDEBUG;_WINDOWS;_USRDLL;_WINDLL;_UNICODE;UNICODE";
        CLTool.PrecompiledHeaderThrough = "$(SolutionDir)\\" + "stdafx.h"; 
        CLTool.PrecompiledHeaderFile = "$(SolutionDir)\\" + "Debug\\" + strProjectName + ".pch";

        var LinkTool = config.Tools('VCLinkerTool');
        LinkTool.LinkDLL = true;
        LinkTool.RegisterOutput = true;
        LinkTool.ModuleDefinitionFile = "$(SolutionDir)\\" + strProjectName + ".def";
        LinkTool.BaseAddress = 0x0F100000;
        LinkTool.GenerateDebugInformation = true;
        LinkTool.ProgramDatabaseFile = /*"$(SolutionDir)\\" + */"Release\\" + strProjectName + ".pdb";
        LinkTool.AdditionalDependencies = "crypt32.lib";
       
      }
      
      else if(true == type2)
      {  
        var config = proj.Object.Configurations('Debug');
        config.IntermediateDirectory = 'Debug';
        config.OutputDirectory = 'Debug';
        config.CharacterSet = charSetUnicode;
        config.ConfigurationType = typeDynamicLibrary;
        config.useOfATL = useATLStatic;

        var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.PreprocessorDefinitions = "WIN32;_WINDOWS;_USRDLL;_UNICODE;UNICODE";

        var LinkTool = config.Tools('VCLinkerTool');
        LinkTool.LinkDLL = true;
        LinkTool.RegisterOutput = true;
        LinkTool.ModuleDefinitionFile = /*"$(SolutionDir)\\" + */strProjectName + ".def";
        LinkTool.BaseAddress = 0x0F100000;

        config = proj.Object.Configurations('Release');
        config.IntermediateDirectory = 'Release';
        config.OutputDirectory = 'Release';
        config.CharacterSet = charSetUnicode;
        config.ConfigurationType = typeDynamicLibrary;
        config.useOfATL = useATLStatic;

        var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.PreprocessorDefinitions = "WIN32;_WINDOWS;_USRDLL;_UNICODE;UNICODE";

        var LinkTool = config.Tools('VCLinkerTool');
        LinkTool.LinkDLL = true;
        LinkTool.RegisterOutput = true;
        LinkTool.ModuleDefinitionFile = "$(SolutionDir)\\" + strProjectName + ".def";
        LinkTool.BaseAddress = 0x0F100000;
      } // else if(true == type2)
      
    } // try
    
    catch(e)
    {
        throw e;
    }
}

function PchSettings(proj)
{
   // ToDo: Specify pch settings.
}

function DelFile(fso, strWizTempFile)
{
    try
    {
        if (fso.FileExists(strWizTempFile))
        {
            var tmpFile = fso.GetFile(strWizTempFile);
            tmpFile.Delete();
        }
    }
    catch(e)
    {
        throw e;
    }
}

function CreateCustomInfFile()
{
    try
    {
        var fso, TemplatesFolder, TemplateFiles, strTemplate;
        fso = new ActiveXObject('Scripting.FileSystemObject');

        var TemporaryFolder = 2;
        var tfolder = fso.GetSpecialFolder(TemporaryFolder);
        var strTempFolder = tfolder.Drive + '\\' + tfolder.Name;

        var strWizTempFile = strTempFolder + "\\" + fso.GetTempName();

        var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
        var strInfFile = strTemplatePath + '\\Templates.inf';
        wizard.RenderTemplate(strInfFile, strWizTempFile);

        var WizTempFile = fso.GetFile(strWizTempFile);
        return WizTempFile;
    }
    catch(e)
    {
        throw e;
    }
}

function GetTargetName(strName, strProjectName)
{
    try
    {
        var strPath = "";
        var strFileName = strName;
        
        if (strName.lastIndexOf("\\") >= 0)
        {
            strFileName = strName.substr(strName.lastIndexOf("\\") + 1);
        }
        
        if (strFileName == "dll.cpp")
        {
            strTarget = strPath + strProjectName + strFileName;
        }
        else if (strFileName.substr(0, 6) == "sample")
        {
            var strlen = strFileName.length;

            strTarget = strPath + strProjectName + strFileName.substr(6, strlen - 6);
        }
        else
        {
            strTarget = strPath + strFileName;
        }
        
        return strTarget; 
    }
    catch(e)
    {
        throw e;
    }
}

function AddFilesToCustomProj(proj, strProjectName, strProjectPath, InfFile)
{
    try
    {
        var projItems = proj.ProjectItems

        var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');

        var strTpl = '';
        var strName = '';

        var strTextStream = InfFile.OpenAsTextStream(1, -2);
        while (!strTextStream.AtEndOfStream)
        {
            strTpl = strTextStream.ReadLine();

            var bExclude = false;
            if (("" != strTpl) && ("|" == strTpl.charAt(0)))
            {
                strTpl = strTpl.substr(1);
                bExclude = true;
            }

            if (strTpl != '')
            {
                strName = strTpl;
                var strTarget = GetTargetName(strName, strProjectName);
                var strTemplate = strTemplatePath + '\\' + strTpl;
                var strFile = strProjectPath + '\\' + strTarget;

                var bCopyOnly = false;  //"true" will only copy the file from strTemplate to strTarget without rendering/adding to the project
                var strExt = strName.substr(strName.lastIndexOf("."));
                if(strExt==".bmp" || strExt==".ico" || strExt==".gif" || strExt==".rtf" || strExt==".css")
                    bCopyOnly = true;
                wizard.RenderTemplate(strTemplate, strFile, bCopyOnly);
                var file = proj.Object.AddFile(strFile);
                if (bExclude)
                {
                    var col = file.FileConfigurations;
                    for (var i = 1; i <= col.Count; i++)
                    {
                        var fileconfig = col.Item(i);
                        fileconfig.ExcludedFromBuild = true;
                    }
                }
            }
        }
        strTextStream.Close();
/*
        var col = proj.Object.Files;
        var file = col.Item("SetContentDistributor.cpp");
        col = file.FileConfigurations;
        var fileconfig = col.Item(1);
        fileconfig.ExcludedFromBuild = true; */
    }
    catch(e)
    {
        throw e;
    }
}
