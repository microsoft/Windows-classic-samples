// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// The code in this file demonstrates how to use the IShellLibrary API by building a simple application
// that exposes all of the functionality of that API via the command-line.  The boiler-plate code in
// CmdLineBase.h/.cpp is used to simplify the tasks of processing options, printing usage information etc.
// but this is not required to use the IShellLibrary API.
//
// The application provides eight sub-commands that expose different parts of the IShellLibrary API as follows:
//
//   create      Creates a library at the specified path.
//   info        Prints info about the given library.
//   enum        Enumerates the folders in the library.
//   add         Adds the specified folder to the specified library.
//   remove      Removes the specified folder from the library.
//   resolve     Resolves the specified folder in the library.
//   resolveall  Resolves all locations in the library in bulk.
//   manage      Displays the Manage Library Dialog for the library.
//
// Usage information for each command can be obtained via the '-?' option (-h and -help are also supported).

#include <windows.h>
#include <stdio.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include "CmdLineBase.h"

// returns the first argument, and advances ppszArgs and cArgs past it to the next one
#define CONSUME_NEXT_ARG(ppszArgs, cArgs) ((cArgs)--, ((ppszArgs)++)[0])

// forward decl
class CCreateCommand;
class CInfoCommand;
class CEnumCommand;
class CSetAttributeCommand;
class CAddCommand;
class CRemoveCommand;
class CSetDefaultSaveFolderCommand;
class CResolveCommand;
class CResolveAllCommand;
class CManageCommand;

// main function - sets up a 'meta command' to contain each of the library commands, and executes it on the given arguments
int wmain(int argc, wchar_t *argv[])
{
    // initialize COM before doing anything else, since the IShellLibrary API depends on COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CMetaCommand::PFNCREATECOMMAND rgMainCmds[] =
        {
            CMetaCommand::Create<CCreateCommand>,
            CMetaCommand::Create<CInfoCommand>,
            CMetaCommand::Create<CEnumCommand>,
            CMetaCommand::Create<CSetAttributeCommand>,
            CMetaCommand::Create<CAddCommand>,
            CMetaCommand::Create<CRemoveCommand>,
            CMetaCommand::Create<CSetDefaultSaveFolderCommand>,
            CMetaCommand::Create<CResolveCommand>,
            CMetaCommand::Create<CResolveAllCommand>,
            CMetaCommand::Create<CManageCommand>,
        };

        {
            PCWSTR pszExeName = PathFindFileNameW(CONSUME_NEXT_ARG(argv, argc));
            CMetaCommand main(pszExeName, L"Displays and modifies the attributes of Shell Libraries.", rgMainCmds, ARRAYSIZE(rgMainCmds));
            main.Execute(const_cast<PCWSTR*>(argv), argc);
        }
        CoUninitialize();
    }

    return 0;
}


// CShlibCommandBase - Base class for commands in shlib.exe
//
// This class provides functionality that is shared across all commands, in particular:
//  - Processing the <Library> argument to create an IShellItem for the specified item (file system path or KNOWNFOLDERID)
//  - Loading the IShellLibrary interface for the specified item
//  - Committing changes after the operation is complete
//  - Options for specifying the creation disposition

class CShlibCommandBase :
    public CCmdBase
{
public:
    CShlibCommandBase(PCWSTR pszName, PCWSTR pszDescription, BOOL fReadOnly = TRUE, BOOL fCreate = FALSE) :
        CCmdBase(pszName, pszDescription, L"<Library> [...]"),
        _fReadOnly(fReadOnly),
        _fCreate(fCreate),
        _lsfSaveOptions(LSF_FAILIFTHERE),
        _psiLibrary(NULL),
        _plib(NULL),
        _pszSavePath(NULL)
    {
        // Options for specifying the creation disposition.
        ARGENTRY<LIBRARYSAVEFLAGS> c_rgLibSaveFlags[] =
        {
            { L"",           LSF_FAILIFTHERE,      L"Fail if the library already exists." },
            { L"overwrite",  LSF_OVERRIDEEXISTING, L"Overwrite any existing library." },
            { L"uniquename", LSF_MAKEUNIQUENAME,   L"Generate a unique name in case of conflict." },
        };

        if (!_fReadOnly)
        {
            AddEnumOptionHandler(L"create", L"creation flag", L"Specifies that a new library should be created.",
                                 &CShlibCommandBase::SetCreateFlags, c_rgLibSaveFlags, ARRAYSIZE(c_rgLibSaveFlags));
        }
    }

    void v_PrintInstructions() const
    {
        Output(L"The library may be specified by a file system path, or by a KNOWNFOLDERID (e.g. \"FOLDERID_DocumentsLibrary\").\n");
        v_PrintLibInstructions(); // Print additional instructions specified by the derived class.
    }

    // Processes a single argument which identifies the library to operate on; passes any remaining arguments to the derived class.
    HRESULT v_ProcessArguments(PCWSTR *ppszArgs, int cArgs)
    {
        PCWSTR pszLibPath = CONSUME_NEXT_ARG(ppszArgs, cArgs);
        HRESULT hr = pszLibPath ? S_OK : E_INVALIDARG;
        if (SUCCEEDED(hr))
        {
            if (_fCreate)
            {
                // When creating a new library, interpret the argument as the file system path to save the library to.
                WCHAR szAbsPath[MAX_PATH];
                hr = SHStrDupW(_wfullpath(szAbsPath, pszLibPath, ARRAYSIZE(szAbsPath)), &_pszSavePath);
            }
            else
            {
                // Check for the 'FOLDERID_' prefix, which indicates that the argument should be interpreted as a KNOWNFOLDERID.
                const WCHAR szPrefix[] = L"FOLDERID_";
                const UINT cchPrefix = ARRAYSIZE(szPrefix) - 1;
                if (StrCmpNCW(pszLibPath, szPrefix, cchPrefix) == 0)
                {
                    IKnownFolderManager *pkfm;
                    hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
                    if (SUCCEEDED(hr))
                    {
                        // KNOWNFOLDERIDs are GUIDs, but they have a corresponding canonical name which is a string.
                        // By convention, the canonical name is the same as the name of the KNOWNFOLDERID #define.
                        // That is, FOLDERID_DocumentsLibrary => "DocumentsLibrary".  So, skip the prefix and pass
                        // the remainder to GetFolderByName to retrieve the known folder.
                        IKnownFolder *pkf;
                        hr = pkfm->GetFolderByName(pszLibPath + cchPrefix, &pkf);
                        if (SUCCEEDED(hr))
                        {
                            hr = pkf->GetShellItem(KF_FLAG_INIT, IID_PPV_ARGS(&_psiLibrary));
                            pkf->Release();
                        }
                        pkfm->Release();
                    }
                }
                else
                {
                    // Default - interpret the argument as a file system path, and create a shell item for it.
                    WCHAR szAbsPath[MAX_PATH];
                    hr = SHCreateItemFromParsingName(_wfullpath(szAbsPath, pszLibPath, ARRAYSIZE(szAbsPath)), NULL, IID_PPV_ARGS(&_psiLibrary));
                }
            }
        }
        else
        {
            ParseError(L"Missing library path argument.\n");
        }

        if (SUCCEEDED(hr))
        {
            // Allow derived command to process any remaining arguments.
            hr = v_ProcessLibArguments(ppszArgs, cArgs);
        }
        return hr;
    }

    // Called by the option processor in CCmdBase to set the creation disposition (if specified by the -create option).
    HRESULT SetCreateFlags(LIBRARYSAVEFLAGS lsfSaveOptions)
    {
        _fCreate = TRUE;
        _lsfSaveOptions = lsfSaveOptions;
        return S_OK;
    }

    // Loads the IShellLibrary interface for the specified item, calls the derived class to perform an operation on
    // the library, and commits/saves any changes as needed.
    HRESULT v_ExecuteCommand()
    {
        HRESULT hr;
        if (_fCreate)
        {
            // If we're in 'create' mode, instantiate a new IShellLibrary in memory.
            hr = SHCreateLibrary(IID_PPV_ARGS(&_plib));
        }
        else
        {
            // Otherwise, load it from the specified IShellItem.
            const DWORD grfMode = _fReadOnly ? (STGM_READ | STGM_SHARE_DENY_WRITE) : (STGM_READWRITE | STGM_SHARE_EXCLUSIVE);
            hr = SHLoadLibraryFromItem(_psiLibrary, grfMode, IID_PPV_ARGS(&_plib));
        }

        if (SUCCEEDED(hr))
        {
            // Call the derived class to execute the operation on the library.
            hr = v_ExecuteLibCommand();
            if (SUCCEEDED(hr) && !_fReadOnly)
            {
                if (_fCreate)
                {
                    // We created a new library in memory; now save it to disk.
                    // The IShellLibrary::Save API takes the destination in the form of the parent folder, and the name
                    // of the library (without any file extension).  However, the argument is in the form of a full file
                    // system path, possibly including the extension.  So, we need to parse it into that form.  For example:
                    //   "C:\some\folder\stuff.library-ms" => "C:\some\folder", "stuff"
                    PWSTR pszName = PathFindFileNameW(_pszSavePath);
                    if (StrCmpICW(PathFindExtensionW(pszName), L".library-ms") == 0)
                    {
                        PathRemoveExtensionW(pszName);
                    }
                    PathRemoveFileSpec(_pszSavePath);

                    // Save the library with the specified name in the specified folder.
                    PWSTR pszSavedToPath;
                    hr = SHSaveLibraryInFolderPath(_plib, _pszSavePath, pszName, _lsfSaveOptions, &pszSavedToPath);
                    if (SUCCEEDED(hr))
                    {
                        // The API returns the full file system path that the library was saved to.
                        // (This may or may not match the original argument, depending on whether LSF_MAKEUNIQUENAME was specified.)
                        Output(L"Library saved to path: %s\n", pszSavedToPath);
                        CoTaskMemFree(pszSavedToPath);
                    }
                    else
                    {
                        RuntimeError(L"Error %#08x saving library to path: %s\\%s.library-ms\n", hr, _pszSavePath, pszName);
                    }
                }
                else
                {
                    // We're operating on an existing library; commit the changes to disk.
                    hr = _plib->Commit();
                    if (SUCCEEDED(hr))
                    {
                        Output(L"Changes successfully committed.\n");
                    }
                }
            }
        }
        else
        {
            RuntimeError(L"Error %#08x loading library from path: %s\n", hr, _pszSavePath);
        }
        return hr;
    }

    ~CShlibCommandBase()
    {
        if (_psiLibrary)
        {
            _psiLibrary->Release();
        }
        if (_plib)
        {
            _plib->Release();
        }
        CoTaskMemFree(_pszSavePath);
    }

protected:
    // derived classes override these functions to process arguments, execute operations on the library, and print usage instructions
    virtual HRESULT v_ProcessLibArguments(PCWSTR*, int) { return S_OK; }
    virtual HRESULT v_ExecuteLibCommand() = 0;
    virtual void v_PrintLibInstructions() const {}

    IShellItem *_psiLibrary;  // The IShellItem representing the library; may be NULL if _fCreate is TRUE.
    IShellLibrary *_plib;     // The IShellLibrary loaded for the specified item; initialized by v_ExecuteCommand prior to calling v_ExecuteLibCommand.
    BOOL _fCreate;            // Indicates whether the user specified to create a new library.

private:
    BOOL _fReadOnly;                   // Indicates that the library should be initialized for read-only access; specified by the derived class.
    LIBRARYSAVEFLAGS _lsfSaveOptions;  // Specifies the creation disposition; provided by the user via the -create option.
    PWSTR _pszSavePath;                // Specifies the path to save the newly-created library to; may be NULL if _fCreate is FALSE.
};


// CCreateCommand - Creates a new library with the specified path/name.
//
// This simple command hard-codes the -create option (although the user can still specify it to indicate the creation
// disposition to use) and performs no operations on the library.  The result is that a new, empty library is created.

class CCreateCommand :
    public CShlibCommandBase
{
public:
    CCreateCommand() :
        CShlibCommandBase(L"create", L"Creates a library at the specified path.", FALSE, TRUE)
    {}

    HRESULT v_ExecuteLibCommand()
    {
        return S_OK;
    }
};


// CEnumCommand - Enumeates the locations included in the specified library.
//
// The 'enum' command uses the IShellLibrary::GetFolders method to list the locations that are included in the library.
// It has a single option, which is used to specify the LIBRARYFOLDERFILTER enum to pass to the API to select what folders
// to filter out of the list (if any).
//
// This class also serves as the base class for the 'info' command, since that command simply builds on the functionality of this one.

class CEnumCommand :
    public CShlibCommandBase
{
public:
    CEnumCommand(PCWSTR pszName = L"enum", PCWSTR pszDescription = L"Enumerates the folders in the library.") :
        CShlibCommandBase(pszName, pszDescription),
        _lffFilter(LFF_ALLITEMS)
    {
        ARGENTRY<LIBRARYFOLDERFILTER> c_rgLibFolderFilters[] =
        {
            { L"allitems", LFF_ALLITEMS,        L"Include all items." },
            { L"all",      LFF_ALLITEMS,        L"Synonym for 'allitems'." },
            { L"",         LFF_ALLITEMS,        L"Synonym for 'allitems'." },
            { L"filesys",  LFF_FORCEFILESYSTEM, L"Include only file system items." },
            { L"fs",       LFF_FORCEFILESYSTEM, L"Synonym for 'filesys'." },
            { L"storage",  LFF_STORAGEITEMS,    L"Include any IStorage-based item." },
            { L"stg",      LFF_STORAGEITEMS,    L"Synonym for 'storage'." },
        };

        AddEnumOptionHandler(L"filter", L"folder filter", L"Specifies which library locations to include in the enumeration.",
                             &CEnumCommand::SetFolderFilter, c_rgLibFolderFilters, ARRAYSIZE(c_rgLibFolderFilters));
    }

    void v_PrintInstructions() const
    {
        Output(L"The private and public default save locations are indicated in the output with the \'*\' and \'#\' symbols, respectively.\n");
    }

    HRESULT v_ExecuteLibCommand()
    {
        // Get the private and public save locations.
        IShellItem *psiPrivateSaveLoc;
        HRESULT hr = _plib->GetDefaultSaveFolder(DSFT_PRIVATE, IID_PPV_ARGS(&psiPrivateSaveLoc));
        if (SUCCEEDED(hr))
        {
            IShellItem *psiPublicSaveLoc;
            hr = _plib->GetDefaultSaveFolder(DSFT_PUBLIC, IID_PPV_ARGS(&psiPublicSaveLoc));
            if (SUCCEEDED(hr))
            {
                // Get the list of folders that match the specified filter.
                IShellItemArray *psiaFolders;
                hr = _plib->GetFolders(_lffFilter, IID_PPV_ARGS(&psiaFolders));
                if (SUCCEEDED(hr))
                {
                    DWORD cFolders;
                    hr = psiaFolders->GetCount(&cFolders);
                    if (SUCCEEDED(hr))
                    {
                        Output(L"Library contains %u folders:\n", cFolders);
                        for (DWORD iFolder = 0; iFolder < cFolders; iFolder++)
                        {
                            IShellItem *psiFolder;
                            if (SUCCEEDED(psiaFolders->GetItemAt(iFolder, &psiFolder)))
                            {
                                // Print each folder's name as an absolute path, suitable for parsing in the Shell Namespace (e.g SHParseDisplayName).
                                // For file system folders (the typical case), this will be the file system path of the folder.
                                PWSTR pszDisplay;
                                if (SUCCEEDED(psiFolder->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszDisplay)))
                                {
                                    PCWSTR pszPrefix = L"  ";
                                    int iCompare;
                                    if (S_OK == psiPrivateSaveLoc->Compare(psiFolder, SICHINT_CANONICAL | SICHINT_TEST_FILESYSPATH_IF_NOT_EQUAL, &iCompare))
                                    {
                                        pszPrefix = L"* ";
                                    }
                                    else if (S_OK == psiPublicSaveLoc->Compare(psiFolder, SICHINT_CANONICAL | SICHINT_TEST_FILESYSPATH_IF_NOT_EQUAL, &iCompare))
                                    {
                                        pszPrefix = L"# ";
                                    }

                                    Output(L"%s%s\n", pszPrefix, pszDisplay);
                                    CoTaskMemFree(pszDisplay);
                                }
                                psiFolder->Release();
                            }
                        }
                    }
                    psiaFolders->Release();
                }
                psiPublicSaveLoc->Release();
            }
            psiPrivateSaveLoc->Release();
        }
        return hr;
    }

    // Called by the option processor in CCmdBase to set the value specified by the -filter option (if any).
    HRESULT SetFolderFilter(LIBRARYFOLDERFILTER lffFilter)
    {
        _lffFilter = lffFilter;
        return S_OK;
    }

private:
    LIBRARYFOLDERFILTER _lffFilter;  // The filter to apply to folders in the library.
};


// CInfoCommand - Displays information about the specified library, including its options, folder type, icon, and locations.
//
// The 'info' command builds on the 'enum' command by outputting some additional information about the library that is exposed
// by the IShellLibrary API.

class CInfoCommand :
    public CEnumCommand
{
public:
    CInfoCommand() :
        CEnumCommand(L"info", L"Prints info about the given library.")
    {}

    HRESULT v_ExecuteLibCommand()
    {
        // Display the option flags for the library (there is presently only one flag defined - LOF_PINNEDTONAVPANE)
        LIBRARYOPTIONFLAGS lofOptions;
        _plib->GetOptions(&lofOptions);
        Output(L"Option flags: ");
        if (lofOptions == LOF_DEFAULT)
        {
            Output(L"LOF_DEFAULT");
        }
        else if (lofOptions & LOF_PINNEDTONAVPANE)
        {
            Output(L"LOF_PINNEDTONAVPANE");
        }
        Output(L"\n");

        // Display the folder type of the library; see shlguid.h for a list of valid folder types.
        FOLDERTYPEID ftid;
        WCHAR szFolderType[39];
        _plib->GetFolderType(&ftid);
        if (ftid == FOLDERTYPEID_Documents)
        {
            StringCchCopyW(szFolderType, ARRAYSIZE(szFolderType), L"FOLDERTYPEID_Documents");
        }
        else if (ftid == FOLDERTYPEID_Pictures)
        {
            StringCchCopyW(szFolderType, ARRAYSIZE(szFolderType), L"FOLDERTYPEID_Pictures");
        }
        else if (ftid == FOLDERTYPEID_Music)
        {
            StringCchCopyW(szFolderType, ARRAYSIZE(szFolderType), L"FOLDERTYPEID_Music");
        }
        else if (ftid == FOLDERTYPEID_Videos)
        {
            StringCchCopyW(szFolderType, ARRAYSIZE(szFolderType), L"FOLDERTYPEID_Videos");
        }
        else if (ftid == GUID_NULL)
        {
            StringCchCopyW(szFolderType, ARRAYSIZE(szFolderType), L"<none>");
        }
        else
        {
            StringFromGUID2(ftid, szFolderType, ARRAYSIZE(szFolderType));
        }
        Output(L"Folder type: %s\n", szFolderType);

        // Display the path of the library's icon; this is also accessible via the Property System as PKEY_IconPath
        PWSTR pszIcon;
        _plib->GetIcon(&pszIcon);
        Output(L"Icon path: %s\n", pszIcon ? pszIcon : L"<none>");
        CoTaskMemFree(pszIcon);

        // Call the super-class to enumerate and display the locations in the library.
        Output(L"\n");
        return __super::v_ExecuteLibCommand();
    }
};


// CSetAttributeCommand - Modifies the attributes of the library.
//
// Several attributes of the library can be modified, including the option flags, folder type, and icon path.
// This command provides options for setting each of these attributes.

class CSetAttributeCommand :
    public CShlibCommandBase
{
public:
    CSetAttributeCommand() :
        CShlibCommandBase(L"setattrib", L"Modifies the attributes of the library.", FALSE),
        _lofSet(LOF_DEFAULT),
        _lofClear(LOF_DEFAULT),
        _ftid(GUID_NULL),
        _pszIconPath(NULL)
    {
        // Initialize option handlers for setting and clearing flags.
        ARGENTRY<LIBRARYOPTIONFLAGS> c_rgOptions[] =
        {
            { L"pinned", LOF_PINNEDTONAVPANE, L"Indicates that the library should be shown in the navigation pane in Explorer." },
            { L"all",    LOF_MASK_ALL,        L"Sets/clears all option flags on the library." },
        };

        AddEnumOptionHandler(L"setflag", L"option flag", L"Sets the specified option flag on the library.",
                             &CSetAttributeCommand::SetFlag, c_rgOptions, ARRAYSIZE(c_rgOptions));

        AddEnumOptionHandler(L"clearflag", L"option flag", L"Clears the specified option flag from the library.",
                             &CSetAttributeCommand::ClearFlag, c_rgOptions, ARRAYSIZE(c_rgOptions));

        // Initialize an option handler for specifying the folder type.
        ARGENTRY<FOLDERTYPEID> c_rgFolderTypes[] =
        {
            { L"documents", FOLDERTYPEID_Documents, L"Specifies that the library primarily contains document content." },
            { L"pictures",  FOLDERTYPEID_Pictures,  L"Specifies that the library primarily contains pictures content." },
            { L"music",     FOLDERTYPEID_Music,     L"Specifies that the library primarily contains music content." },
            { L"videos",    FOLDERTYPEID_Videos,    L"Specifies that the library primarily contains video content." },
            { L"none",      GUID_NULL,              L"Clears the folder type of the library." },
        };

        AddEnumOptionHandler(L"foldertype", L"folder type", L"Sets the folder type of the library.",
                             &CSetAttributeCommand::SetFolderType, c_rgFolderTypes, ARRAYSIZE(c_rgFolderTypes));

        // Initialize option handler for specifying the icon.
        AddStringOptionHandler(L"icon", L"Specifies the path to the icon to display for the library.", &CSetAttributeCommand::SetIconPath);
    }

    void v_PrintInstructions() const
    {
        Output(L"The form of the icon path is \"C:\\Path\\To\\Some\\Module.dll,-123\" where the icon is resource ID 123 in Module.dll.  Alternatively, the path to a .ico file can be specified.  Relative paths will be evaluated against the %PATH% environment variable.\n");
    }

    HRESULT SetFlag(LIBRARYOPTIONFLAGS lofSet)
    {
        _lofSet |= lofSet;
        return S_OK;
    }

    HRESULT ClearFlag(LIBRARYOPTIONFLAGS lofClear)
    {
        _lofClear |= lofClear;
        return S_OK;
    }

    HRESULT SetFolderType(FOLDERTYPEID ftid)
    {
        _ftid = ftid;
        return S_OK;
    }

    HRESULT SetIconPath(PCWSTR pszIconPath)
    {
        return SHStrDupW(pszIconPath, &_pszIconPath);
    }

    // Modify any attributes that have been specified.
    HRESULT v_ExecuteLibCommand()
    {
        HRESULT hr = S_OK;

        if (_lofSet != LOF_DEFAULT)
        {
            hr = _plib->SetOptions(_lofSet, _lofSet);
        }

        if (SUCCEEDED(hr) && _lofClear != LOF_DEFAULT)
        {
            hr = _plib->SetOptions(_lofClear, LOF_DEFAULT);
        }

        if (SUCCEEDED(hr) && _ftid != GUID_NULL)
        {
            hr = _plib->SetFolderType(_ftid);
        }

        if (SUCCEEDED(hr) && _pszIconPath)
        {
            hr = _plib->SetIcon(_pszIconPath);
        }

        if (FAILED(hr))
        {
            RuntimeError(L"Error %#08x modifying library attributes.\n", hr);
        }
        return hr;
    }

    ~CSetAttributeCommand()
    {
        CoTaskMemFree(_pszIconPath);
    }

private:
    LIBRARYOPTIONFLAGS _lofSet;
    LIBRARYOPTIONFLAGS _lofClear;
    FOLDERTYPEID _ftid;
    PWSTR _pszIconPath;
};


// CFolderCommandBase - Base class for commands that operate on folders within a library.
//
// This class handles interpreting the second argument as the path to a folder to operate on with respect to the library.
// The specified folder is provided to derived commands via the protected member variables _psiFolder and/or _pszFolderPath.

class CFolderCommandBase :
    public CShlibCommandBase
{
public:
    CFolderCommandBase(PCWSTR pszName, PCWSTR pszDescription) :
        CShlibCommandBase(pszName, pszDescription, FALSE),
        _psiFolder(NULL)
    {}

    // Interpret the next argument as a path, and obtain the IShellItem for it to be consumed by the derived class.
    HRESULT v_ProcessLibArguments(PCWSTR *ppszArgs, int cArgs)
    {
        PCWSTR pszFolderPath = CONSUME_NEXT_ARG(ppszArgs, cArgs);
        HRESULT hr = pszFolderPath ? S_OK : E_INVALIDARG;
        if (SUCCEEDED(hr))
        {
            hr = SHStrDupW(pszFolderPath, &_pszFolderPath);
            if (SUCCEEDED(hr))
            {
                hr = SHCreateItemFromParsingName(pszFolderPath, NULL, IID_PPV_ARGS(&_psiFolder));
            }
        }
        else
        {
            ParseError(L"Missing folder path argument.\n");
        }
        // On success, pass any remaining arguments on to the derived class.
        return SUCCEEDED(hr) ? v_ProcessFolderArguments(ppszArgs, cArgs) : hr;
    }

    void v_PrintLibInstructions() const
    {
        Output(L"Specify the path of the folder to operate on after <Library>.\n");
    }

    ~CFolderCommandBase()
    {
        if (_psiFolder)
        {
            _psiFolder->Release();
        }
        CoTaskMemFree(_pszFolderPath);
    }

protected:
    virtual HRESULT v_ProcessFolderArguments(PCWSTR*, int) { return S_OK; }

    IShellItem *_psiFolder;
    PWSTR _pszFolderPath;
};


// CAddCommand - Adds the specified folder to the library.
//
// This command uses the AddFolder method of IShellLibrary to add a folder to the library.  The folder is specified
// as the second argument, as per the usage of CFolderCommandBase.

class CAddCommand :
    public CFolderCommandBase
{
public:
    CAddCommand() :
        CFolderCommandBase(L"add", L"Adds the specified folder to the specified library.")
    {}

    HRESULT v_ExecuteLibCommand()
    {
        HRESULT hr = _plib->AddFolder(_psiFolder);
        if (FAILED(hr))
        {
            RuntimeError(L"Error %#08x adding folder %s to the library.\n", hr, _pszFolderPath);
        }
        return hr;
    }
};


// CRemoveCommand - Removes the specified folder from the library.
//
// This command uses the RemoveFolder method of IShellLibrary to remove a folder from the library.  The folder is specified
// as the second argument, as per the usage of CFolderCommandBase.

class CRemoveCommand :
    public CFolderCommandBase
{
public:
    CRemoveCommand() :
        CFolderCommandBase(L"remove", L"Removes the specified folder from the library.")
    {}

    HRESULT v_ExecuteLibCommand()
    {
        HRESULT hr = _plib->RemoveFolder(_psiFolder);
        if (FAILED(hr))
        {
            RuntimeError(L"Error %#08x removing folder %s from the library.\n", hr, _pszFolderPath);
        }
        return hr;
    }
};


// CSetDefaultSaveFolderCommand - Sets the default save location of the library.
//
// When a user attempts to save an item into a library, the library needs to know which location to actually save the data into.
// The default save location is one of the folders included in the library that has been designated for this purpose.

class CSetDefaultSaveFolderCommand :
    public CFolderCommandBase
{
public:
    CSetDefaultSaveFolderCommand() :
        CFolderCommandBase(L"setsaveloc", L"Sets the default save location of the library."),
        _dsft(DSFT_DETECT)
    {
        ARGENTRY<DEFAULTSAVEFOLDERTYPE> c_rgScopes[] =
        {
            { L"detect",  DSFT_DETECT,  L"Detect which save location to set based on the current user and the owner of the library. (default)" },
            { L"private", DSFT_PRIVATE, L"Set the private default save location." },
            { L"public",  DSFT_PUBLIC,  L"Set the public default save location." },
        };

        AddEnumOptionHandler(L"scope", L"scope", L"Specifies which default save location to set (public or private).",
                             &CSetDefaultSaveFolderCommand::SetScope, c_rgScopes, ARRAYSIZE(c_rgScopes));
    }

    void v_PrintInstructions() const
    {
        Output(L"The default save location must be one of the folders already included in the library.  This is the folder that Explorer and other applications will use when saving items into the library.  Since libraries can be shared with other users, each library has a \"private\" and a \"public\" save location, which take effect for the owner of the library, and other users, respectively.\n");
    }

    HRESULT SetScope(DEFAULTSAVEFOLDERTYPE dsft)
    {
        _dsft = dsft;
        return S_OK;
    }

    HRESULT v_ExecuteLibCommand()
    {
        HRESULT hr = _plib->SetDefaultSaveFolder(_dsft, _psiFolder);
        if (FAILED(hr))
        {
            if (hr == __HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
            {
                RuntimeError(L"The specified folder is not included in the library.\n");
            }
            else
            {
                RuntimeError(L"Error %#08x setting default save location to %s.\n", hr, _pszFolderPath);
            }
        }
        return hr;
    }

private:
    DEFAULTSAVEFOLDERTYPE _dsft;
};


// CResolveCommand - Resolves the specified folder in the library.
//
// This command uses the ResolveFolder method of IShellLibrary to resolve a folder in the library.  The folder is specified
// as the second argument, as per the usage of CFolderCommandBase.  Libraries store references to their constituent locations
// as Shell Links (shortcuts), and when the target is moved or renamed, the shortcut must be resolved to point to the new target.
// This process is normally fast, but can be time consuming, so IShellLibrary::ResolveFolder includes a timeout parameter to
// specify the maximum number of milliseconds to wait before aborting the resolution.

class CResolveCommand :
    public CFolderCommandBase
{
public:
    CResolveCommand() :
        CFolderCommandBase(L"resolve", L"Resolves the specified folder in the library."),
        _dwTimeout(1000)
    {
        AddStringOptionHandler(L"timeout", L"Specifies the timeout value in milliseconds (defaults to 1000).", &CResolveCommand::SetTimeout);
    }

    HRESULT SetTimeout(PCWSTR pszTimeout)
    {
        // Convert the specified timeout to an integer.
        PWSTR pszEnd;
        _dwTimeout = wcstol(pszTimeout, &pszEnd, 0);
        HRESULT hr = (*pszEnd ? E_INVALIDARG : S_OK);
        if (FAILED(hr))
        {
            ParseError(L"Invalid timeout: %s\n", pszTimeout);
        }
        return hr;
    }

    HRESULT v_ExecuteLibCommand()
    {
        // Attempt to resolve the folder.  An IShellItem representing the updated target location is returned.
        IShellItem *psiResolved;
        HRESULT hr = _plib->ResolveFolder(_psiFolder, _dwTimeout, IID_PPV_ARGS(&psiResolved));
        if (SUCCEEDED(hr))
        {
            PWSTR pszResolvedPath;
            psiResolved->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszResolvedPath);
            Output(L"Resolved folder path %s to: %s\n", _pszFolderPath, pszResolvedPath);
            CoTaskMemFree(pszResolvedPath);
            psiResolved->Release();
        }
        else
        {
            RuntimeError(L"Error %#08x resolving folder %s from the library.\n", hr, _pszFolderPath);
        }
        return hr;
    }

private:
    DWORD _dwTimeout;
};


// CResolveAllCommand - Resolves all of the locations in the library at once.
//
// This command uses the SHResolveLibrary API to execute a 'bulk resolve' of all of the locations in the library,
// rather than iterating over each one returned from IShellLibrary::GetFolders and calling ResolveFolder.

class CResolveAllCommand :
    public CShlibCommandBase
{
public:
    CResolveAllCommand() :
        CShlibCommandBase(L"resolveall", L"Resolves all locations in the library in bulk.")
    {}

    // Since we just need the IShellItem and not the IShellLibrary provided by CShlibCommandBase, override v_ExecuteCommand directly.
    HRESULT v_ExecuteCommand()
    {
        HRESULT hr = SHResolveLibrary(_psiLibrary);
        if (SUCCEEDED(hr))
        {
            Output(L"Resolution succeeded.\n");
        }
        else
        {
            RuntimeError(L"Error %#08x resolving the library.\n", hr);
        }
        return hr;
    }

    // This is not used since CShlibCommandBase::v_ExecuteCommand has been overridden directly.
    HRESULT v_ExecuteLibCommand() { return E_NOTIMPL; }
};


// CManageCommand - Displays the Manage Library Dialog for the library.
//
// The SHShowManageLibraryUI API provides an entry-point for an application to display UI for the user to view and modify
// the settings of the library.  This command exposes that UI, and provides options to specify the title and instructions
// text, as well as behavior options.

class CManageCommand :
    public CShlibCommandBase
{
public:
    CManageCommand() :
        CShlibCommandBase(L"manage", L"Displays the Manage Library Dialog for the library."),
        _pszTitle(NULL),
        _pszInstructions(NULL),
        _lmdOptions(LMD_DEFAULT)
    {
        // Options for the Manage Library Dialog UI.
        ARGENTRY<LIBRARYMANAGEDIALOGOPTIONS> c_rgLibManageDialogOptions[] =
        {
            { L"default",   LMD_DEFAULT,                          L"Prevent un-indexable network locations from being added." },
            { L"",          LMD_DEFAULT,                          L"Synonym for 'default'" },
            { L"allowslow", LMD_ALLOWUNINDEXABLENETWORKLOCATIONS, L"Allow un-indexable network locations to be added." },
        };

        AddStringOptionHandler(L"title",                     L"Sets the title of the dialog to ARG.", &CManageCommand::SetTitle);
        AddStringOptionHandler(L"instructions",              L"Sets the instructions text of the dialog to ARG.", &CManageCommand::SetInstructions);
        AddEnumOptionHandler  (L"options", L"dialog option", L"Specifies options for controlling the dialog's behavior.",
                               &CManageCommand::SetOptions, c_rgLibManageDialogOptions, ARRAYSIZE(c_rgLibManageDialogOptions));
    }

    HRESULT SetTitle(PCWSTR pszTitle)
    {
        return SHStrDup(pszTitle, &_pszTitle);
    }

    HRESULT SetInstructions(PCWSTR pszInstructions)
    {
        return SHStrDup(pszInstructions, &_pszInstructions);
    }

    HRESULT SetOptions(LIBRARYMANAGEDIALOGOPTIONS lmdOptions)
    {
        _lmdOptions = lmdOptions;
        return S_OK;
    }

    // Since we just need the IShellItem and not the IShellLibrary provided by CShlibCommandBase, override v_ExecuteCommand directly.
    HRESULT v_ExecuteCommand()
    {
        HRESULT hr = SHShowManageLibraryUI(_psiLibrary, NULL, _pszTitle, _pszInstructions, _lmdOptions);
        if (FAILED(hr))
        {
            RuntimeError(L"Error %#08x returned from Manage Library Dialog.\n", hr);
        }
        return hr;
    }

    // This is not used since CShlibCommandBase::v_ExecuteCommand has been overridden directly.
    HRESULT v_ExecuteLibCommand() { return E_NOTIMPL; }

    ~CManageCommand()
    {
        CoTaskMemFree(_pszTitle);
        CoTaskMemFree(_pszInstructions);
    }

private:
    PWSTR _pszTitle;
    PWSTR _pszInstructions;
    LIBRARYMANAGEDIALOGOPTIONS _lmdOptions;
};
