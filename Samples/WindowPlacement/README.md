
# Remembering Window Positions with PlacementEx

This README is paired with the header files in this directory:

- [PlacementEx.h](PlacementEx.h)
- [MonitorData.h](MonitorData.h)
- [MiscUser32.h](MiscUser32.h)
- [CurrentMonitorTopology.h](CurrentMonitorTopology.h)
- [VirtualDesktopIds.h](VirtualDesktopIds.h)

These header files simplify storing & restoring window positions for apps. For
example, storing the position when a window closes and using it to pick an
initial position when the app launches. In this way, they are an extension of
[`SetWindowPlacement`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowplacement)
and
[`GetWindowPlacement`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowplacement).

The intended audience of this file is developers who are using or modifying
these headers in apps or frameworks.

These header files use only public APIs that have been stable since before
Windows 10 (and in most cases long before). For example:

 - [CreateWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa)
 - [SetWindowPos](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos)
 - [GetMonitorInfo](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmonitorinfoa)
 - [GetWindowPlacement](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowplacement)
 - [SetWindowPlacement](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowplacement)
 - [GetDpiForWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow)
 - [IsWindowArranged](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-iswindowarranged)
 - [GetDpiForMonitor](https://learn.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor)
 - [DwmGetWindowAttribute](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmgetwindowattribute)
 - [IVirtualDesktopManager](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ivirtualdesktopmanager)

## Overview

Most users expect apps to launch where they last closed. For example, apps
closed on secondary monitors should relaunch on that monitor (and not on the
primary). Or if maximized, apps should relaunch maximized (not at their restore
position).

Most apps today attempt to do this. The
[`SetWindowPlacement`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowplacement)
and
[`GetWindowPlacement`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowplacement)
APIs assist with this. But even when using these APIs correctly, sometimes apps
launch in unexpected places, especially for users with monitors that have
different DPI scales, or whose machines reboot or dock/undock frequently.

This happened because restore was initially (~Win3.1) fairly simple and grew in
complexity over many decades. **Multiple monitors**, **DPI**, **Arrangement**,
and **Virtual Desktops** in particular make positioning top-level windows more
complicated, which in turn makes storing window positions (and using them later)
more complicated. This README, and [PlacementEx.h](PlacementEx.h), introduce
fresh guidance for storing window positions.

## Background

### Screen Coordinates

Windows (HWNDs) parented to the Desktop window are called [**top-level windows**](https://learn.microsoft.com/en-us/windows/win32/winmsg/about-windows#parent-or-owner-window-handle).
These windows can move freely between all monitors. (As opposed to **child windows**,
which move relative to their parent).

The coordinate space of the Desktop window, the one top-level windows move within,
is called **Screen Coordinates**.

There is always a monitor connected to the system whose top-left corner is 0,0
in screen coordinates. This is the primary monitor.

Monitors to the left of the primary have negative X values, and ones above the
primary have negative Y values.

### Monitors (`HMONITOR`s)

APIs like [`MonitorFromPoint`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-monitorfrompoint) and
[`MonitorFromRect`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-monitorfromrect)
return a handle to a **monitor**, an `HMONITOR`. This handle can be used with APIs
like [`GetMonitorInfo`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmonitorinfoa)
and [`GetDpiForMonitor`](https://learn.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getdpiformonitor)
to read critical information about a monitor:

- **Monitor Rect**. This is the position and size (resolution) of the monitor.
- **Work Area**. This is a subset of the monitor rect that isn't covered by the
 taskbar (or other docked toolbars, like Voice Access).
- **DPI**. This is the scale for content on the monitor. See **DPI**, below.
- **Display Name**. This is a string uniquely identifying the monitor. This is
 available in `MONITORINFOEX` as `szDevice`.

See [MonitorData.h](MonitorData.h), which defines helpers to query the monitor
data.

### DPI

Since Vista, Windows supports custom **DPIs** (Dots Per Inch) and scale factors,
so that apps scale properly on high-resolution displays. In 8.1, this was
extended to allow each monitor to have its own DPI, and this has improved a few
times since. Windows scales 'DPI-unaware' apps by stretching them, while
'DPI-aware' apps/windows must scale themselves. This is typically done via a
**scale factor**, which is `(DPI) / 96`; a logical DPI of 144 would have a scale
factor of 144 / 96 = 150%.

**Logical** units refer to units that have been scaled by this scale factor,
whereas **physical** units refer to units that have not. For example, 12 logical
pixels on a 150% scale monitor correspond to 18 physical pixels. Screen
coordinates are in physical pixels.

Today, there are 3 types of [DPI awareness](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows). A window can be any one of these:

 - **DPI unaware**. The window renders itself at the Windows default of 96 DPI (scale factor 100%). Windows stretches this window bitmap to the actual DPI of the monitor.

 - **"System DPI" aware**. Previously called 'Aware'. The window receives the
   primary monitor's DPI at the time the process was launched, and must scale
   its UI appropriately. Windows stretches this window bitmap if the window
   moves to a monitor with a different DPI or if the primary monitor's DPI
   changes.

 - **"Per-Monitor DPI" aware**. The window is expected to handle
    [`WM_DPICHANGED`](https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged),
    which tells the window when its DPI scale should change (e.g. when moved
    between monitors). This provides the window a `RECT` that must be used to
    resize the window to the new DPI.

Consider a console window with some number of lines of text, or a browser window
with some number of paragraphs or pictures. If the window is Per-Monitor DPI
aware, and changes from 200% to 100% scaling without changing its window size
(but scales its *content* correctly), the content visible in the window would
double/quadruple. For example, a window that is 640 logical pixels wide would be
1280 physical pixels at 200% scaling, and should shrink down to 640 physical
pixels when moved to an 100% scale monitor.

It is possible for a thread to change its awareness, using [`SetThreadDpiAwarenessContext`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setthreaddpiawarenesscontext).
When a window is created, it is 'stamped' with the thread awareness at the
time, and the thread will automatically switch back to that awareness when
dispatching messages to the window. This allows a thread to create two windows
with different awarenesses, which means the coordinates seen by the two windows
will be different.

To know the DPI a window is currently scaling to: [`GetDpiForWindow`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow).

>![WARNING]
>
>Do not use `MonitorFromWindow` followed by `GetDpiForMonitor` to determine a
window's current DPI. This can be wrong while a window is being dragged between
monitors of different DPIs, or briefly while monitors are changing (for example
after the primary monitor changes but before the window is moved to stay on the
same monitor it was on, which is now in a different location).

## Window positioning concepts

### Maximize, Minimize, and Restore

The API `ShowWindow` can **Maximize**, **Minimize**, and **Restore** a window.

**Maximized** windows fit the monitor. By default, this will moves the window's
resize borders *outside* the work area. Indeed, moving the cursor to the top of
the monitor and dragging will begin *moving* a Maximized window, not resizing it
(although this will typically *cause* a resize as the window unmaximizes and
begins moving). And moving the cursor to the top-right corner of the monitor
should put it over the Maximized window's close button.

**Minimized** windows are normally off-screen and can be restored by clicking on
the Taskbar, or pressing Alt-Tab. On SKUs without a Shell (or a Taskbar), the
default Minimize position is the bottom-right of the monitor.

If the window is not in a special state like Maximized/Minimized, the window is
'normal', or **Restored**. When a normal window becomes Maximized/Minimized, its
previous position is saved (as the normal position). Restoring a window in a
state like Maximized/Minimized moves it back to this normal position.

The window styles `WS_MAXIMIZED` and `WS_MINIMIZED` are set when a window is in
these states. But using `SetWindowLongPtr` to set these styles directly will not
function correctly: it will not move the window to be maximized or cache its
normal/restore position. Use [`ShowWindow`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow) with something like `SW_MAXIMIZE`,
`SW_MINIMIZE`, or `SW_SHOWMINNOACTIVE` instead.

To check if a window is Maximized: [`IsZoomed`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-iszoomed)

To check if a window is Minimized: [`IsIconic`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-isiconic)

### Arranged (Snapped)

**Arranged** (or **Snapped**) windows are similar to Maximized windows. They have a normal
position that is different from their 'real' position (`GetWindowRect`), and the
real position is 'fit' to the monitor's work area.

While Maximized windows fill the entire work area, Arranged windows are aligned
with some number of edges of the work area. For example, left half, corners,
columns, etc. Dragging the Arranged window or double-clicking the title bar will
(like with Maximized windows) cause the window to restore to its normal size.

To check if a window is Arranged: [`IsWindowArranged`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-iswindowarranged)

Users can Arrange windows in many ways:

 - Drag a window and hit the edges of the monitor with the cursor
 - Drag a window and drop it onto the Snap Flyout that appears at the top of the screen
 - Hotkeys like Win+Left/Right
 - Snap another window and choose this window from Snap Assist
 - See [this MSDN page](https://support.microsoft.com/en-us/windows/snap-your-windows-885a9b1e-a983-a3b1-16cd-c531795e6241) for more info.

However, `GetWindowPlacement` (detailed below) does not handle Arranged windows.
Also, unlike Maximized/Minimized, there is no `ShowWindow` command (or other
API) to Arrange a window. There is a workaround for this, but it is not trivial.
It's based on 2 factors:

 - Double-clicking on a window's top (or bottom) resize area will Arrange
 the window. This aligns the top and bottom borders with the top/bottom of the
 work area.

 - Apps can move themselves while Arranged.

    This is true of Maximized as well, though it is not generally advised.

    You can call `SetWindowPos` to move a Maximized, Minimized, or Arranged
    window, but this will not change the window's state (styles) or normal
    position. Doing this without consulting the monitor information can leave
    the window in an unexpected position, possibly off-screen!

This means that you can do something like this to Arrange a window
and set its position:

```cpp
DefWindowProc(hwnd, WM_NCLBUTTONDBLCLK, HTTOP, 0);
SetWindowPos(hwnd, nullptr, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
```

Be careful when moving an Arranged or Maximized window:

 - Arranged positions should fit a work area, aligning visibly with the monitor
   edges.

    While you can move an Arranged or Maximized window into the center of the
    screen, this will look and behave strangely! The user will see unexpected
    window borders (ones for Max/Arranged windows), and moving the window will
    'restore' the window to a previous size, which could look unexpected.

    Normally, Arranged windows are perfectly aligned with 2 or 3 edges of the
    work area. For example, left half, top-left corner, or top/bottom.

 - When moving an Arranged rect between monitors, the relative distance to
    each edge of the monitor should be maintained. This is unlike the normal
    rect, which generally should retain its logical size.

 - Some windows have invisible resize areas.

    Simply positioning the window (`SetWindowPos`) to align with the edges of
    the monitor will leave visible gaps between the window and the monitor,
    because part of the window is invisible/transparent.

    To query the visible bounds of a window (the extended frame bounds):

    ```cpp
    RECT rcFrame;
    HRESULT hr = DwmGetWindowAttribute(hwnd,
        DWMWA_EXTENDED_FRAME_BOUNDS, &rcFrame, sizeof(rcFrame));
    ```

    This value can be compared with `GetWindowRect` to know the size of the
    window's invisible resize areas.

    Caveats:
     - The window should not be Maximized/Minimized when getting the frame
        bounds (the values are different in those states).
     - Changing a window's DPI (moving it between monitors) can change the
        size of the invisible area! Apps moving windows between monitors
        should move the window to the target monitor before querying the
        window's frame bounds.
     - This API does not handle DPI virtualization. If an app or window is
        virtualized for DPI (for example, a DPI unaware window), these values
        may not match those from GetWindowRect.

### Cascading

If an app launches multiple windows, or if the user launches an app multiple
times, it is common to **cascade** the windows. This means moving each window
down and right a bit, leaving the previous window's title bar visible. If the
new position is now outside the work area, the window is moved to the top or
left side of the same monitor.

The size of the 'nudge' is ideally the height of the window's caption bar.
This can be queried using system metrics:

```cpp
UINT captionheight =
    GetSystemMetricsForDpi(SM_CYCAPTION, dpi) +
    (2 * GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi));
```

Adding `2 * SM_CXSIZEFRAME` ensures the entire title bar from the previous
window is visible, since it includes the invisible resize area (which is part of
the title bar when not Maximized).

> ![NOTE]
>
> The similar function [`GetSystemMetrics`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetrics)
returns values for the *process* DPI. If an app is Per-Monitor DPI aware, it
should use [`GetSystemMetricsForDpi`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetricsfordpi)
and the DPI of the window, [`GetDpiForWindow`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow).

## Common factors when relaunching windows

### `Get*` and `SetWindowPlacement`

[GetWindowPlacement](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowplacement) and [SetWindowPlacement](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowplacement) are the main APIs intended to remember window placement between launches. They handle Maximized, Minimized, and Restored windows to an extent. However, they do not handle Arranged windows.

`GetWindowPlacement` returns a [`WINDOWPLACEMENT`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-windowplacement), which contains:

 - `rcNormalPosition` is the window position (the normal, restore position if
 Maximized, Minimized, or Arranged.)
 - `showCmd` is a `ShowWindow` command, `SW_MAXIMIZE`, `SW_MINIMIZE`, or `SW_NORMAL`.
 - `flags` can be `WPF_RESTORETOMAXIMIZED` (and others).
 - **Note:** It is not recommended to use the other fields, like ptMinPosition.

[`SetWindowPlacement`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowplacement) accepts this `WINDOWPLACEMENT` struct and
sets a window's normal position and Maximize state. Internally, this calls [`SetWindowPos`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos)
and [`ShowWindow`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow).

When these APIs were created, prior to multiple monitor support, apps could
`GetWindowPlacement` on exit, store the `WINDOWPLACEMENT` to the registry, and
`SetWindowPlacement` when launching, and the app successfully launched where it
was when last closed in all cases. Today, doing this (and nothing else to
adjust the position) would lead to the app sometimes being the wrong size or
partially off-screen.

> ![NOTE]
> The normal rect is in 'workspace coordinates', which is offset by the space
between the window's monitor's work area and monitor rect. These coordinates
match screen coordinates, even on secondary monitors, unless the taskbar is on
the top or left.

### The `STARTUPINFO`

There is a struct called [**`STARTUPINFO`**](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-startupinfoa),
which contains flags set by whoever launched the process, for example by the
taskbar or start menu. Apps can query these flags using [`GetStartupInfo`](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getstartupinfow). A few `STARTUPINFO` flags relate to the position of the app's first window.

 - `STARTF_USESHOWWINDOW` is set if the caller requested that the window launch
 `SW_MAXIMIZE` or `SW_MINIMIZE`. Note that apps can 'overrule' this if the app last
 closed Maximized and is being launched with `SW_NORMAL`.

   For example, from cmd:
    ```cmd
    > start /min notepad
    ```

   From PowerShell:
    ```powershell
    > start -WindowStyle Minimized notepad
    ```

 - The `hStdOutput` can, for non-console apps, be an `HMONITOR`. This is the
 'Monitor Hint.' If launched from the Start Menu or Taskbar on a secondary
 monitor, this `HMONITOR` will be the monitor the user clicked on to launch the
 app.

 - Less used/useful are `STARTF_USEPOSITION` and `STARTF_USESIZE`. These specify the
 position and size of the first window.

### `CreateWindow` parameters

[**`CreateWindow`**](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa)
takes an: `x`, `y`, `nWidth`, `nHeight`, and styles. (And other parameters not
discussed here.)

If the position and size parameters are ALL `CW_USEDEFAULT` (a special value),
the window will be in the default position:

- On the primary, unless a monitor hint is set in `STARTUPINFO`.
- Cascading: at a position a bit down/right from the last window to get the default
 position on that monitor, see above.
- A size picked from the size of the monitor (likely too large on very large
 monitors).

Note that if the size parameters are set, the window will *always* launch on the
primary and at the requested size (which is assumed scaled to the DPI of the
primary monitor).

If a process is launched with the `STARTUPINFO` show command set, that is applied
to the first window the app makes visible (matching some criteria, like `WS_CAPTION`).

Since these parameters must all be `CW_USEDEFAULT` for the magic behavior to
occur, when restoring window positions (including Maximize state, size, and
monitor), it is recommended to create the window hidden (without `WS_VISIBLE`)
and in the default position (`CW_USEDEFAULT`). Then, after the window is
created, the app should move the window to its correct position (which in some
cases requires moving multiple times), and only show the window when it is in
the final position.

Providing a size or position to `CreateWindow` requires querying the monitor
information and pre-scaling the size to the DPI of the monitor (and adjusting
it to ensure the position is on screen). If Maximizing/Minimizing, this would
be translated to the styles field (`WS_MAXIMIZE`/`WS_MINIMIZE`).

Apps can also position themselves within `WM_CREATE`. Apps can pick an initial
size here (and call `SetWindowPos`), but calling `ShowWindow` or
`SetWindowPlacement` to Maximize the window can cause problems! After
`WM_CREATE` returns, if the window has `WS_MAXIMIZE`/`WS_MINIMIZE` styles, it is
assumed that the window was created with these styles (and its current position
is the normal position). If the window has been Maximized already at this point,
restoring it will not move the window (its normal position was set to the
Maximize position). This is (much) worse if done for Minimize, since it could
cause the window to be stuck off screen.

## Restarting windows after updates/crashes

There are also several other factors to consider when apps *restart themselves*
(e.g. after updating themselves) or are restarted automatically (e.g. after a
system update causes a reboot).

### Relaunching Minimized

If an app is closed while the window is Maximized, it is ideal to launch
Maximized. But, if closed while Minimized, for example via Taskbar/Alt-Tab, the
window should typically launch to its restore position. (As a bonus, you can use
`WPF_RESTORETOMAXIMIZED` to launch Maximized if the window was Maximized prior
to Minimizing and closing.)

BUT, what if the app restarts itself? If a window is Minimized while the app
restarts, the app should relaunch the window Minimized, 'staying Minimized'
over the app restart.

Similarly, if the system reboots while the app is open and minimized, the window
should be Minimized after reboot. (In other words, if a machine reboots
overnight, all Minimized windows should not be restored from Minimize).

As a general rule: when storing positions, remember the Minimize state. When
launching normally, this state should be ignored (the window should launch to
the normal position, or Maximized if WPF_RESTORETOMAXIMIZED). But when
*relaunching* after a reboot or crash, relaunch to Minimized if it was
previously Minimized.

>![NOTE]
>
> A previous section described `STARTUPINFO`, which allows users to launch an
appwith an explicit show command, e.g. `SW_MINIMIZE`. If launched in this way,
like `start /min`, the app should launch Minimized regardless of the last close
position.

### Virtual Desktops

Win+Tab and Taskview button on the taskbar allow the user to create
multiple [**Virtual Desktops**](https://support.microsoft.com/en-us/windows/configure-multiple-desktops-in-windows-36f52e38-5b4a-557b-2ff9-e1a60c976434) (sometimes called **Desktops**). These are groups
of windows that the user can switch between. When a window is on a background
virtual desktop, it is cloaked (hidden).

Apps generally do not need to worry about virtual desktops, including for most
scenarios related to restoring window positions. In a normal app launch, it is
always the case that the window should launch on the current virtual desktop
(even if closed while on a background desktop).

But, when restarted (automatic app/sytstem update, etc), apps should ideally
'keep' their windows on the same virtual desktops. Consider an app like a
browser that creates many windows, and a user has organized them onto different
virtual desktops. If the system reboots and the app is restarted, it should
restore all window positions, *and* it should move each window to its previous
virtual desktop.

This is done using the [`IVirtualDesktopManager`](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ivirtualdesktopmanager) APIs, which has these two functions:
 - [`GetWindowDesktopId`](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ivirtualdesktopmanager-getwindowdesktopid)
 - [`MoveWindowToDesktop`](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ivirtualdesktopmanager-movewindowtodesktop)

When storing previous window positions, apps should remember the GUID for the
window's virtual desktop. This is used ONLY when the app is restarted, not in a
'normal launch'. This is similar to minimized; if closed while minimized and
launched normally, the app should start restored (or maximized).

### Restarting after Windows Update with `RegisterApplicationRestart`

If you search for 'Restart apps after signing in' from the start menu, you'll
find a user setting (which is off by default). If enabled, apps should [relaunch
themselves if opened while the system
reboots](https://blogs.windows.com/windowsdeveloper/2023/07/20/help-users-resume-your-app-seamlessly-after-a-windows-update/).
Your app should call [`RegisterApplicationRestart`](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-registerapplicationrestart), which accepts a parameter
that is passed as a command line argument when the system relaunches your app
after a reboot.

For example:

```cpp
// Register your app for restart; you can control which types of restart with flags.
PCWSTR restartCmdLine = L"restart";
DWORD flags = 0;
RegisterApplicationRestart(restartCmdLine, flags);

// Detect a restart by checking for the argument
const bool isRestart = (wcsstr(cmdLine, restartCmdLine) != nullptr);
```

If your app receives this command line argument, you know your app is restarting. You should use this as a signal to reuse previous minimized state and virtual desktops.

Note that when closing, apps should handle `WM_ENDSESSION` to know when they are
being closed in all cases. This message is sent prior to destroying the window
when the system is shutting down.

## Example: putting it all together

We now have the background to correctly relaunch windows in their previous
locations.

We'll need this stored information to do so correctly:

* the window's last **normal position** (a `RECT` in screen coordinates)
* potentially its last **Arranged position** (also in screen coordinates)
  * If a window is Arranged, it has both a normal rect and an Arranged rect. (If
    Max/Min/Normal, the Arranged rect is not set.)
* some information about the window's last monitor
  * its **work area**
  * its **DPI**
  * its **device name** (`szDevice` from `MONITORINFOEX`)

### Moving stored positions between monitors

First, we need an algorithm for translating stored positions between monitors.
This is necessary if the stored position is from a monitor that no longer exists
or changed in some way. Given the stored information above and a new monitor
with its own work area & DPI, we can adjust our `RECT`s:

* **The normal rect**

   1. Adjust the rect to stay the same logical offset from its work area.
      (Calculate the logical offset by calculating the offset in physical pixels
      and then scaling by the DPI).
   2. Scale the rect's size from the previous DPI to the new DPI, keeping the same
      logical size.
   3. If the rect is now larger than the work area of its new monitor, pick a
      different size. Preferably, scale the original size based on the ratio
      between the old work area to the new one.
   4. Crop the rect as needed to remain entirely within the work area.

* **The Arrange rect**
   - The Arranged rect should be stored in 'frame bounds' (without invisible
      resize borders). The size of the invisible resize borders change when the
      DPI changes, and this size does not scale linearly. To ensure Arranged
      positions keep windows visibly aligned when their DPI changes while
      Arranged, it is necessary to remove the invisible resize borders prior to
      storing the rect, and add them back in when Arranging the window, after
      the window is already on the monitor it is being Arranged on.
   - The Arranged rect always is sized to stay the same relative distance from
      each edge of the work area.

Note that it is not necessary to handle the Maximize/Minimize positions when
moving the other RECTs to a different monitor. These positions are chosen
depending on the monitor a window is on when it is Maximized/Minimized, and apps
can handle messages like `WM_GETMINMAXINFO` to override these default positions.

### Moving a window to a stored position

Next, we need an algorithm for moving a window to a desired position on any
monitor.

Let's go step by step. Given the same stored information plus a **show window
command** (`SW_*`):

1. **Pick a monitor.**

   To pick the best monitor in all cases, it is recommended to store the device
   name (`MONITORINFOEX`'s `szDevice`), in addition to the work area. In cases
   like changing the primary monitor, this results in better behavior than
   reusing the work area (since a user changing the primary monitor likely does
   not think of it as a change in coordinate space).

   So:

    - If a monitor exists with the same name, use that monitor.

    - If that's not available, fall back to `MonitorFromRect` with
      `MONITOR_DEFAULTTONEAREST`.

2. **Adjust the normal rect** (and Arrange rect if Arranged) for this desired
   monitor.

    - Each rect (normal/arranged) is transformed in a different way.

    - See previous section.

3. **Disable painting & animations**, if needed.

    - In some cases, it will be necessary to move the window multiple times. To
      avoid flickering, temporarily disable painting and animations.

    - Calling `WM_SETREDRAW` with `false` for `wParam` disables painting for the
      window. While disabled, the window's contents on screen will not be
      changed (though the window will not be hidden, if it is visible). When
      re-enabling painting (`WM_SETREDRAW` `true`) at the end, the window will need
      to be explicitly invalidated (repainted) and activated, since both are
      skipped while `WM_SETREDRAW` false.

    - `SystemParametersInfo`'s `SPI_SETANIMATION` sets a global user setting for
      animating window transitions (Max/Min/Arrange/Restore). If moving the
      window multiple times, it is best to temporarily disable animations. This
      ensures the window isn't shown animating from a position the user didn't
      see the window at.

      ```c++
      DefWindowProc(hwnd, WM_SETREDRAW, 1, 0);

      ANIMATIONINFO animationInfo = { sizeof(ANIMATIONINFO) };
      SystemParametersInfo(SPI_GETANIMATION, 0, &animationInfo, 0);
      bool needsAnimationReset = !!animationInfo.iMinAnimate;
      if (needsAnimationReset)
      {
         animationInfo.iMinAnimate = false;
         SystemParametersInfo(SPI_SETANIMATION, 0, &animationInfo, 0);
      }

      // ... move window multiple times

      DefWindowProc(hwnd, WM_SETREDRAW, 1, 0);
      SetActiveWindow(hwnd);
      InvalidateRect(hwnd, nullptr, true);

      if (needsAnimationReset)
      {
         animationInfo.iMinAnimate = true;
         SystemParametersInfo(SPI_SETANIMATION, 0, &animationInfo, 0);
      }
      ```

4. **Restore the window**. If a window is Maximized, Minimized, or Arranged, the
      window needs to be restored (SW_RESTORE) before it is moved to the desired
      normal position.

    - The normal position is defined as the last position the window had before
         Maximizing/Minimizing/Arranging.

    - If Maximized and moving to another monitor (but staying Maximized), the
         window must be restored first, moved to the other monitor, and then
         Maximized. Moving only once would leave the normal position on the
         previous monitor (restoring the window would move it to another monitor).

5. **Move the window to the correct monitor**. If a window is changing monitors, it must be moved to that monitor prior to
      maximizing/minimizing.

    - This is especially important when changing DPI, since moving a window
         between monitors of different DPIs causes the window to change size
         (via WM_DPICHANGED).

    - It is fine to simply SetWindowPos with the desired normal RECT, expecting
         that the window may receive a WM_DPICHANGED with a size that is scaled
         by some amount (from the window's current DPI to the new monitor's DPI).
         The next time the window is moved it will already be at the right DPI and
         not have the size scaled.

    - One 'gotcha' with the above is that if moving to a higher DPI, this
         'unwanted WM_DPICHANGED' RECT could potentially move the window onto some
         other monitor! (If moving to the bottom right of a high DPI monitor and
         we find that another monitor is to the right/below.) To account for this,
         scale the temp RECT from the monitor DPI to the window DPI, ensuring that
         when the system scales the RECT in the opposite direction, the final size
         and position match the stored (and now modified) normal RECT.

6. Once on the desired monitor, call `SetWindowPlacement` to set the desired
      normal position and show state.

7. If Arranging, the show command with SetWindowPlacement is SW_NORMAL. We need
      to make the window arranged and move it to the Arranged position.
      (SetWindowPlacement doesn't support Arranged.)

    ```c++
      DefWindowProc(hwnd, WM_NCLBUTTONDBLCLK, HTTOP, 0);

      // The Arranged RECT is stored in frame bounds (no invisible resize
      // borders). It has already been 'moved' (fit) to the monitor we're moving
      // to, staying aligned with the edges of the work area.
      // Now that the window is on the monitor it is moving to, and Normal (not
      // Maximized/Minimized), query the size of the invisible resize borders
      // and expand the Arranged RECT by that size.
      RECT rcWindow, rcFrame;
      GetWindowRect(hwnd, &rcWindow);
      DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rcFrame, sizeof(rcFrame));
      arrangeRect->left -= rcFrame.left - rcWindow.left;
      arrangeRect->top -= rcFrame.top - rcWindow.top;
      arrangeRect->right += rcWindow.right - rcFrame.right;
      arrangeRect->bottom += rcWindow.bottom - rcFrame.botto;

      SetWindowPos(hwnd,
         nullptr,
         arrangeRect.left,
         arrangeRect.top,
         arrangeRect.right - arrangeRect.left,
         arrangeRect.bottom - arrangeRect.top,
         SWP_NOZORDER | SWP_NOACTIVATE);
    ```

### Remembering Window Positions

Now we can stitch this all together so that your app will relaunch wherever it
was when last closed:

1. The app defines some persisted data store, like a registry path.

    ```cpp
    PCWSTR appRegKeyName = L"SOFTWARE\\UniqueNameForThisApp";
    PCWSTR lastCloseRegKeyName = L"LastClosePosition";
    ```

2. Before creating the window, check if another instance of the app is already
   running.

   - Note: [`FindWindow`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-findwindowa)
returns the top window in z-order if multiple other instances are running.

    ```c++
    HWND hwndPrev = FindWindow(wndClassName, nullptr);
    ```

3. Create the window without `VS_VISIBLE` and with `CW_USEDEFAULT` position and size.

    ```c++
    HWND hwnd = CreateWindowEx(
        ...
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ...);

    if (!hwnd)
    {
        // Handle failure.
    }
    ```

4. Set the initial position.

    - If there was a previous window position, start with that window's position.
    - If that window is Minimized, use the restored position.
    - Cascade the position, moving it down/right to keep both windows title bars visible.
    - If no previous window, check the last close position.
    - If no last close position, set some hard coded logical size reasonable for
         the app to use by default.
    - If the STARTUPINFO has relevant flags set, adjust the position picked above
         as needed.
    - Set the window position and show the window.

3. Call `RegisterApplicationRestart`.

    - Do this after a successful launch. If the app is running while the system
      shuts down for updates, it will then be relaunched after the system reboots.

    ```c++
        RegisterApplicationRestart(L"", 0);
    ```

4. When the window gets `WM_CLOSE`, store the window position in the registry.

   ```c++
   // ...wndproc...
   case WM_CLOSE:
      PlacementEx::StorePlacementInRegistry(
            hwnd,
            appRegKeyName,
            lastCloseRegKeyName);
      break;
   ```

## More scenarios

### FullScreen windows

All previous sections describe 4 possible window states, Max/Min/Arrange/Normal.
In these four states, all required information to capture a window's position is
known to the system (and exposed via APIs like `GetWindowPlacement` and `GetDpiForWindow`).

FullScreen is another (separate) state:

 - A FullScreen window does not have the `WS_CAPTION` or `WS_THICKFRAME` styles.
      (It has no title bar or resize borders).
 - A FullScreen window fits the monitor rect, covering the taskbar (if one is
      present on the monitor).

Some uncommon knowledge about FullScreen windows:

 - They are NOT always above other apps (z-order), or covering all monitors.
 - They can be Maximized or Minimized. (ex, maximize a browser window then F11
      twice. This enters FullScreen and exits back to Maximized).
 - They can change monitors. (ex, unplug a FullScreen window's monitor, or
      change the resolution of the monitor a FullScreen window is on).

The main difference between FullScreen and Maximize is that the system does
not know all of the state for the window (only the app knows the window's restore
from FullScreen position).

If an app becomes FullScreen, and is FullScreen when it is closed, it should NOT
store the FullScreen position (the one sized to the monitor). Launching to this
size, without being FullScreen (and without knowing the right restore position
upon exiting FullScreen) would lead to unexpected behavior. Instead, apps that
enter FullScreen must remember their position prior to becoming FullScreen. They
can use this as their "restore position" when exiting full screen, and they
should then store this as their "current position" when storing placement .

   ```c++
      case WM_DESTROY:
         if (WI_IsFlagSet(placement.flags, PlacementFlags::FullScreen))
         {
            placement.MoveToMonitor(hwnd);
         }
         else
         {
            PlacementEx::GetPlacement(hwnd, &placement);
         }

         placement.StoreInRegistry(registryPath, lastCloseRegKeyName);
         break;
   ```


### Handling monitor changes

After an app creates a window, the monitors can change at any time! This means
that screen coordinates that are 'on-screen' (within the bounds of some monitor)
could become off-screen if that monitor is removed, or changes size.

Most apps care about 2 "monitor" events:

* `WM_DISPLAYCHANGE`: sent to all top-level windows when monitors change.
* `WM_SETTINGCHANGE` (for `SPI_SETWORKAREA`): sent when work area changes. This is important: if a new docked toolbar appears (e.g. Voice Access), only this event is fired, not `WM_DISPLAYCHANGE`.

DANGER: If an app moves itself in response to WM_DISPLAYCHANGE, it may end up
off screen or in unexpected places!

 - A user has two monitors connected to a laptop, using two ports on the
 laptop. The user unplugs one cable, then immediately unplugs the other.

 - It is possible that the first display change moves the window to the monitor
 that is about to be removed. If this happens, the window may be moved twice.

 - It is possible that the window receives the first WM_DISPLAYCHANGE after
 being moved once, but after that monitor it was moved to was removed. During
 this time, the window moving itself would cause the system to NOT move the
 window the second time.

 - The final state of the window becomes hard to define. If the user expects
 the system's behavior (like moving the window back to a monitor it was on when
 the monitor was unplugged), the window might end up on the wrong monitor. And
 if the app only sizes itself, but does not check that it's position is on
 screen, this could cause the window to end up off screen.

It is important to handle failures when using HMONITOR APIs, even when using
MONITOR_DEFAULTTONEAREST. (The handle this call returns will never be null, but
it could become invalid before it is used.)

```cpp
MONITORINFOEX mi { sizeof(mi) };
if (!GetMonitorInfo(MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST), &mi))
{
    // Return without moving the window. The monitors are changing.
    return;
}
```

Note: Apps sometimes query the monitors very frequently, which can make these
'very rare' error cases difficult to handle, or expensive to compute. See
[CurrentMonitorTopology.h](CurrentMonitorTopology.h), which defines a cache
for the monitor data that addresses these issues.