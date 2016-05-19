#include "private.h"
#include "TextEditSink.h"
#include "TextEditor.h"
#include "DisplayAttribute.h"

//////////////////////////////////////////////////////////////////////////////
//
// CTextEditSink
//
//////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextEditSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTextEditSink::Release()
{
    long cr;

    cr = --_cRef;

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CTextEditSink::CTextEditSink(CTextEditor *pEditor)
{
    _cRef = 1;
    _dwEditCookie = TES_INVALID_COOKIE;
    _pEditor = pEditor;
}

//+---------------------------------------------------------------------------
//
// EndEdit
//
//----------------------------------------------------------------------------

STDAPI CTextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    CDispAttrProps *pDispAttrProps = GetDispAttrProps();
    if (pDispAttrProps)
    {
        IEnumTfRanges *pEnum;
        if (SUCCEEDED(pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT,
                                                             pDispAttrProps->GetPropTablePointer(),
                                                             pDispAttrProps->Count(),
                                                             &pEnum)) && pEnum)
        {
            ITfRange *pRange;
            if (pEnum->Next(1, &pRange, NULL) == S_OK)
            {
                // We check if there is a range to be changed.
                pRange->Release();

                _pEditor->ClearCompositionRenderInfo();

                // We read the display attribute for entire range.
                // It could be optimized by filtering the only delta with ITfEditRecord interface. 
                ITfRange *pRangeEntire = NULL;
                ITfRange *pRangeEnd = NULL;
                if (SUCCEEDED(pic->GetStart(ecReadOnly, &pRangeEntire)) &&
                    SUCCEEDED(pic->GetEnd(ecReadOnly, &pRangeEnd)) &&
                    SUCCEEDED(pRangeEntire->ShiftEndToRange(ecReadOnly, pRangeEnd, TF_ANCHOR_END)))
                {
                    IEnumTfRanges *pEnumRanges;
                    ITfReadOnlyProperty *pProp = NULL;

                    GetDisplayAttributeTrackPropertyRange(ecReadOnly, pic, pRangeEntire, &pProp, pDispAttrProps);

                    if (SUCCEEDED(pProp->EnumRanges(ecReadOnly, &pEnumRanges, pRangeEntire)))
                    {
                        while (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
                        {
                            TF_DISPLAYATTRIBUTE da;
                            TfGuidAtom guid;
                            if (GetDisplayAttributeData(ecReadOnly, pProp, pRange, &da, &guid) == S_OK)
                            {
                                ITfRangeACP *pRangeACP;
                                if (pRange->QueryInterface(IID_ITfRangeACP, (void **)&pRangeACP) == S_OK)
                                {
                                    LONG nStart;
                                    LONG nEnd;
                                    pRangeACP->GetExtent(&nStart, &nEnd);
                                    _pEditor->AddCompositionRenderInfo(nStart, nStart + nEnd, &da);
                                    pRangeACP->Release();
                                }
                            }
                        }
                    }
                }

                if (pRangeEntire)
                    pRangeEntire->Release();
                if (pRangeEnd)
                    pRangeEnd->Release();
 
            }
            pEnum->Release();
        }

        delete pDispAttrProps;
    }

    return S_OK;
}


//+---------------------------------------------------------------------------
//
// CTextEditSink::Advise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _pic = NULL;
    hr = E_FAIL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwEditCookie)))
        goto Exit;

    _pic = pic;
    _pic->AddRef();

    hr = S_OK;

Exit:
    if (source)
        source->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// CTextEditSink::Unadvise
//
//----------------------------------------------------------------------------

HRESULT CTextEditSink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_pic == NULL)
        goto Exit;

    if (FAILED(_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->UnadviseSink(_dwEditCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    if (source)
        source->Release();

    if (_pic)
    {
        _pic->Release();
        _pic = NULL;
    }

    return hr;
}
