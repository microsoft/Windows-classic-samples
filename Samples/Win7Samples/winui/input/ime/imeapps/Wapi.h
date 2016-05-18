/* Unicode/Ansi stubs */
#ifdef USEWAPI
typedef LPWSTR            LPMYSTR;
typedef WCHAR             MYCHAR;
#define MYTEXT(x)         L ## x
#define Mylstrlen(x)      lstrlenW(x)
#define Mylstrcmp(x, y)   MylstrcmpW((x), (y))
#define MyCharPrev(x, y)  MyCharPrevW((x), (y))
#define MyCharNext(x)     MyCharNextW(x)
#define LPMYIMEMENUITEMINFO LPIMEMENUITEMINFOW
#define MYIMEMENUITEMINFO IMEMENUITEMINFOW
#define MyImmGetImeMenuItems  ImmGetImeMenuItemsW
#define MyImmGetCompositionString ImmGetCompositionStringW
#else
typedef LPTSTR            LPMYSTR;
typedef TCHAR             MYCHAR;
#define MYTEXT(x)         x
#define Mylstrlen(x)      lstrlen(x)
#define Mylstrcmp(x, y)   lstrcmp((x), (y))
#define MyCharPrev(x, y)  AnsiPrev((x), (y))
#define MyCharNext(x)     AnsiNext(x)
#define LPMYIMEMENUITEMINFO LPIMEMENUITEMINFO
#define MYIMEMENUITEMINFO IMEMENUITEMINFO
#define MyImmGetImeMenuItems  ImmGetImeMenuItems
#define MyImmGetCompositionString ImmGetCompositionString
#endif

#ifdef USEWAPI
int WINAPI MylstrcmpW(LPWSTR lp0, LPWSTR lp1);
LPWSTR WINAPI MyCharPrevW(LPWSTR lpStart, LPWSTR lpCur);
LPWSTR WINAPI MyCharNextW(LPWSTR lp);
#endif
