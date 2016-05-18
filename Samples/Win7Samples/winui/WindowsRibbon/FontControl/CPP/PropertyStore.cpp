// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "PropertyStore.h"
#include <UIRibbonPropertyHelpers.h>
#include <strsafe.h>

// Convert from IPropertyStore to CHARFORMAT2 so it can be used formatting text.
void GetCharFormat2FromIPropertyStore(__in IPropertyStore* pPropStore, __out CHARFORMAT2 *pCharFormat)
{
    // Initialize the output parameter.
    ZeroMemory(pCharFormat, sizeof(*pCharFormat));
    pCharFormat->cbSize = sizeof(CHARFORMAT2);
    
    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    UINT uValue;

    // Get the bold value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_Bold, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_Bold, propvar, &uValue);
        if ((UI_FONTPROPERTIES) uValue != UI_FONTPROPERTIES_NOTAVAILABLE)
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_BOLD;
            pCharFormat->dwEffects |= ((UI_FONTPROPERTIES) uValue == UI_FONTPROPERTIES_SET) ? CFE_BOLD : 0;
        }
    }
    PropVariantClear(&propvar);

    // Get the italic value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_Italic, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_Italic, propvar, &uValue);
        if ((UI_FONTPROPERTIES) uValue != UI_FONTPROPERTIES_NOTAVAILABLE)
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_ITALIC;
            pCharFormat->dwEffects |= ((UI_FONTPROPERTIES) uValue == UI_FONTPROPERTIES_SET) ? CFE_ITALIC : 0;
        }
    }
    PropVariantClear(&propvar);

    // Get the underline value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_Underline, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_Underline, propvar, &uValue);
        if ((UI_FONTUNDERLINE) uValue != UI_FONTUNDERLINE_NOTAVAILABLE)
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_UNDERLINE;
            pCharFormat->dwEffects |= ((UI_FONTUNDERLINE) uValue == UI_FONTUNDERLINE_SET) ? CFE_UNDERLINE : 0;
        }
    }
    PropVariantClear(&propvar);

    // Get the strikethrough value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_Strikethrough, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_Strikethrough, propvar, &uValue);
        if ((UI_FONTPROPERTIES) uValue != UI_FONTPROPERTIES_NOTAVAILABLE)
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_STRIKEOUT;
            pCharFormat->dwEffects |= ((UI_FONTPROPERTIES) uValue == UI_FONTPROPERTIES_SET) ? CFE_STRIKEOUT : 0;
        }
    }    
    PropVariantClear(&propvar);

    // Get the vertical positioning value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_VerticalPositioning, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_VerticalPositioning, propvar, &uValue);
        UI_FONTVERTICALPOSITION uVerticalPosition = (UI_FONTVERTICALPOSITION) uValue;
        if ((uVerticalPosition != UI_FONTVERTICALPOSITION_NOTAVAILABLE))
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= (CFM_SUPERSCRIPT | CFM_SUBSCRIPT);
            if (uVerticalPosition != UI_FONTVERTICALPOSITION_NOTSET)
            {
                pCharFormat->dwEffects |= (uVerticalPosition == UI_FONTVERTICALPOSITION_SUPERSCRIPT) ? CFE_SUPERSCRIPT : CFE_SUBSCRIPT;
            }
        }
    }
    PropVariantClear(&propvar);

    // Get the font family value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_Family, &propvar)))
    {
        // Get the string for the font family.
        PWSTR pszFamily;
        UIPropertyToStringAlloc(UI_PKEY_FontProperties_Family, propvar, &pszFamily);
        // Blank string is used as "Not Available" value.
        if (lstrcmp(pszFamily, L"")) 
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            // Copy the string for font family.
            StringCchCopyW(pCharFormat->szFaceName, ARRAYSIZE(pCharFormat->szFaceName), pszFamily);
            pCharFormat->dwMask |= CFM_FACE;
        }
        // Free the allocated string.
        CoTaskMemFree(pszFamily);
    }
    PropVariantClear(&propvar);

    // Get the font size value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_Size, &propvar)))
    {
        // Get the decimal font size value.
        DECIMAL decSize;
        UIPropertyToDecimal(UI_PKEY_FontProperties_Size, propvar, &decSize);
        DOUBLE dSize;
        VarR8FromDec(&decSize, &dSize);
        // Zero is used as "Not Available" value.
        if (dSize > 0)
        {
            // There is a value assigned, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_SIZE;
            // Set the height as twips.
            pCharFormat->yHeight = (LONG)(dSize * TWIPS_PER_POINT);
        }
    }
    PropVariantClear(&propvar);

    // Get the foreground color type value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_ForegroundColorType, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_ForegroundColorType, propvar, &uValue);
        if (UI_SWATCHCOLORTYPE_AUTOMATIC == (UI_SWATCHCOLORTYPE)uValue)
        {
            // The color type is automatic, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_COLOR;
            pCharFormat->dwEffects |= CFE_AUTOCOLOR;
        }
    }
    PropVariantClear(&propvar);

    // Get the foreground color value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_ForegroundColor, &propvar)))
    {
        // A font color is specified so set the corresponding members in CharFormat2 variable.
        UIPropertyToUInt32(UI_PKEY_FontProperties_ForegroundColor, propvar, &uValue);
        pCharFormat->dwMask |= CFM_COLOR;
        pCharFormat->crTextColor = (COLORREF)uValue;
    }
    PropVariantClear(&propvar);

    // Get the background color type value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_BackgroundColorType, &propvar)))
    {
        UIPropertyToUInt32(UI_PKEY_FontProperties_BackgroundColorType, propvar, &uValue);
        if (UI_SWATCHCOLORTYPE_NOCOLOR == (UI_SWATCHCOLORTYPE)uValue)
        {
            // The color type is no color, so set the corresponding members in CharFormat2 variable.
            pCharFormat->dwMask |= CFM_BACKCOLOR;
            pCharFormat->dwEffects |= CFE_AUTOBACKCOLOR;
        }
    }
    PropVariantClear(&propvar);

    // Get the background color value from the property store.
    if (SUCCEEDED(pPropStore->GetValue(UI_PKEY_FontProperties_BackgroundColor, &propvar)))
    {
        // A color is specified so set the corresponding members in CharFormat2 variable.
        UIPropertyToUInt32(UI_PKEY_FontProperties_BackgroundColor, propvar, &uValue);
        pCharFormat->dwMask |= CFM_BACKCOLOR;
        pCharFormat->crBackColor = (COLORREF)uValue;
    }
    PropVariantClear(&propvar);
}

// Convert from CHARFORMAT2 to IPropertyStore so it can be passed to the font control.
void GetIPropStoreFromCharFormat2(const __in CHARFORMAT2* pCharFormat, __in IPropertyStore *pPropStore)
{
    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    
    if (pCharFormat->dwMask & CFM_BOLD)
    {
        // Set the bold value to UI_FONTPROPERTIES_SET or UI_FONTPROPERTIES_NOTSET.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Bold, (pCharFormat->dwEffects & CFE_BOLD) ? UI_FONTPROPERTIES_SET : UI_FONTPROPERTIES_NOTSET, &propvar);
    }
    else
    {
        // The bold value is not available so set it to UI_FONTPROPERTIES_NOTAVAILABLE.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Bold, UI_FONTPROPERTIES_NOTAVAILABLE, &propvar);
    }
    // Set UI_PKEY_FontProperties_Bold value in property store.
    pPropStore->SetValue(UI_PKEY_FontProperties_Bold, propvar);
    PropVariantClear(&propvar);

    if (pCharFormat->dwMask & CFM_ITALIC)
    {
        // Set the italic value to UI_FONTPROPERTIES_SET or UI_FONTPROPERTIES_NOTSET.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Italic, (pCharFormat->dwEffects & CFE_ITALIC)?UI_FONTPROPERTIES_SET:UI_FONTPROPERTIES_NOTSET, &propvar);
    }
    else
    {
        // The italic value is not available so set it to UI_FONTPROPERTIES_NOTAVAILABLE.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Italic, UI_FONTPROPERTIES_NOTAVAILABLE, &propvar);
    }
    // Set UI_PKEY_FontProperties_Italic value in property store.
    pPropStore->SetValue(UI_PKEY_FontProperties_Italic, propvar);
    PropVariantClear(&propvar);

    if (pCharFormat->dwMask & CFM_UNDERLINE)
    {
        // Set the underline value to UI_FONTUNDERLINE_SET or UI_FONTUNDERLINE_NOTSET.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Underline, (pCharFormat->dwEffects & CFE_UNDERLINE) ? UI_FONTUNDERLINE_SET : UI_FONTUNDERLINE_NOTSET, &propvar);
    }
    else
    {
        // The underline value is not available so set it to UI_FONTUNDERLINE_NOTAVAILABLE.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Underline, UI_FONTUNDERLINE_NOTAVAILABLE, &propvar);
    }
    // Set UI_PKEY_FontProperties_Underline value in property store.
    pPropStore->SetValue(UI_PKEY_FontProperties_Underline, propvar);
    PropVariantClear(&propvar);
    
    if (pCharFormat->dwMask & CFM_STRIKEOUT)
    {
        // Set the strikethrough value to UI_FONTPROPERTIES_SET or UI_FONTPROPERTIES_NOTSET.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Strikethrough, (pCharFormat->dwEffects & CFE_STRIKEOUT) ? UI_FONTPROPERTIES_SET : UI_FONTPROPERTIES_NOTSET, &propvar);
    }
    else
    {
        // The strikethrough value is not available so set it to UI_FONTPROPERTIES_NOTAVAILABLE.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_Strikethrough, UI_FONTPROPERTIES_NOTAVAILABLE, &propvar);
    }
    // Set UI_PKEY_FontProperties_Strikethrough value in property store.
    pPropStore->SetValue(UI_PKEY_FontProperties_Strikethrough, propvar);
    PropVariantClear(&propvar);

    if (pCharFormat->dwMask & CFE_SUBSCRIPT)
    {
        if ((pCharFormat->dwMask & CFM_SUBSCRIPT) && (pCharFormat->dwEffects & CFE_SUBSCRIPT))
        {
            // Set the vertical positioning value to UI_FONTVERTICALPOSITION_SUBSCRIPT.
            UIInitPropertyFromUInt32(UI_PKEY_FontProperties_VerticalPositioning, UI_FONTVERTICALPOSITION_SUBSCRIPT, &propvar);
        }
        else if (pCharFormat->dwEffects & CFE_SUPERSCRIPT)
        {
            // Set the vertical positioning value to UI_FONTVERTICALPOSITION_SUPERSCRIPT.
            UIInitPropertyFromUInt32(UI_PKEY_FontProperties_VerticalPositioning, UI_FONTVERTICALPOSITION_SUPERSCRIPT, &propvar);
        }
    }
    else if (pCharFormat->dwMask & CFM_OFFSET)
    {
        if (pCharFormat->yOffset > 0)
        {
            // Set the vertical positioning value to UI_FONTVERTICALPOSITION_SUPERSCRIPT.
            UIInitPropertyFromUInt32(UI_PKEY_FontProperties_VerticalPositioning, UI_FONTVERTICALPOSITION_SUPERSCRIPT, &propvar);
        }
        else if (pCharFormat->yOffset < 0)
        {
            // Set the vertical positioning value to UI_FONTVERTICALPOSITION_SUBSCRIPT.
            UIInitPropertyFromUInt32(UI_PKEY_FontProperties_VerticalPositioning, UI_FONTVERTICALPOSITION_SUBSCRIPT, &propvar);
        }
        else
        {
            // The value is not available so set the vertical positioning value to UI_FONTVERTICALPOSITION_NOTAVAILABLE.
            UIInitPropertyFromUInt32(UI_PKEY_FontProperties_VerticalPositioning, UI_FONTVERTICALPOSITION_NOTAVAILABLE, &propvar);
        }

    }
    // Set UI_PKEY_FontProperties_VerticalPositioning value in property store.
    pPropStore->SetValue(UI_PKEY_FontProperties_VerticalPositioning, propvar);
    PropVariantClear(&propvar);

    if (pCharFormat->dwMask & CFM_FACE)
    {
        // Set the font family value to the font name.
        UIInitPropertyFromString(UI_PKEY_FontProperties_Family, pCharFormat->szFaceName, &propvar);
    }
    else
    {
        // Font family name is not available so set it to blank string.
        UIInitPropertyFromString(UI_PKEY_FontProperties_Family, L"", &propvar);
    }
    // Set UI_PKEY_FontProperties_Family value in property store.
    pPropStore->SetValue(UI_PKEY_FontProperties_Family, propvar);
    PropVariantClear(&propvar);

    DECIMAL decSize;
    if (pCharFormat->dwMask & CFM_SIZE)
    {
        // Font size value is available so get the font size.
        VarDecFromR8((DOUBLE)pCharFormat->yHeight / TWIPS_PER_POINT, &decSize);
    }
    else
    {
        // The font size is not available so set it to zero.
        VarDecFromI4(0, &decSize);
    }
    // Set UI_PKEY_FontProperties_Size value in property store.
    UIInitPropertyFromDecimal(UI_PKEY_FontProperties_Size, decSize, &propvar); 
    pPropStore->SetValue(UI_PKEY_FontProperties_Size, propvar);
    PropVariantClear(&propvar);

    if ((pCharFormat->dwMask & CFM_COLOR) && !(pCharFormat->dwEffects & CFE_AUTOCOLOR))
    {
        // There is a color value so set the type to UI_SWATCHCOLORTYPE_RGB in property store.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_ForegroundColorType, UI_SWATCHCOLORTYPE_RGB, &propvar);
        pPropStore->SetValue(UI_PKEY_FontProperties_ForegroundColorType, propvar);
        PropVariantClear(&propvar);
        
        // Set the color value in property store.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_ForegroundColor, pCharFormat->crTextColor, &propvar);
        pPropStore->SetValue(UI_PKEY_FontProperties_ForegroundColor, propvar);
    }
    else if ((pCharFormat->dwMask & CFM_COLOR) && (pCharFormat->dwEffects & CFE_AUTOCOLOR))
    {
        // The color is automatic color so set the type to UI_SWATCHCOLORTYPE_AUTOMATIC in property store.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_ForegroundColorType, UI_SWATCHCOLORTYPE_AUTOMATIC, &propvar);
        pPropStore->SetValue(UI_PKEY_FontProperties_ForegroundColorType, propvar);
    }
    PropVariantClear(&propvar);

    if ((pCharFormat->dwMask & CFM_BACKCOLOR) && !(pCharFormat->dwEffects & CFE_AUTOBACKCOLOR))
    {
        // There is a color value so set the type to UI_SWATCHCOLORTYPE_RGB in property store.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_BackgroundColorType, UI_SWATCHCOLORTYPE_RGB, &propvar);
        pPropStore->SetValue(UI_PKEY_FontProperties_BackgroundColorType, propvar);
        PropVariantClear(&propvar);
        
        // Set the color value in property store.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_BackgroundColor, pCharFormat->crBackColor, &propvar);
        pPropStore->SetValue(UI_PKEY_FontProperties_BackgroundColor, propvar);
    }
    else
    {
        // There is no color so set the type to UI_SWATCHCOLORTYPE_NOCOLOR in property store.
        UIInitPropertyFromUInt32(UI_PKEY_FontProperties_BackgroundColorType, UI_SWATCHCOLORTYPE_NOCOLOR, &propvar);
        pPropStore->SetValue(UI_PKEY_FontProperties_BackgroundColorType, propvar);
    }
}
