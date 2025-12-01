#ifndef __COMPOSITION_DCOMP_PREVIEW_H__
#define __COMPOSITION_DCOMP_PREVIEW_H__

#if (NTDDI_VERSION >= NTDDI_WIN11_GE)

#undef INTERFACE
#define INTERFACE PREVIEW_IDCompositionDynamicTexture
DECLARE_INTERFACE_IID_(PREVIEW_IDCompositionDynamicTexture, IUnknown, "A1DE1D3F-6405-447F-8E95-1383A34B0277")
{
    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture * pTexture) PURE;

    STDMETHOD(SetTexture)(THIS_
        _In_ IDCompositionTexture * pTexture,
        _In_count_(rectCount) const D2D_RECT_L * pRects,
        _In_ size_t rectCount) PURE;
};

#undef INTERFACE
#define INTERFACE PREVIEW_IDCompositionDevice5
DECLARE_INTERFACE_IID_(PREVIEW_IDCompositionDevice5, IDCompositionDevice4, "2C6BEBFE-A603-472F-AF34-D2443356E61B")
{
    STDMETHOD(CreateDynamicTexture)(THIS_
        _Outptr_ PREVIEW_IDCompositionDynamicTexture * *compositionDynamicTexture) PURE;
};

#endif // #if (NTDDI_VERSION >= NTDDI_WIN11_GE)

#endif // #ifndef __COMPOSITION_DCOMP_PREVIEW_H__
