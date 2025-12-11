#pragma once
// Note: Intended to be included by User32Utils.h.

//
// CurrentMonitorTopology
//
// This class caches the data about all of the connected monitors. It contains
// helpers that can be used to get data about the monitors, or find the data
// for a monitor given a point/rect/window, etc.
//
// IMPORTANT: This requires the thread is pumping messages (Get/PeekMessage).
// The monitor list changes when a window created by this class receives a
// WM_DISPLAYCHANGE or WM_SETTINGCHANGE/SPI_SETWORKAREA message.
//
// There are two benefits of using this class, as opposed to calling OS APIs
// (MonitorFromRect, GetMonitorInfo, etc) whenever the data is needed.
//
//  1. Performance
//
//      This class caches all monitor info. The OS APIs read the real monitor
//      data, which requires taking a global system lock. Calling these APIs
//      frequently enough can cause this app and the whole system to slow down.
//
//  2. Consistency
//
//      The real monitor list (the HMONITORs) can change at any time, including
//      between calls to MonitorFromPoint and GetMonitorInfo.
//      Using this class provides a consistent view of the monitor topology.
//      It will never change while this thread is processing another message
//

// A SharedMonitorPtr is a referenced counted pointer to data about a monitor.
// This data is for a moment in time (potentially in the past).
// It is safe to use this pointer in the future, because it is a shared_ptr,
// but the current monitor topology may have changed.
// See MonitorData.h for MonitorData definition.
typedef std::shared_ptr<const MonitorData> SharedMonitorPtr;

// TODO: Split out 'slim' version, which doesn't create a window and requires
// the user to call Update() on WM_DISPLAYCHANGE/WM_SETTINGCHANGE.

class CurrentMonitorTopology
{
public:
    // Returns the number of monitors.
    UINT NumMonitors() const
    {
        return m_numMonitors;
    }

    // Returns the primary monitor (the one whose origin is 0, 0).
    SharedMonitorPtr
    GetPrimaryMonitor()
    {
        return MonitorFromPoint( {0, 0} );
    }

    // The topology ID is a unique ID for the monitor topology.
    //
    // This can be used to see if any changes have been made since some point
    // in the past.
    //
    // Note: In GE_RELEASE+, this uses a new system API, GetCurrentMonitorTopologyId.
    // On releases that have the API, this topology ID is global (shared by all
    // apps in the user session). On earlier releases, this class implements a
    // local counter whenever the monitor topology is refreshed.
    UINT GetTopologyId() const
    {
        return m_lastTopologyId;
    }

    //
    // ForEachMonitor
    // Allows the caller to do something for each monitor.
    // The provided function takes a MonitorData and returns true if the walk
    // should continue.
    //
    typedef std::function<bool (SharedMonitorPtr monitor)> ForEachMonitorFn;
    void ForEachMonitor(ForEachMonitorFn fn);

    //
    // MonitorFromX functions.
    //
    //      MonitorFromPoint
    //      MonitorFromRect
    //      MonitorFromWindow
    //      MonitorFromPastMonitor
    //
    // These functions default to 'nearest'. This means that if the point/rect
    // is not on any monitor, the function falls back to the nearest monitor.
    // Each function also accepts 'null' and 'primary' as fallback options.
    //

    enum class Fallback
    {
        Null = 0,       // MONITOR_DEFAULTTONULL
        Primary = 1,    // MONITOR_DEFAULTTOPRIMARY
        Nearest = 2,    // MONITOR_DEFAULTTONEAREST
    };

    SharedMonitorPtr
    MonitorFromPoint(
        POINT pt,
        Fallback fallback = Fallback::Nearest);

    SharedMonitorPtr
    MonitorFromRect(
        const RECT& rc,
        Fallback fallback = Fallback::Nearest);

    // A window's monitor is defined as the monitor it's RECT is mostly on.
    // (If a window is not overlapping any monitor, the fallback determines if
    // this returns null/primary/nearest.)
    //
    // IMPORTANT: This could be different from the monitor that the window is
    // scaling to (for DPI). For all scaling purposes, use GetDpiForWindow,
    // NOT the DPI of the MonitorFromWindow.
    SharedMonitorPtr
    MonitorFromWindow(
        HWND hwnd,
        Fallback fallback = Fallback::Nearest)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        return MonitorFromRect(rc, fallback);
    }

    SharedMonitorPtr
    MonitorFromName(PCWSTR deviceName);

    // Given a monitor from the past (copied from the current monitor topology
    // at some point in the past), this finds a monitor in the current topology
    // that best matches the past monitor.
    SharedMonitorPtr
    MonitorFromPastMonitor(
        const MonitorData& previousMonitor,
        Fallback fallback = Fallback::Nearest);

    //
    // Topology change notification.
    //
    // A window can register to be notified when the topology changes. It
    // provides its window handle and a message ID, and when the topology
    // data is changed the window is sent this message, allowing it to respond
    // to the updated topology data.
    //

    void RegisterTopologyChangeMessage(HWND hwndNotify, UINT msgId);

    //
    // Constructor and destructor.
    //

    CurrentMonitorTopology()
    {
        m_validDpiAwarenesses[GetThreadDpiAwareness()] = true;

        RefreshMonitorData(false /* force */);

        m_hwndListener = CreateListenerWindow();
    }

    ~CurrentMonitorTopology()
    {
        DestroyWindow(m_hwndListener);
    }

private:
    // Note: This function attempts to dynamically load (GetProcAddress) a
    // recently-added API:
    //
    //  user32!GetCurrentMonitorTopologyId
    //
    // This API returns a non-zero unique ID for the monitor topology. If any
    // of the monitors change, this ID is updated. If the API is not available,
    // this returns 0.
    //
    // Note: this API was added to user32 in GE_RELEASE but is not present in
    // header files, so it must be dynamically loaded.
    // https://learn.microsoft.com/en-us/windows/win32/winmsg/winuser/nf-winuser-getcurrentmonitortopologyid
    static UINT GetCurrentMonitorTopologyId();

    // Called for each monitor when syncing the monitor data with the current
    // monitor topology. This queries for some additional data, stores the data
    // in a MonitorData, and adds it to the list.
    static BOOL
    MonitorEnumProc(HMONITOR handle, HDC, PRECT prcMonitor, LPARAM pThisPtr);

    // Called when the object is created and whenever the listener window gets
    // a WM_DISPLAYCHANGE or WM_SETTINGCHANGE/SPI_SETWORKAREA.
    // This refreshes all monitor data to reflect the current monitor topology.
    void RefreshMonitorData(bool force = false);

    // Called after refreshing the monitor data.
    // Notifies all windows that have registered for topology change notifications.
    void NotifyWindowsForTopologyChange();

    //
    // The listener window is a top level (but never visible) window used to
    // listen for WM_DISPLAYCHANGE and WM_SETTINGCHANGE/SPI_SETWORKAREA.
    // These messages are broadcast to all top level windows when the monitors
    // change.
    //

    static LRESULT CALLBACK
    ListenerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND CreateListenerWindow();

    //
    // The monitor topology data
    //

    UINT m_lastTopologyId = 0;
    UINT m_numMonitors = 0;

    // There are three monitor lists, one for each DPI awareness. If the
    // monitors have >100% DPI, the values (coordinates) for each monitor are
    // different depending on the thread's awareness.
    // This class uses the data queried at the same awareness as the thread.
    // This allows a thread to switch between awarenesses,
    // SetThreadDpiAwarenessContext, and still see correct information (the
    // same data that would be returned by MonitorFromRect and GetMonitorInfo).
    bool m_validDpiAwarenesses[NUM_DPI_AWARENESS] = {};
    std::vector<SharedMonitorPtr> m_monitorLists[NUM_DPI_AWARENESS];

    // The listener window.
    HWND m_hwndListener = nullptr;

    // Windows that have requested topology change notifications.
    // Used by RegisterTopologyChangeMessage.
    struct NotifyChangeRegistration
    {
        HWND hwndNotify;
        UINT msgId;
    };
    std::vector<NotifyChangeRegistration> m_notifyChangeRegistrations;
};

inline void
CurrentMonitorTopology::RegisterTopologyChangeMessage(
    HWND hwndNotify,
    UINT msgId)
{
    NotifyChangeRegistration info{};
    info.hwndNotify = hwndNotify;
    info.msgId = msgId;
    m_notifyChangeRegistrations.push_back(info);
}

inline void
CurrentMonitorTopology::NotifyWindowsForTopologyChange()
{
    for (const auto& registration : m_notifyChangeRegistrations)
    {
        SendMessage(registration.hwndNotify, registration.msgId, 0, 0);
    }
}

inline void
CurrentMonitorTopology::ForEachMonitor(ForEachMonitorFn fn)
{
    DPI_AWARENESS awareness = GetThreadDpiAwareness();

    // If the current thread is running at an awareness that we haven't
    // seen before, make this awareness valid and refresh the monitor data
    // with 'force' parameter. This forces us to reload the monitor data
    // at this awareness.
    if (!m_validDpiAwarenesses[awareness])
    {
        m_validDpiAwarenesses[awareness] = true;

        RefreshMonitorData(true /* force */);
    }

    // Call the function for each monitor in the list.
    for (const auto& monitor : m_monitorLists[awareness])
    {
        if (!fn(monitor))
        {
            break;
        }
    }
}

/* static */
inline BOOL
CurrentMonitorTopology::MonitorEnumProc(
    HMONITOR handle,
    HDC,
    PRECT,
    LPARAM pMonListPtr)
{
    // Get the data for this monitor.
    // This can fail (HMONITORs can become invalid at any time).
    MonitorData monData;
    if (!MonitorData::FromHandle(handle, &monData))
    {
        return false;
    }

    // Allocate a SharedMonitorPtr (std::shared_ptr<const MonitorData>).
    SharedMonitorPtr monitorData = SharedMonitorPtr(new MonitorData(monData));
    if (!monitorData)
    {
        return false;
    }

    // Add the shared monitor pointer to the list provided by the caller.
    auto pMonitorList = reinterpret_cast<std::vector<SharedMonitorPtr>*>(pMonListPtr);
    pMonitorList->push_back(monitorData);

    return true;
}

inline void
CurrentMonitorTopology::RefreshMonitorData(bool force)
{
    // There is a new API GetCurrentMonitorTopologyId, added in GE_RELEASE.
    //
    // We use this API if it is available to know the current topology ID.
    // This makes this ID global (shared with all apps).
    //
    // Using the system's ID also allows us to skip unneeded work when we get
    // several WM_DISPLAYCHANGE in a row. It's likely we receive the first one
    // after all changes to the monitors have been made, so we can throw out
    // subsequent messages by comparing the topology ID and finding it hasn't
    // changed.
    const UINT topologyId = GetCurrentMonitorTopologyId();
    if (topologyId)
    {
        // If called with force, we're being called because the thread changed
        // its DPI awareness, requiring that we rebuild the monitor lists
        // because the number of awarenesses we're maintaining has changed, not
        // because we think the monitors have changed.
        if (!force && (m_lastTopologyId == topologyId))
        {
            return;
        }
        m_lastTopologyId = topologyId;
    }
    else
    {
        // Fallback (topology ID API not available): Increment the ID every
        // time we receive a display change or setting change.
        // This ID could be used by someone using this class in the same way
        // that we use the API when it is available (to know if anything about
        // the monitors may have changed between two points in time).
        m_lastTopologyId++;
    }

    bool succeeded = true;

    m_numMonitors = GetSystemMetrics(SM_CMONITORS);

    DPI_AWARENESS_CONTEXT dpiPrev = GetThreadDpiAwarenessContext();
    for (UINT i = 0; i < NUM_DPI_AWARENESS; i++)
    {
        // We only maintain the list for this awareness if the awareness is
        // valid (if we've seen the thread running at this awareness before).
        if (!m_validDpiAwarenesses[i])
        {
            continue;
        }

        // Switch the thread to this awareness.
        SetThreadDpiAwarenessContext(
            (i == 0) ? DPI_AWARENESS_CONTEXT_UNAWARE :
            (i == 1) ? DPI_AWARENESS_CONTEXT_SYSTEM_AWARE :
                       DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        // TODO: Build new data in a temp list and swap it with the real list
        // iff successful? (avoiding ever leaving the state invalid, without
        // any monitors)

        // Delete the previous monitors in the list.
        m_monitorLists[i].clear();

        // Build the monitor list.
        if (!EnumDisplayMonitors(nullptr, nullptr,
                MonitorEnumProc, reinterpret_cast<LPARAM>(&m_monitorLists[i])))
        {
            succeeded = false;
        }
    }
    SetThreadDpiAwarenessContext(dpiPrev);

    // If we succeeded but the topology ID has changed since we started, the
    // data we've gathered is invalid. Don't treat this as a success. We know
    // in this case (and other failures) that it was caused by a mode change
    // that we haven't yet been notified for (and hopefully the next time we
    // try will be very soon and will succeed).
    if (succeeded &&
        topologyId &&
        (topologyId != GetCurrentMonitorTopologyId()))
    {
        succeeded = false;
    }

    // Iff the operation succeeded, notify anyone that has registered for
    // topology change messages.
    if (succeeded)
    {
        NotifyWindowsForTopologyChange();
    }
}

inline SharedMonitorPtr
CurrentMonitorTopology::MonitorFromPoint(
    POINT pt,
    Fallback fallback)
{
    SharedMonitorPtr candidate = nullptr;
    double nearestDistance = 0;

    ForEachMonitor([&](SharedMonitorPtr monitor)
    {
        RECT rc = monitor->monitorRect;

        // If the point is on this monitor, pick it.
        if (PtInRect(&rc, pt))
        {
            candidate = monitor;

            // Don't continue walking the monitor list.
            return false;
        }

        // If fallback is nearest, keep track of the nearest monitor.
        if (fallback == Fallback::Nearest)
        {
            // Measure the distance between the center of each monitor and the
            // point.
            //
            // Note: This is an approximation. A very wide monitor and very
            // small monitor that are both close to a point would prefer the
            // smaller monitor (because its center is closer to the point,
            // even if the wider monitor's edge is really closer).
            //
            // See the MonitorFromPoint implementation (internal):
            // MONITORFROMPOINTALGORITHM, onecoreuap/windows/core/ntuser/rtl/mmrtl.cxx
            // https://microsoft.visualstudio.com/DefaultCollection/OS/_git/0d54b6ef-7283-444f-847a-343728d58a4d?path=%2fonecoreuap%2fwindows%2fcore%2fntuser%2frtl%2fmmrtl.cxx&version=GBofficial/ge_current_directadept_hip1&line=126&lineEnd=126&lineStartColumn=8&lineEndColumn=34&lineStyle=plain

            POINT ptCenter = {
                rc.left + (RECTWIDTH(rc) / 2),
                rc.top + (RECTHEIGHT(rc) / 2) };

            double distanceSquared =
                std::pow(pt.x - ptCenter.x, 2) + std::pow(pt.y - ptCenter.y, 2);

            if (!candidate || (nearestDistance > distanceSquared))
            {
                candidate = monitor;
                nearestDistance = distanceSquared;
            }
        }

        // Keep walking the monitor list.
        return true;
    });

    if (candidate)
    {
        return candidate;
    }

    switch (fallback)
    {
        case Fallback::Nearest:
            // Assert(false)
            // We're guarenteed >1 monitors and we should have picked some
            // monitor as the nearest (candidate).

        case Fallback::Primary:
            return GetPrimaryMonitor();

        default: /* Fallback::Null */
            return nullptr;
    }
}

inline SharedMonitorPtr
CurrentMonitorTopology::MonitorFromRect(
    const RECT& rc,
    Fallback fallback)
{
    SharedMonitorPtr candidate = nullptr;
    UINT candidateArea = 0;
    const UINT rectArea = RECTWIDTH(rc) * RECTHEIGHT(rc);

    // Intersect the rect with each monitor rect, looking for the monitor
    // that intersects with the rect the most.
    ForEachMonitor([&](SharedMonitorPtr monitor)
    {
        RECT rcI;
        if (IntersectRect(&rcI, &monitor->monitorRect, &rc))
        {
            UINT area = RECTWIDTH(rcI) * RECTHEIGHT(rcI);

            // If the rect is entirely on this monitor, set the result and
            // stop the search.
            if (area == rectArea)
            {
                candidate = monitor;
                return false;
            }

            // If this monitor intersects with the rect more than the current
            // candidate, set this monitor as the candidate (but continue
            // walking the monitor list).
            if (area > candidateArea)
            {
                candidate = monitor;
                candidateArea = area;
            }
        }

        // Keep walking the monitor list.
        return true;
    });

    if (candidate)
    {
        return candidate;
    }

    switch (fallback)
    {
        case Fallback::Nearest:
            // Return the monitor that is closest to the center of the rect.
            //
            // TODO: This is an approximation. It may not be the best (closest)
            // monitor to the point. We could consider improving this.
            //
            // See MonitorFromRect implementation (internal):
            // MONITORFROMRECTALGORITHM, onecoreuap/windows/core/ntuser/rtl/mmrtl.cxx
            // https://microsoft.visualstudio.com/DefaultCollection/OS/_git/0d54b6ef-7283-444f-847a-343728d58a4d?path=%2fonecoreuap%2fwindows%2fcore%2fntuser%2frtl%2fmmrtl.cxx&version=GBofficial/ge_current_directadept_hip1&line=371&lineEnd=371&lineStartColumn=8&lineEndColumn=33&lineStyle=plain
            return MonitorFromPoint({
                rc.left + (RECTWIDTH(rc) / 2),
                rc.top + (RECTHEIGHT(rc) / 2)
            });

        case Fallback::Primary:
            return GetPrimaryMonitor();

        default: /* Fallback::Null */
            return nullptr;
    }
}

inline SharedMonitorPtr
CurrentMonitorTopology::MonitorFromName(PCWSTR deviceName)
{
    SharedMonitorPtr result = nullptr;

    ForEachMonitor([&](SharedMonitorPtr monitor)
    {
        if (monitor->MatchesDeviceName(deviceName))
        {
            result = monitor;
            return false;
        }

        // Keep walking the monitor list.
        return true;
    });

    return result;
}

inline SharedMonitorPtr
CurrentMonitorTopology::MonitorFromPastMonitor(
    const MonitorData& previousMonitor,
    Fallback fallback)
{
    // If any monitor has a matching display name, use that monitor.
    // Consider the case where the primary monitor changes. One monitor's
    // handle, rect, etc, may have changed, but still be around. The most
    // stable thing we know about the monitor is the device name string.

    SharedMonitorPtr result = MonitorFromName(previousMonitor.deviceName);

    if (result)
    {
        return result;
    }

    switch (fallback)
    {
        case Fallback::Nearest:
            // Return the monitor nearest to the previous monitor rect.
            return MonitorFromRect(previousMonitor.monitorRect, fallback);

        case Fallback::Primary:
            return GetPrimaryMonitor();

        default: /* Fallback::Null */
            return nullptr;
    }
}

/* static */
inline UINT
CurrentMonitorTopology::GetCurrentMonitorTopologyId()
{
    using pfnType = UINT(*)();
    static pfnType pfnGetTopologyId = nullptr;
    static bool loadedApi = false;

    // This public API is not in header files; load it dynamically.
    // https://learn.microsoft.com/en-us/windows/win32/winmsg/winuser/nf-winuser-getcurrentmonitortopologyid
    if (!loadedApi)
    {
        loadedApi = true;
        HMODULE hmod = LoadLibraryA("user32.dll");
        if (hmod)
        {
            pfnGetTopologyId = reinterpret_cast<pfnType>(
                GetProcAddress(hmod, "GetCurrentMonitorTopologyId"));
        }
    }

    if (pfnGetTopologyId)
    {
        return pfnGetTopologyId();
    }

    return 0;
}

/* static */
inline LRESULT CALLBACK
CurrentMonitorTopology::ListenerWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Store the instance pointer (CreateWindow params) in the window.
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(
            reinterpret_cast<LPCREATESTRUCTW>(lParam)->lpCreateParams));
        break;
    }

    // Most changes to the monitors will broadcast WM_DISPLAYCHANGE.
    // But, apps (or the taskbar) can change the work area directly, which
    // isn't considered a display change. To respond to these changes as
    // well, also listen for WM_SETTINGCHANGE/SPI_SETWORKAREA.
    case WM_SETTINGCHANGE:
        if (wParam != SPI_SETWORKAREA)
        {
            break;
        }
    case WM_DISPLAYCHANGE:
    {
        // Get the instance pointer, stored on the window.
        CurrentMonitorTopology* instance =
            reinterpret_cast<CurrentMonitorTopology*>(
                GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        // Refresh the monitor data.
        if (instance)
        {
            instance->RefreshMonitorData();
        }

        break;
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

inline HWND
CurrentMonitorTopology::CreateListenerWindow()
{
    PCWSTR windowTitle = L"TrackMonitorsListenerWindow";
    PCWSTR wndClassName = L"TrackMonitorsListenerWindowClass";
    HINSTANCE hInst = GetModuleHandle(NULL);

    static bool registered = false;
    if (!registered)
    {
        WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ListenerWndProc;
        wc.hInstance = hInst;
        wc.lpszClassName = wndClassName;

        if (!RegisterClassEx(&wc))
        {
            return nullptr;
        }

        registered = true;
    }

    // The listener window is mostly blank (no styles, position, etc),
    // but it has the topmost flag, WS_EX_TOPMOST. This moves the window
    // to the top, in Z-order (it is above other non-topmost windows).
    //
    // Broadcasted messages are received in Z-order, windows on top first.
    // If another window on this thread is listening for the same messages
    // and queries the monitor data from those messages, we want this
    // window to receive these messages first. Otherwise, the other window
    // could be left with stale monitor topology info.
    //
    // Note: RegisterTopologyChangeMessage allows a window to register for
    // updates from this class. Using that is strictly better than listening
    // to the broadcasted messages (but we make ourselves topmost anyway to
    // hopefully account for that usage).

    return CreateWindowEx(WS_EX_TOPMOST,
                          wndClassName,
                          windowTitle,
                          0,
                          0, 0, 0, 0,
                          nullptr,
                          nullptr,
                          hInst,
                          reinterpret_cast<PVOID>(this));
}
