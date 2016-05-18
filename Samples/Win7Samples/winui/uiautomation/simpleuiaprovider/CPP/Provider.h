/*************************************************************************************************
 *
 * Description: Declaration of the Provider class.
 *
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 *************************************************************************************************/


class Provider : public IRawElementProviderSimple, 
        public IInvokeProvider
{
public:
    Provider(HWND hwnd);

    void InitIds();

    // IUnknown methods
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(REFIID riid, void**);
    
    // IRawElementProviderSimple methods
    IFACEMETHODIMP get_ProviderOptions(ProviderOptions * pRetVal);
    IFACEMETHODIMP GetPatternProvider(PATTERNID patternId, IUnknown ** pRetVal);
    IFACEMETHODIMP GetPropertyValue(PROPERTYID propertyId, VARIANT * pRetVal);
    IFACEMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple ** pRetVal);

    // IInvokeProvider methods
    IFACEMETHODIMP Invoke();

private:
    virtual ~Provider();
    // Ref Counter for this COM object
    ULONG m_refCount;

    HWND m_controlHWnd; // The HWND for the control.
};
