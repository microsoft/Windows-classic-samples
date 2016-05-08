#ifndef REGISTRATION_H
#define REGISTRATION_H

#define CLSIDSTR_MS_SHVUI L"{230b2a03-bbb3-4d50-839b-74f095e2b53e}"
// This AppID has been created by 'regsvr32 SdkShv.dll' earlier, so we simply use it.
#define STR_APPID_SDK_SHV_CONFIG L"{AD310CB9-8B4B-46ae-93BB-2D3C4DBE35DD}"
#define CLSID_MS_SHVUI_FRIENDLY_NAME L"Sample SHV Configuration UI"
#define MAX_LENGTH 256

#define REGCLSID  L"CLSID"
#define REGAPPID  L"AppID"
#define INPROCSERVER32  L"InprocServer32"
#define THREADINGMODEL  L"ThreadingModel"
#define FREETHREADING  L"Free"
#define LOCALSERVER32  L"LocalServer32"

STDMETHODIMP
ShvUIRegisterServer();

STDMETHODIMP
ShvUIUnRegisterServer();

#endif //REGISTRATION_H