// This file is for code snippets that are variations on the code in main.cpp

#include <windows.h>
#include <shobjidl.h> 
#include <atlbase.h> // Contains the declaration of CComPtr.

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | 
        COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CComPtr<IFileOpenDialog> pFileOpen;

        // Create the FileOpenDialog object.
        hr = pFileOpen.CoCreateInstance(__uuidof(FileOpenDialog));
        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                CComPtr<IShellItem> pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
                        CoTaskMemFree(pszFilePath);
                    }
                }

                // pItem goes out of scope.
            }

            // pFileOpen goes out of scope.
        }
        CoUninitialize();
    }
    return 0;
}


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


void UseSafeRelease()
{
    IFileOpenDialog *pFileOpen = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, 
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
    if (SUCCEEDED(hr))
    {
        // Use the object.
    }
    SafeRelease(&pFileOpen);
}


// The following code examples demonstrate additional variations discussed in the 
// documentration.

namespace NestedIfs
{
	HRESULT ShowDialog()
	{
		IFileOpenDialog *pFileOpen;

		HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, 
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
		if (SUCCEEDED(hr))
		{
			hr = pFileOpen->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					// Use pItem (not shown). 
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		return hr;
	}
};

namespace CascadingIfs
{

	HRESULT ShowDialog()
	{
		IFileOpenDialog *pFileOpen = NULL;
		IShellItem *pItem = NULL;

		HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, 
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			hr = pFileOpen->Show(NULL);
		}
		if (SUCCEEDED(hr))
		{
			hr = pFileOpen->GetResult(&pItem);
		}
		if (SUCCEEDED(hr))
		{
			// Use pItem (not shown).
		}

		// Clean up.
		SafeRelease(&pItem);
		SafeRelease(&pFileOpen);
		return hr;
	}
};

namespace JumpOnFail
{
	HRESULT ShowDialog()
	{
		IFileOpenDialog *pFileOpen = NULL;
		IShellItem *pItem = NULL;

		HRESULT hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, 
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen));
		if (FAILED(hr))
		{
			goto done;
		}

		hr = pFileOpen->Show(NULL);
		if (FAILED(hr))
		{
			goto done;
		}

		hr = pFileOpen->GetResult(&pItem);
		if (FAILED(hr))
		{
			goto done;
		}

		// Use pItem (not shown).

	done:
		// Clean up.
		SafeRelease(&pItem);
		SafeRelease(&pFileOpen);
		return hr;
	}
};

namespace ThrowOnFail
{
	#include <comdef.h>  // Declares _com_error

	inline void throw_if_fail(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw _com_error(hr);
		}
	}

	void ShowDialog()
	{
		try
		{
			CComPtr<IFileOpenDialog> pFileOpen;
			throw_if_fail(CoCreateInstance(__uuidof(FileOpenDialog), NULL, 
				CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOpen)));

			throw_if_fail(pFileOpen->Show(NULL));

			CComPtr<IShellItem> pItem;
			throw_if_fail(pFileOpen->GetResult(&pItem));

			// Use pItem (not shown).
		}
		catch (_com_error err)
		{
			// Handle error.
		}
	}
};