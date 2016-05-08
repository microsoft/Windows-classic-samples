#pragma once



// Class Factory implementation


/* 

// Example of usage:


// g_ClassFactories: Array of class factory data.
// Defines a look-up table of CLSIDs and corresponding creation functions.

ClassFactoryData g_ClassFactories[] =
{
    {   &CLSID_MyObject, MyObject::CreateInstance }
};

DWORD g_numClassFactories = ARRAY_SIZE(g_ClassFactories);

// Defines the static ClassFactory::m_serverLocks member. 
DEFINE_CLASSFACTORY_SERVER_LOCK;

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv)
{
    ClassFactory *pFactory = NULL;

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE; // Default to failure

    // Find an entry in our look-up table for the specified CLSID.
    for (DWORD index = 0; index < g_numClassFactories; index++)
    {
        if (*g_ClassFactories[index].pclsid == clsid)
        {
            // Found an entry. Create a new class factory object.
            pFactory = new ClassFactory(g_ClassFactories[index].pfnCreate);
            if (pFactory)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pFactory->QueryInterface(riid, ppv);
    }
    SAFE_RELEASE(pFactory);

    return hr;
}

STDAPI DllCanUnloadNow()
{
    if (!ClassFactory::IsLocked())
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

*/

namespace MediaFoundationSamples
{
    // Function pointer for creating COM objects. (Used by the class factory.)
    typedef HRESULT (*CreateInstanceFn)(IUnknown *pUnkOuter, REFIID iid, void **ppv);

    // Structure to associate CLSID with object creation function.
    struct ClassFactoryData
    {
        const GUID          *pclsid;
        CreateInstanceFn    pfnCreate;
    };


    // ClassFactory:
    // Implements a class factory for COM objects.

    class ClassFactory : public IClassFactory
    {
    private:
        volatile long           m_refCount;     // Reference count.
        static volatile long    m_serverLocks;  // Number of server locks

        CreateInstanceFn        m_pfnCreation;  // Function to create an instance of the object.

    public:

        ClassFactory(CreateInstanceFn pfnCreation) : m_pfnCreation(pfnCreation), m_refCount(1)
        {
        }

        static bool IsLocked()
        {
            return (m_serverLocks != 0);
        }

        STDMETHODIMP_(ULONG) AddRef()
        {
            return InterlockedIncrement(&m_refCount);
        }
        STDMETHODIMP_(ULONG) Release()
        {
            assert(m_refCount >= 0);
            ULONG uCount = InterlockedDecrement(&m_refCount);
            if (uCount == 0)
            {
                delete this;
            }
            // Return the temporary variable, not the member
            // variable, for thread safety.
            return uCount;
        }
        // IUnknown methods
        STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
        {
            if (NULL == ppv)
            {
                return E_POINTER;
            }
            else if (riid == __uuidof(IUnknown))
            {
                *ppv = static_cast<IUnknown*>(this);
            }
            else if (riid == __uuidof(IClassFactory))
            {
                *ppv = static_cast<IClassFactory*>(this);
            }
            else 
            {
                *ppv = NULL;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
        {
            // If the caller is aggregating the object, the caller may only request
            // IUknown. (See MSDN documenation for IClassFactory::CreateInstance.)
            if (pUnkOuter != NULL)
            {
                if (riid != __uuidof(IUnknown))
                {
                    return E_NOINTERFACE;
                }
            }

            return m_pfnCreation(pUnkOuter, riid, ppv);
        }

        STDMETHODIMP LockServer(BOOL lock)
        {   
            if (lock)
            {
                LockServer();
            }
            else
            {
                UnlockServer();
            }
            return S_OK;
        }


        // Static methods to lock and unlock the the server.
        static void LockServer()
        {
            InterlockedIncrement(&m_serverLocks);
        }

        static void UnlockServer()
        {
            InterlockedDecrement(&m_serverLocks);
        }

    };

    // BaseObjects
    // All COM objects that are implemented in the server (DLL) must derive from BaseObject
    // so that the server is not unlocked while objects are still active.
    class BaseObject
    {
    public:
        BaseObject() 
        {
            ClassFactory::LockServer();
        }
        virtual ~BaseObject()
        {
            ClassFactory::UnlockServer();
        }
    };

    // RefCountedObject
    // You can use this when implementing IUnknown or any object that uses reference counting.
    class RefCountedObject
    {
    protected:
        volatile long   m_refCount;

    public:
        RefCountedObject() : m_refCount(1) {}
        virtual ~RefCountedObject()
        {
            assert(m_refCount == 0);
        }

        ULONG AddRef()
        {
            return InterlockedIncrement(&m_refCount);
        }
        ULONG Release()
        {
            assert(m_refCount > 0);
            ULONG uCount = InterlockedDecrement(&m_refCount);
            if (uCount == 0)
            {
                delete this;
            }
            return uCount;
        }
    };


#define DEFINE_CLASSFACTORY_SERVER_LOCK  volatile long ClassFactory::m_serverLocks = 0;

}; // namespace MediaFoundationSamples