#pragma once

// {83A1FDBC-AB3A-4376-A529-80E18C206534}
DEFINE_GUID(CLSID_DX11VideoRenderer, 0x83a1fdbc, 0xab3a, 0x4376, 0xa5, 0x29, 0x80, 0xe1, 0x8c, 0x20, 0x65, 0x34);

// {0743FA5C-DA9E-4760-8187-CCAC3DC15D77}
DEFINE_GUID(CLSID_DX11VideoRendererActivate, 0x743fa5c, 0xda9e, 0x4760, 0x81, 0x87, 0xcc, 0xac, 0x3d, 0xc1, 0x5d, 0x77);

// creation methods exposed by the lib
STDAPI CreateDX11VideoRenderer(REFIID riid, void** ppvObject);
STDAPI CreateDX11VideoRendererActivate(HWND hwnd, IMFActivate** ppActivate);
