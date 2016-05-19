/**********************************************************************/
/*                                                                    */
/*      comp.C                                                        */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"

DWORD GetTargetClause()
{
    int nMaxClause = (int)(dwCompClsLen / sizeof(DWORD)) - 1;
    int nCnt;


    if (nMaxClause <= 0)
    {
        return (DWORD)-1L;
    }
    
    for (nCnt = 0; nCnt < nMaxClause; nCnt++)
    {
        DWORD dwCls = dwCompCls[nCnt];

        if ((bCompAttr[dwCompCls[nCnt]] == ATTR_TARGET_CONVERTED) ||
            (bCompAttr[dwCompCls[nCnt]] == ATTR_TARGET_NOTCONVERTED))
        {
            return nCnt;
        }
    }

    return (DWORD)-1L;
}

LRESULT HandleChangeAttr(HWND hWnd, BOOL fNext)
{
    BYTE bAttr[512];
    BYTE bAttrRead[512];
    UINT i,j;
    UINT uClause;
    UINT uMaxClause;
    UINT uCnt;
    UINT uCntRead;
    BYTE bAt;
    BOOL fAttrOK = FALSE;
    BOOL fAttrReadOK = FALSE;
    HIMC hIMC = NULL;
    BOOL bRet = FALSE;


    uMaxClause = (dwCompClsLen / sizeof(DWORD)) - 1;
    uClause = GetTargetClause();

    if (uClause == (DWORD)-1L)
    {
         return 0L;
    }

    if (fNext)
    {
        if (uClause + 1 <= uMaxClause)
        {
            uClause++;
        }
        else
        {
            return 0L;
        }
    }
    else
    {
        if (uClause > 0)
        {
            uClause--;
        }
        else
        {
            return 0L;
        }
    }


    uCnt = 0;

    if (uClause < uMaxClause)
    {
        for (i=0; i < uMaxClause; i++)
        {
            if (i == uClause)
            {
                switch (bCompAttr[dwCompCls[i]])
                {
                    case ATTR_INPUT:
                        bAt = ATTR_TARGET_NOTCONVERTED;
                        break;
                        break;
                                    
                    case ATTR_CONVERTED:
                        bAt = ATTR_TARGET_CONVERTED;
                        break;
                        
                    default:
                        bAt = bCompAttr[dwCompCls[i]];
                        break;
                }
            }
            else
            {
                switch (bCompAttr[dwCompCls[i]])
                {
                    case ATTR_TARGET_CONVERTED:
                        bAt = ATTR_CONVERTED;
                        break;
                        break;

                    case ATTR_TARGET_NOTCONVERTED:
                        bAt = ATTR_INPUT;
                        break;
                        
                    default:
                        bAt = bCompAttr[dwCompCls[i]];
                        break;
                }
            }

            for (j = 0; j < (dwCompCls[i+1] - dwCompCls[i]); j++)
            {
                bAttr[uCnt++] = bAt;
            }
        }
        fAttrOK = TRUE;
    }

    uCntRead = 0;

    if (uClause < uMaxClause)
    {
        for (i=0; i < uMaxClause; i++)
        {
            if (i == uClause)
            {
                switch (bCompReadAttr[dwCompReadCls[i]])
                {
                    case ATTR_INPUT:
                        bAt = ATTR_TARGET_NOTCONVERTED;
                        break;
                        break;
                        
                    case ATTR_CONVERTED:
                        bAt = ATTR_TARGET_CONVERTED;
                        break;
                        
                    default:
                        bAt = bCompReadAttr[dwCompReadCls[i]];
                        break;
                }
            }
            else
            {
                switch (bCompReadAttr[dwCompReadCls[i]])
                {
                    case ATTR_TARGET_CONVERTED:
                        bAt = ATTR_CONVERTED;
                        break;
                        break;
                        
                    case ATTR_TARGET_NOTCONVERTED:
                        bAt = ATTR_INPUT;
                        break;
                        
                    default:
                        bAt = bCompReadAttr[dwCompReadCls[i]];
                        break;
                }
            }

            for (j = 0; j < (dwCompReadCls[i+1] - dwCompReadCls[i]); j++)
            {
                bAttrRead[uCntRead++] = bAt;
            }
        }
        fAttrReadOK = TRUE;
    }
 

    if (fAttrReadOK && fAttrOK)
    {
        hIMC = ImmGetContext(hWndCompStr);
#ifdef USEWAPI
        bRet = ImmSetCompositionStringW(hIMC,SCS_CHANGEATTR,bAttr,uCnt,
                                                    bAttrRead,uCntRead);
#else
        bRet = ImmSetCompositionString(hIMC,SCS_CHANGEATTR,bAttr,uCnt,
                                                    bAttrRead,uCntRead);
#endif
        // bRet = ImmSetCompositionString(hIMC,SCS_CHANGEATTR,NULL,0,
        //                                             bAttrRead,uCntRead);
        // bRet = ImmSetCompositionString(hIMC,SCS_CHANGEATTR,bAttr,uCnt,
        //                                             NULL,0);
        ImmReleaseContext(hWndCompStr,hIMC);
    }
    else
    {
#ifdef DEBUG
        OutputDebugString("Can not call ImmSetCompositionString\r\n");
        bRet = TRUE;
#endif
    }

#ifdef DEBUG
    if (!bRet)
    {
        OutputDebugString("ImmSetCompositionString return FALSE\r\n");
    }
#endif


    return 1;
}
