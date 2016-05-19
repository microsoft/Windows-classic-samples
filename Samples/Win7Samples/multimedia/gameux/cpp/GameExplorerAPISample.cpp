// TasksTest.cpp : Defines the entry point for the console application.
//

#pragma comment(lib,"wbemuuid")

#define _WIN32_DCOM

#include <stdio.h>
#include <gameux.h>
#include <wbemidl.h>
#include <atlbase.h>

#define WMI_GAME_NAME           L"Name"
#define WMI_GAME_ID             L"GameID"
#define WMI_GAME_INSTANCE_ID    L"InstanceID"
#define WMI_GAME_INSTALL_SCOPE  L"InstallScope"

#define WMI_GAME_NAMESPACE      L"ROOT\\CIMV2\\Applications\\Games"

#define WMI_QUERY_TYPE           L"WQL"
#define WMI_GAME_QUERY_ALL_GAMES L"SELECT * FROM GAME"

#define GUID_BUFFER_LENGTH 39
#define ARGUMENT_COUNT 3


HRESULT InstallGame(__in_z PCWSTR pszGDFBinaryPath,
                    __in_z PCWSTR pszGameInstallPath, 
                    GAME_INSTALL_SCOPE gameInstallScope, 
                    __out GUID *pguidInstanceId);

HRESULT RemoveGame(__in GUID guidInstanceId);
HRESULT UpdateGame(__in GUID guidInstanceId);
HRESULT ListGames();

int __cdecl wmain(int argc, __in_ecount(argc) wchar_t *argv[], __in wchar_t *)
{
    if(argc < ARGUMENT_COUNT)
    {
        wprintf(L"Usage: GameExplorerAPISample.exe <pathToGDF> <gameInstallDirectory>\n");
        return -1;
    }

    wprintf (L"Begin sample.\n");
    // For the WMI portions we need a multithreaded COM.

    HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(SUCCEEDED(hr))
    {

        hr = CoInitializeSecurity(NULL,
            -1,
            NULL,
            NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_NONE,
            NULL);

        if(SUCCEEDED(hr))
        {
            // Important! This must be a GUID_NULL for AddGame to create a new instance GUID.
            GUID guidInstanceId = GUID_NULL;


            wprintf(L"Adding game for the current user.\n");
            wprintf(L"Change the call to InstallGame to install for all users.\n");
            wprintf(L"(Installing for all users requires Administrator.\n");

            hr = InstallGame(argv[1],argv[2],GIS_CURRENT_USER,&guidInstanceId);
            // (The following call Requires Administrator privleges)
            // hr = InstallGame(argv[1],argv[2],GIS_ALL_USERS,&guidInstanceId);

            if(SUCCEEDED(hr))
            {
                wprintf(L"Added game for the current user only. Now listing games.\n");
                hr = ListGames();

                if(SUCCEEDED(hr))
                {
                    wprintf(L"Press enter to remove game...");
                    wscanf_s(L"%*c");
                    wprintf(L"Removing game\n");
                    
                    hr = RemoveGame(guidInstanceId);

                    if(SUCCEEDED(hr))
                    {
                        wprintf(L"Game removed.\n");
                    }
                    else
                    {
                        wprintf(L"Failure removing game. Error 0x%x",hr);
                    }
                }
            }
            else
            {
                wprintf(L"Failure adding game. Error 0x%x\n",hr);
            }
        }
        else
        {
            wprintf(L"Failed to initialize COM Security. 0x%x",hr);
        }
    }
    else
    {
        wprintf(L"Failed to initialize COM. 0x%x\n",hr);
    }

    CoUninitialize();
    return hr;
}

// UpdateGame is used to force the Parental Controls and Game Explorer to refresh its information about a Game.
// UpdateGame is usually only needed if a game's Game Definition File has changed.
// guidInstanceId is the game's instance ID.
HRESULT UpdateGame(__in GUID guidInstanceId)
{
    CComPtr<IGameExplorer> spiGameExplorer;
    HRESULT hr = spiGameExplorer.CoCreateInstance(_uuidof(GameExplorer));

    if(SUCCEEDED(hr))
    {
        if(&guidInstanceId != NULL && guidInstanceId != GUID_NULL)
        {
            hr = spiGameExplorer->UpdateGame(guidInstanceId);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

// RemoveGame removes a game from the Game Explorer. It takes the Instance ID used in the AddGame call.
HRESULT RemoveGame(__in GUID guidInstanceId)
{
    CComPtr<IGameExplorer> spiGameExplorer;
    HRESULT hr = spiGameExplorer.CoCreateInstance(_uuidof(GameExplorer));

    if(SUCCEEDED(hr))
    {
        if(&guidInstanceId != NULL && guidInstanceId != GUID_NULL)
        {
            hr = spiGameExplorer->RemoveGame(guidInstanceId);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

// pszGDFBinaryPath is the path to the DLL or EXE which contains the embedded Game Definition File
// pszGameInstallPath is the game's installation directory (i.e. C:\Program Files\CompanyName\GameName)
// gameInstallScope can be either GIS_ALL_USERS or GIS_CURRENT_USER
//
// pguidInstanceID - Game Instance ID, unique per game installation.
//                   If the GUID passed in GUID_NULL, we will generate a new random Instance ID and return it as an OUT parameter.
//                   If an actual GUID is passed in, the AddGame() call will use that GUID as the instance ID as an IN parameter.
//                   If the instance ID passed in is already taken, this call will result in an error.
//
// InstallGame will return S_FALSE if the game has already been added 
// to the Game Explorer for this specific GameID and Install Path

HRESULT InstallGame(__in_z PCWSTR pszGDFBinaryPath,
                    __in_z PCWSTR pszGameInstallPath, 
                    GAME_INSTALL_SCOPE gameInstallScope, 
                    __inout GUID *pguidInstanceId)
{
    if(NULL == pszGDFBinaryPath)
    {
        wprintf(L"Error: GDF Path must not be null.\n");
        return E_INVALIDARG;
    }

    if(NULL == pszGameInstallPath)
    {
        wprintf(L"Error: Game Install Path must not be null.\n");
        return E_INVALIDARG;
    }

    CComPtr<IGameExplorer> spiGameExplorer;
    HRESULT hr = spiGameExplorer.CoCreateInstance(__uuidof(GameExplorer));

    if(SUCCEEDED(hr))
    {
        BSTR bstrGDFBinaryPath = SysAllocString(pszGDFBinaryPath);
        if(NULL != bstrGDFBinaryPath)
        {
            BSTR bstrGameInstallPath = SysAllocString(pszGameInstallPath);
            if(NULL != bstrGameInstallPath)
            {
                wprintf(L"********************\n");
                wprintf(L"Attempting to Add Game:\n\n");
                wprintf(L"Game Install Directory: %s\n",bstrGameInstallPath);
                wprintf(L"Game GDF Path: %s\n",bstrGDFBinaryPath);
                wprintf(L"Game Installl Scope: %d\n",gameInstallScope);
                wprintf(L"********************\n\n");

                hr = spiGameExplorer->AddGame(bstrGDFBinaryPath, bstrGameInstallPath, gameInstallScope,pguidInstanceId);

                if(SUCCEEDED(hr))
                {
                    WCHAR szInstanceId[GUID_BUFFER_LENGTH];
                    if (StringFromGUID2(*pguidInstanceId, szInstanceId, GUID_BUFFER_LENGTH) == GUID_BUFFER_LENGTH)
                    {
                        wprintf(L"Game Instance ID: %s\n",szInstanceId);
                    }

                    // S_FALSE occurs if the game is already installed.
                    if(S_FALSE == hr)
                    {
                        wprintf(L"********************\n");
                        wprintf(L"Warning. Game Already Installed:\n\n");
                        wprintf(L"Game Install Directory: %s\n",bstrGameInstallPath);
                        wprintf(L"Game GDF Path: %s\n",bstrGDFBinaryPath);
                        wprintf(L"Game Installl Scope: %d\n",gameInstallScope);
                        wprintf(L"********************\n\n");
                    }
                }
                SysFreeString(bstrGameInstallPath);
            }
            else
            {
                wprintf(L"Error allocating memory for Game Install Path BSTR\n");
                hr=E_OUTOFMEMORY;
            }
            SysFreeString(bstrGDFBinaryPath);
        }
        else
        {
            wprintf(L"Error allocating memory for GDF Binary Path BSTR");
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        wprintf(L"Error initializing the Game Explorer COM Object.\n");
    }
    return hr;
}

//ListGames uses WMI (Windows Management Instrumentation) to enumerate all of the games installed in the Game Explorer
//Consult MSDN for more information on WMI, or the Game Explorer documentation in the Platform and DirectX SDKs for more
//information on the Game Explorer WMI provider.
HRESULT ListGames()
{

    CComPtr<IWbemLocator> spiWbemLocator;
    CComPtr<IWbemServices> spiWbemServices;
    CComPtr<IEnumWbemClassObject> spiWbemEnumerator;

    HRESULT hr = spiWbemLocator.CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER);

    if(SUCCEEDED(hr))
    {
        BSTR bstrGameNamespace = SysAllocString(WMI_GAME_NAMESPACE);

        if(NULL != bstrGameNamespace)
        {
            hr = spiWbemLocator->ConnectServer(bstrGameNamespace,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &spiWbemServices);

            if(SUCCEEDED(hr))
            {
                hr = CoSetProxyBlanket(spiWbemServices,
                    RPC_C_AUTHN_WINNT,
                    RPC_C_AUTHZ_NONE,
                    NULL,
                    RPC_C_AUTHN_LEVEL_CALL,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,
                    EOAC_NONE);

                if(SUCCEEDED(hr))
                {
                    BSTR bstrQueryType = SysAllocString(WMI_QUERY_TYPE);
                    if(NULL != bstrQueryType)
                    {
                        BSTR bstrQuery = SysAllocString(WMI_GAME_QUERY_ALL_GAMES);
                        if(NULL != bstrQuery)
                        {
                            hr = spiWbemServices->ExecQuery(bstrQueryType,
                                bstrQuery,
                                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                NULL,
                                &spiWbemEnumerator);


                            if(SUCCEEDED(hr))
                            {

                                ULONG uReturn = 0;

                                while(spiWbemEnumerator)
                                {
                                    CComPtr<IWbemClassObject> spiWbemClassObject;
                                    hr = spiWbemEnumerator->Next(WBEM_INFINITE,1,&spiWbemClassObject,&uReturn);
                                    if(uReturn <= 0)
                                    {
                                        break;
                                    }

                                    if(SUCCEEDED(hr))
                                    {

                                        VARIANT vtGameName;
                                        VARIANT vtGameInstanceID;
                                        VARIANT vtGameInstallScope;

                                        VariantInit(&vtGameName);
                                        VariantInit(&vtGameInstanceID);
                                        VariantInit(&vtGameInstallScope);

                                        hr = spiWbemClassObject->Get(WMI_GAME_NAME,
                                            NULL,
                                            &vtGameName,
                                            NULL,
                                            NULL);

                                        if(SUCCEEDED(hr))
                                        {    
                                            hr = spiWbemClassObject->Get(WMI_GAME_INSTANCE_ID,
                                                NULL,
                                                &vtGameInstanceID,
                                                NULL,
                                                NULL);

                                            if(SUCCEEDED(hr))
                                            {
                                                hr = spiWbemClassObject->Get(WMI_GAME_INSTALL_SCOPE,
                                                    NULL,
                                                    &vtGameInstallScope,
                                                    NULL,
                                                    NULL);

                                                if(SUCCEEDED(hr))
                                                {
                                                    wprintf(L"Game: %s, %s, %u\n",vtGameName.bstrVal,vtGameInstanceID.bstrVal, vtGameInstallScope.uintVal);
                                                }
                                                else
                                                {
                                                    wprintf(L"Failure getting game install scope for this game.\n");
                                                }
                                            }
                                            else
                                            {
                                                wprintf(L"Failure getting game instance ID for this game.\n");
                                            }
                                        }
                                        else
                                        {
                                            wprintf(L"Failure getting game name for this game.\n");
                                        }

                                        VariantClear(&vtGameName);
                                        VariantClear(&vtGameInstanceID);
                                        VariantClear(&vtGameInstallScope);
                                    }
                                    else
                                    {
                                        wprintf(L"Failure in Enumerator: Error 0x%x\n",hr);
                                    }

                                }
                            }
                            else
                            {
                                wprintf(L"Error: Could not allocate BSTR for Query.\n");
                                hr=E_OUTOFMEMORY;
                            }

                            SysFreeString(bstrQuery);
                        }
                        else
                        {
                            wprintf(L"Error: Could not allocate BSTR for Query.\n");
                            hr = E_OUTOFMEMORY;
                        }

                        SysFreeString(bstrQueryType);
                    }
                    else
                    {
                        wprintf(L"Error: Could not allocate BSTR for QueryType.\n");
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    wprintf(L"Could not set proxy blanket.  Error Code 0x%x\n",hr);
                }

                SysFreeString(bstrGameNamespace);
            }
            else
            {
                wprintf(L"Could not connect to Game WMI namespace.  Error Code = 0x%x\n",hr);
            }

        }
        else
        {
            wprintf(L"Error allocating namespace BSTR.\n");
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        wprintf(L"Error CoCreating IWbemLocator. 0x%x\n",hr);
    }
    return hr;
}