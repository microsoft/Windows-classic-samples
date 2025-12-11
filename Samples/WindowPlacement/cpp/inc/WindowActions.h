#pragma once
// Note: Included by User32Utils.h, if USE_WINDOW_ACTION_APIS is defined.

//
// Note: These APIs are only available on newer OS builds. Apps using these
// APIs that need to run on older releases should handle a fallback when the
// APIs fail.
//

// ========================================================================

//
// CWindowAction
//
// Helper class to set fields in a WINDOW_ACTION and call ApplyWindowAction.
//
// A Window Action describes changes to make to a top level window. Moving,
// sizing, activating, maximizing, etc.
//
// The 'kinds' field of the action is flags that describe the changes to make.
// Some have no additional payload, like WAK_ACTIVATE, and others like
// WAK_POSITION have a corresponding field containing the value.
//
// The 'modifiers' field is also flags, some of which have their own field. The
// modifiers change the behavior of one or more of the kinds in some way. For
// example, the modifier WAM_FRAME_BOUNDS changes how the provided rect
// (position/size) are interpretted.
//
class CWindowAction : public WINDOW_ACTION
{
public:
    CWindowAction()
    {
        RtlZeroMemory(this, sizeof(this));
    }

    // Calls ApplyWindowAction to apply the changes in the action to the window.
    bool Apply(HWND hwnd)
    {
        return ApplyWindowActionWrapper(hwnd, this);
    }

    //
    // The functions below set fields in the action.
    //

    // Activates the window.
    // This makes the window the active window (GetActiveWindow), and the
    // foreground window (GetForegroundWindow) if the app is in foreground.
    void SetActivate()
    {
        WI_SetFlag(kinds, WAK_ACTIVATE);
    }

    // Shows (or hides) a window. This sets/clears WS_VISIBLE.
    void SetVisible(bool val = true)
    {
        WI_SetFlag(kinds, WAK_VISIBILITY);
        visible = val;
    }

    // The insert after window is the window that this window is behind, or
    // below (in z-order). This window can be special sentinel values like
    // HWND_TOP or HWND_TOPMOST, which have special meaning.
    void SetInsertAfter(HWND val)
    {
        WI_SetFlag(kinds, WAK_INSERT_AFTER);
        insertAfter = val;
    }

    // Set the position and size (the rect).
    // By default, this rect includes parts of the window that are invisible
    // resize borders. (As opposed to 'frame bounds', the visible bounds of
    // the window, with invisible resize borders removed.)
    void SetRect(RECT rc)
    {
        WI_SetAllFlags(kinds, WAK_POSITION | WAK_SIZE);
        position.x = rc.left;
        position.y = rc.top;
        size.cx = RECTWIDTH(rc);
        size.cy = RECTHEIGHT(rc);
    }

    // Changes the behavior of the provided rect, indicating the rect does not
    // include invisible resize borders (it is the desired visible bounds of
    // the window). This is most useful with Arranged positions, which normally
    // align the visible bounds of the window with the edges of the monitor.
    void SetFrameBounds()
    {
        WI_SetFlag(modifiers, WAM_FRAME_BOUNDS);
    }

    // Sets the state, Minimize, Maximize, Arrange, Restore.
    // When Min/Max/Arranged, the window has a 'normal' position that is
    // separate from it's current position. (Normal is the last non-special
    // position.) Restoring a window from Max/Min/Arrange moves it back to the
    // normal position.
    void SetState(WINDOW_PLACEMENT_STATE state)
    {
        WI_SetFlag(kinds, WAK_PLACEMENT_STATE);
        placementState = state;
    }

    // Sets the normal rect. This requires also setting the state.
    //
    // If Max/Min/Arranged, the normal rect overrides the default normal
    // position (which is the previous non-special position the window had).
    // If the state is Restored, the normal position has the same meaning as
    // the position and size (the rect).
    void SetNormalRect(RECT rc)
    {
        WI_SetFlag(kinds, WAK_NORMAL_RECT);
        normalRect = rc;
    }

    void SetMaximized()
    {
        SetState(WPS_MAXIMIZED);
    }

    void SetRestored()
    {
        SetState(WPS_NORMAL);
    }

    // Setting Arranged requires an arranged rect, which is in frame bounds
    // (visible bounds, no invisible resize borders). This is expected to be
    // aligned with edges of the work area (like left half, or corner, etc).
    void SetArranged(RECT arrangeRect)
    {
        SetState(WPS_ARRANGED);
        SetRect(arrangeRect);
        SetFrameBounds();
    }

    // Minimize normally remembers the previous state of the window (if
    // Maximized or Arranged). Restoring a Minimized windows returns the window
    // to that previous state. Optionally, an action can specify the window be
    // Minimized but restore to Maximized or Arranged.
    void SetMinimized()
    {
        SetState(WPS_MINIMIZED);
    }

    void SetMinRestoreToMaximized()
    {
        SetState(WPS_MINIMIZED);
        WI_SetFlag(modifiers, WAM_RESTORE_TO_MAXIMIZED);
    }

    void SetMinRestoreToArranged(RECT arrangeRect)
    {
        SetState(WPS_MINIMIZED);
        WI_SetFlag(modifiers, WAM_RESTORE_TO_ARRANGED);
        SetRect(arrangeRect);
        SetFrameBounds();
    }

    // The fit to monitor flag causes the window's normal position to be moved
    // as needed to stay entirely within the bounds of the work area.
    void SetFitToMonitor()
    {
        WI_SetFlag(kinds, WAK_FIT_TO_MONITOR);
    }

    // The Move to Monitor flag specifies the monitor the window should be on,
    // using a point (this picks the nearest monitor to this point, in screen
    // coordinates).
    void SetMoveToMonitorPoint(POINT point)
    {
        WI_SetFlag(kinds, WAK_MOVE_TO_MONITOR);
        pointOnMonitor = point;
    }

    void SetMoveToMonitor(MonitorData monitor)
    {
        SetMoveToMonitorPoint({ monitor.workArea.left, monitor.workArea.top });
    }

    // The provided position and size are assumed to be picked for the current
    // monitors. If a position is from the past, the action can specify the
    // work area from the past. The provided position is adjusted if the work
    // area has changed, to ensure the window's position relative to the monitor
    // stays the same.
    void SetPreviousWorkArea(RECT prevWorkArea)
    {
        WI_SetFlag(modifiers, WAM_WORK_AREA);
        workArea = prevWorkArea;
    }

    // The provided size is assumed to be picked for the window's current DPI.
    // If the size is picked from a past DPI, or for the DPI of the monitor,
    // the DPI field in the action should be set to that DPI. This ensures that
    // if the window changes DPI, the final size matches the one provided.
    void SetPreviousDpi(UINT prevDpi)
    {
        WI_SetFlag(modifiers, WAM_DPI);
        dpi = prevDpi;
    }
};

// Called by PlacementEx::SetPlacement, if IsApplyWindowActionSupported.
// This translates the PlacementEx into a Window Action and calls ApplyWindowAction.
// If this succeeds, the caller will return without making additional changes
// (all the changes SetPlacement makes are made here if ApplyWindowAction is supported).
/* static */
inline bool
PlacementEx::ApplyPlacementExAsAction(HWND hwnd, PlacementEx* placement)
{
    const bool fVirtDesktop = placement->HasFlag(PlacementFlags::VirtualDesktopId);
    const bool fFullScreen = placement->HasFlag(PlacementFlags::FullScreen);

    // Hide window temporarily if moving multiple times.
    TempCloakWindowIf hideIf(hwnd, fVirtDesktop || fFullScreen);

    CWindowAction action;

    // Activate by default, unless NoActivate flag.
    // If setting a virtual desktop this implies NoActivate.
    if (!placement->HasFlag(PlacementFlags::NoActivate) && !fVirtDesktop)
    {
        action.SetActivate();
    }

    // Show window by default, unless KeepHidden and previously invisible.
    if (!IsWindowVisible(hwnd) &&
        !placement->HasFlag(PlacementFlags::KeepHidden))
    {
        action.SetVisible();
    }

    // Set Fit to Monitor by default, unless AllowPartiallyOffScreen.
    if (!placement->HasFlag(PlacementFlags::AllowPartiallyOffScreen))
    {
        action.SetFitToMonitor();
    }

    // Set the state (Min/Max/Arrange/Restore).
    //
    // This is determined by the show command (SW_MAXIMIZE, SW_MINIMIZE, etc),
    // and by the Arranged PlacementEx flag.
    //
    // If Minimize, the two PlacementEx flags RestoreToMaximized/Arranged set
    // the restore state (when window restores from Min).
    //
    // If Arrange, or Min restore to Arrange, the PlacementEx has an arrange
    // position. This is a rect within the work area in the placement, normally
    // aligned with 2 or 3 edges of the work area. (Position does not include
    // invisible frame bounds, WAM_FRAME_BOUNDS.)
    if (placement->showCmd == SW_MAXIMIZE)
    {
        action.SetMaximized();
    }
    else if (IsMinimizeShowCmd(placement->showCmd))
    {
        if (placement->HasFlag(PlacementFlags::RestoreToMaximized))
        {
            action.SetMinRestoreToMaximized();
        }
        else if (placement->HasFlag(PlacementFlags::RestoreToArranged))
        {
            action.SetMinRestoreToArranged(placement->arrangeRect);
        }
        else
        {
            action.SetMinimized();
        }
    }
    else if (placement->HasFlag(PlacementFlags::Arranged))
    {
        action.SetArranged(placement->arrangeRect);
    }
    else
    {
        action.SetRestored();
    }

    // Set the normal rect. This is the restore position if Min/Max/Arrange,
    // or the window position if restored.
    action.SetNormalRect(placement->normalRect);

    // Set the work area and DPI. These are from the window/monitor when the
    // placement was created, and are used to update the position as needed if
    // the monitors have changed.
    action.SetPreviousWorkArea(placement->workArea);
    action.SetPreviousDpi(placement->dpi);

    // Call ApplyWindowAction to apply the changes.
    if (!action.Apply(hwnd))
    {
        return false;
    }

    // Enter FullScreen, if needed.
    if (fFullScreen)
    {
        placement->EnterFullScreen(hwnd);
    }

#ifdef USE_VIRTUAL_DESKTOP_APIS
    // Set virtual desktop ID, if needed.
    if (fVirtDesktop)
    {
        MoveToVirtualDesktop(hwnd, placement->virtualDesktopId);
    }
#endif

    return true;
}

// ApplyWindowAction is dynamically loaded the first time it is called.
using fnApplyWindowAction = BOOL(*)(HWND hwnd, WINDOW_ACTION* action);
fnApplyWindowAction pfnApplyWindowAction = nullptr;

// Called once per process to load ApplyWindowAction (and check if supported).
inline void LoadApplyWindowActionApi()
{
    // Dynamically load the user32!ApplyWindowAction API.
    pfnApplyWindowAction = reinterpret_cast<fnApplyWindowAction>(
        GetProcAddress(LoadLibrary(L"user32.dll"), "ApplyWindowAction"));

    // There are some OS builds that have the API export prior to the API being
    // supported. To know if the API is supported (prior to attempting to use
    // it to move the window), we create a dummy window on the first call and
    // attempt to call ApplyWindowAction on it, to see if it returns false.
    if (pfnApplyWindowAction)
    {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        PCWSTR className = L"ProbeApplyApiWindowClassName";

        WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
        wc.hInstance = hInstance;
        wc.lpfnWndProc = DefWindowProc;
        wc.lpszClassName = className;
        RegisterClassEx(&wc);

        HWND hwnd = CreateWindowEx(
            0, className, nullptr,
            0, 0, 0, 0, 0,
            nullptr, nullptr, hInstance, nullptr);

        WINDOW_ACTION action{};
        action.kinds = WAK_POSITION;

        if (!pfnApplyWindowAction(hwnd, &action))
        {
            // The API is present but disabled. Clear the pointer so that we
            // consider it not available.
            pfnApplyWindowAction = nullptr;
        }

        DestroyWindow(hwnd);
        UnregisterClass(className, hInstance);
    }
}

// Returns true if the ApplyWindowAction API is supported on the current OS.
bool IsApplyWindowActionSupported()
{
    static bool initOnce = false;
    if (!initOnce)
    {
        initOnce = true;
        LoadApplyWindowActionApi();
    }

    return (pfnApplyWindowAction != nullptr);
}

// Calls ApplyWindowAction, if it is available and supported on the current OS.
inline bool ApplyWindowActionWrapper(HWND hwnd, WINDOW_ACTION* action)
{
    if (IsApplyWindowActionSupported())
    {
        if (pfnApplyWindowAction(hwnd, action))
        {
            return true;
        }

#ifdef BREAK_ON_WINDOW_ACTIONS_FAILURE
        __debugbreak();
#endif
    }

    return false;
}
