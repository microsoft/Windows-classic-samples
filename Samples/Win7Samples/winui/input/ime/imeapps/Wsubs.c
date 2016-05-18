#include <windows.h>

#ifdef USEWAPI
int WINAPI MylstrcmpW(LPWSTR lp0, LPWSTR lp1)
{

    while(*lp0)
    {
        if (*lp0 != *lp1)
        {
            return (-1);
        }
        lp0++;
        lp1++;

    }
    
    if (!*lp1)
    {
        return 0;
    }

    return 1;
}


LPWSTR WINAPI MyCharPrevW(LPWSTR lpStart, LPWSTR lpCur)
{
    LPWSTR lpRet = lpStart;
    if (lpCur > lpStart)
    {
        lpRet = lpCur - 1;
    }

    return lpRet;
}

LPWSTR WINAPI MyCharNextW(LPWSTR lp)
{
    return lp++;
}
#endif

