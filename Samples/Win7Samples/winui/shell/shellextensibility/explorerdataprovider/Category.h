/**************************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

(c) Microsoft Corporation. All Rights Reserved.
**************************************************************************/

#pragma once

class CFolderViewImplCategoryProvider : public ICategoryProvider
{
public:
    CFolderViewImplCategoryProvider(IShellFolder2 *psf);

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICategoryProvider methods.
    IFACEMETHODIMP CanCategorizeOnSCID(const PROPERTYKEY *key);
    IFACEMETHODIMP GetDefaultCategory(GUID* pguid, PROPERTYKEY *key);
    IFACEMETHODIMP GetCategoryForSCID(const PROPERTYKEY *key, GUID *pguid);
    IFACEMETHODIMP EnumCategories(IEnumGUID** penum);
    IFACEMETHODIMP GetCategoryName(const GUID* pguid, PWSTR pszName, UINT cch);
    IFACEMETHODIMP CreateCategory(const GUID* pguid, REFIID riid, void** ppv);

private:
    ~CFolderViewImplCategoryProvider();
    long            m_cRef;
    IShellFolder2  *m_psf;
};

class CFolderViewImplEnumGUID : public IEnumGUID
{
public:
    CFolderViewImplEnumGUID();

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IEnumGUID
    IFACEMETHODIMP Next(ULONG celt, GUID *rgelt, ULONG *pceltFetched);
    IFACEMETHODIMP Skip(ULONG);
    IFACEMETHODIMP Reset();
    IFACEMETHODIMP Clone(IEnumGUID**);

private:
    ~CFolderViewImplEnumGUID();
    long m_cRef;
    ULONG m_ulCurrentIndex;
};

class CFolderViewImplCategorizer_Name : public ICategorizer
{
public:
    CFolderViewImplCategorizer_Name(IShellFolder2 *psf);

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICategorizer
    IFACEMETHODIMP GetDescription(PWSTR pszDesc, UINT cch);
    IFACEMETHODIMP GetCategory(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD *rgCategoryIds);
    IFACEMETHODIMP GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO *pci);
    IFACEMETHODIMP CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2);

private:
    ~CFolderViewImplCategorizer_Name();
    long m_cRef;
    IShellFolder2  *m_psf;
};

class CFolderViewImplCategorizer_Size : public ICategorizer
{
public:
    CFolderViewImplCategorizer_Size(IShellFolder2 *psf);

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICategorizer
    IFACEMETHODIMP GetDescription(PWSTR pszDesc, UINT cch);
    IFACEMETHODIMP GetCategory(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD *rgCategoryIds);
    IFACEMETHODIMP GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO *pci);
    IFACEMETHODIMP CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2);

private:
    ~CFolderViewImplCategorizer_Size();
    long m_cRef;
    IShellFolder2  *m_psf;
};

class CFolderViewImplCategorizer_Sides : public ICategorizer
{
public:
    CFolderViewImplCategorizer_Sides(IShellFolder2 *psf);

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICategorizer
    IFACEMETHODIMP GetDescription(PWSTR pszDesc, UINT cch);
    IFACEMETHODIMP GetCategory(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD *rgCategoryIds);
    IFACEMETHODIMP GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO *pci);
    IFACEMETHODIMP CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2);

private:
    ~CFolderViewImplCategorizer_Sides();
    long m_cRef;
    IShellFolder2  *m_psf;
};

class CFolderViewImplCategorizer_Level : public ICategorizer
{
public:
    CFolderViewImplCategorizer_Level(IShellFolder2 *psf);

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICategorizer
    IFACEMETHODIMP GetDescription(PWSTR pszDesc, UINT cch);
    IFACEMETHODIMP GetCategory(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD *rgCategoryIds);
    IFACEMETHODIMP GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO *pci);
    IFACEMETHODIMP CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2);

private:
    ~CFolderViewImplCategorizer_Level();
    long m_cRef;
    IShellFolder2  *m_psf;
};

class CFolderViewImplCategorizer_Value : public ICategorizer
{
public:
    CFolderViewImplCategorizer_Value(IShellFolder2 *psf);

    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICategorizer
    IFACEMETHODIMP GetDescription(PWSTR pszDesc, UINT cch);
    IFACEMETHODIMP GetCategory(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD *rgCategoryIds);
    IFACEMETHODIMP GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO *pci);
    IFACEMETHODIMP CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2);

private:
    ~CFolderViewImplCategorizer_Value();
    long m_cRef;
    IShellFolder2  *m_psf;
};
