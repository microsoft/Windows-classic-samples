#ifndef __COM_FAXACCOUNTNOTIFY_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------


#define __COM_FAXACCOUNTNOTIFY_SAMPLE

#include "FaxNotify.h"

class ATL_NO_VTABLE _CFaxAccountNotify : 
        public CComObjectRootEx<CComSingleThreadModel>,
        public IDispEventImpl<1, _CFaxAccountNotify, &DIID_IFaxAccountNotify, &LIBID_FAXCOMEXLib, 1, 0>
{
        public:
                _CFaxAccountNotify();

                ~_CFaxAccountNotify();

                BEGIN_COM_MAP(_CFaxAccountNotify)
                END_COM_MAP()

                BEGIN_SINK_MAP(_CFaxAccountNotify)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 1, OnIncomingJobAdded)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 2, OnIncomingJobRemoved)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 3, OnIncomingJobChanged)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 4, OnOutgoingJobAdded)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 5, OnOutgoingJobRemoved)    
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 6, OnOutgoingJobChanged)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 7, OnIncomingMessageAdded)
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 8, OnIncomingMessageRemoved)    
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 9, OnOutgoingMessageAdded)  
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 10, OnOutgoingMessageRemoved)  
                    SINK_ENTRY_EX(1, DIID_IFaxAccountNotify, 11, OnServerShutDown)  
                END_SINK_MAP()                


                HRESULT GetFuncInfoFromId(const IID& iid, DISPID dispidMember,
                                LCID lcid, _ATL_FUNC_INFO& info)
                {
                        if (InlineIsEqualGUID(iid, DIID_IFaxAccountNotify))
                        {
                                info.cc = CC_STDCALL;
                                switch(dispidMember)
                                {
                                        case 1:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 2;
                                                info.pVarTypes[1] =  VT_UNKNOWN;
                                                info.pVarTypes[0] =  VT_BSTR;                                                
                                                return S_OK;
                                       
                                        case 2:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 2;
                                                info.pVarTypes[1] =  VT_UNKNOWN;
                                                info.pVarTypes[0] =  VT_BSTR;  
                                                return S_OK;
                                        case 3:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 3;
                                                info.pVarTypes[2] =  VT_UNKNOWN;
                                                info.pVarTypes[1] =  VT_BSTR;  
                                                info.pVarTypes[0] =  VT_UNKNOWN;
                                                return S_OK;     
                                        case 4:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 2;
                                                info.pVarTypes[1] =  VT_UNKNOWN;
                                                info.pVarTypes[0] =  VT_BSTR;  
                                                return S_OK;
                                        case 5:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 2;
                                                info.pVarTypes[1] =  VT_UNKNOWN;
                                                info.pVarTypes[0] =  VT_BSTR;  
                                                return S_OK;

                                        case 6:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 3;
                                                info.pVarTypes[2] =  VT_UNKNOWN;
                                                info.pVarTypes[1] =  VT_BSTR;  
                                                info.pVarTypes[0] =  VT_UNKNOWN;
                                                return S_OK;
                                       
                                        case 7:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 3;
                                                info.pVarTypes[2] =  VT_UNKNOWN;
                                                info.pVarTypes[1] =  VT_BSTR; 
                                                info.pVarTypes[0] =  VT_BOOL;
                                                return S_OK;
                                        case 8:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 3;
                                                info.pVarTypes[2] =  VT_UNKNOWN;
                                                info.pVarTypes[1] =  VT_BSTR;  
                                                info.pVarTypes[0] =  VT_BOOL;
                                                return S_OK;     
                                        case 9:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 2;
                                                info.pVarTypes[1] =  VT_UNKNOWN;
                                                info.pVarTypes[0] =  VT_BSTR;  
                                                return S_OK;
                                        case 10:
                                                info.vtReturn = VT_I4;
                                                info.nParams = 2;
                                                info.pVarTypes[1] =  VT_UNKNOWN;
                                                info.pVarTypes[0] =  VT_BSTR;  
                                                return S_OK;

                                        case 11:                    
                                                info.vtReturn = VT_I4;
                                                info.nParams = 1;
                                                info.pVarTypes[0] =  VT_UNKNOWN;
                                                return S_OK;

                                        default:
                                                return E_FAIL;
                                }
                        }
                        return E_FAIL;
                }
                //
                // Start of IFaxAccountNotify methods
                //
                STDMETHOD(OnIncomingJobAdded)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrJobId);

                STDMETHOD(OnIncomingJobRemoved)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrJobId);

                STDMETHOD(OnIncomingJobChanged)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrJobId, 
                                /*[in]*/ IFaxJobStatus *pJobStatus);

                STDMETHOD(OnOutgoingJobAdded)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrJobId);

                STDMETHOD(OnOutgoingJobRemoved)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrJobId);

                STDMETHOD(OnOutgoingJobChanged)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrJobId, 
                                /*[in]*/ IFaxJobStatus *pJobStatus);

                STDMETHOD(OnIncomingMessageAdded)(
                                /*[in]*/ IFaxAccount *pFaxAccount,
                                /*[in]*/ BSTR bstrMessageId,
                                /*[in]*/ VARIANT_BOOL fAddedToReceiveFolder);

                STDMETHOD(OnIncomingMessageRemoved)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrMessageId,
                                /*[in]*/ VARIANT_BOOL fRemovedFromReceiveFolder);

                STDMETHOD(OnOutgoingMessageAdded)(
                                /*[in]*/ IFaxAccount *pFaxAccount, 
                                /*[in]*/ BSTR bstrMessageId);

                STDMETHOD(OnOutgoingMessageRemoved)(
                                /*[in]*/ IFaxAccount *pFaxAccount,
                                /*[in]*/ BSTR bstrMessageId);

                STDMETHOD(OnServerShutDown)(/*[in]*/ IFaxServer2 *pFaxServer);

                //
                // End of IFaxAccountNotify methods
                //

};

// _CFaxAccountNotify is an abstract class because the IUnknown methods
// aren't implemented. Make use of CComObject & with the help of
// typedef a new (concrete) class CFaxAccountNotify is created!!
typedef CComObject<_CFaxAccountNotify> CFaxAccountNotify;

#endif

