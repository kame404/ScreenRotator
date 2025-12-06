#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif
#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif
#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>
#define WM_TRAYICON         (WM_USER + 1)
#define ID_TRAYICON         1
#define IDM_ABOUT           1001
#define IDM_EXIT            1002
#define SHELL32_ICON_INDEX  15
#define CLASS_NAME          L"ScreenRotatorClass"
#define MUTEX_NAME          L"Global\\ScreenRotatorMutexApp"
#define APP_NAME            L"Screen Rotator"
#define APP_VERSION         L"1.0"
#define TARGET_MOUSE    0
#define TARGET_PRIMARY  1
#define TARGET_INDEX    2
static HWND g_hwnd = NULL;
static HICON g_icon = NULL;
static NOTIFYICONDATA g_nid;
static BOOL g_initialized = FALSE;
static BOOL g_trayIconAdded = FALSE;
static void ShowError(LPCWSTR message) {
    MessageBox(NULL, message, APP_NAME, MB_OK | MB_ICONERROR);
}
static void ShowNotification(LPCWSTR title, LPCWSTR message, DWORD iconType) {
    if (!g_hwnd || !g_trayIconAdded) return;
    g_nid.uFlags |= NIF_INFO;
    StringCchCopy(g_nid.szInfoTitle, ARRAYSIZE(g_nid.szInfoTitle), title);
    StringCchCopy(g_nid.szInfo, ARRAYSIZE(g_nid.szInfo), message);
    g_nid.dwInfoFlags = iconType;
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
    g_nid.uFlags &= ~NIF_INFO;
}
static void MinimizeMemory(void) {
    SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
}
static BOOL GetDeviceNameByIndex(int targetIndex, WCHAR *outDeviceName, size_t bufSize) {
    DISPLAY_DEVICE dd;
    int monitorCount = 0;
    int i;
    
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);
    
    for (i = 0; EnumDisplayDevices(NULL, i, &dd, 0); ++i) {
        if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            if (++monitorCount == targetIndex) {
                StringCchCopy(outDeviceName, bufSize, dd.DeviceName);
                return TRUE;
            }
        }
    }
    return FALSE;
}
static HMONITOR GetTargetMonitor(int targetMode) {
    POINT pt;
    if (targetMode == TARGET_MOUSE) {
        GetCursorPos(&pt);
        return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    }
    pt.x = 0; pt.y = 0;
    return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
}
static void ApplyRotation(LPCWSTR deviceName, DWORD orientation) {
    DEVMODE dm;
    BOOL currentIsPortrait, targetIsPortrait;
    DWORD temp;
    
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    if (!EnumDisplaySettings(deviceName, ENUM_CURRENT_SETTINGS, &dm)) {
        ShowNotification(L"Error", L"Failed to get display settings.", NIIF_ERROR);
        return;
    }
    if (dm.dmDisplayOrientation == orientation) return;
    currentIsPortrait = (dm.dmDisplayOrientation == DMDO_90 || dm.dmDisplayOrientation == DMDO_270);
    targetIsPortrait = (orientation == DMDO_90 || orientation == DMDO_270);
    if (currentIsPortrait != targetIsPortrait) {
        temp = dm.dmPelsWidth;
        dm.dmPelsWidth = dm.dmPelsHeight;
        dm.dmPelsHeight = temp;
    }
    dm.dmDisplayOrientation = orientation;
    dm.dmFields = DM_DISPLAYORIENTATION | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
    if (ChangeDisplaySettingsEx(deviceName, &dm, NULL, CDS_UPDATEREGISTRY, NULL) != DISP_CHANGE_SUCCESSFUL) {
        ShowNotification(L"Rotation Failed", L"Graphics driver did not accept the settings.", NIIF_ERROR);
    }
}
static void RotateScreen(DWORD orientation, int targetMode, int targetIndex) {
    WCHAR deviceName[32];
    MONITORINFOEX mi;
    HMONITOR hMonitor;
    if (targetMode == TARGET_INDEX) {
        if (!GetDeviceNameByIndex(targetIndex, deviceName, ARRAYSIZE(deviceName))) {
            ShowNotification(L"Error", L"Monitor index not found.", NIIF_ERROR);
            return;
        }
    } else {
        hMonitor = GetTargetMonitor(targetMode);
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(MONITORINFOEX);
        if (!GetMonitorInfo(hMonitor, (LPMONITORINFO)&mi)) {
            ShowNotification(L"Error", L"Failed to get monitor info.", NIIF_ERROR);
            return;
        }
        StringCchCopy(deviceName, ARRAYSIZE(deviceName), mi.szDevice);
    }
    ApplyRotation(deviceName, orientation);
}
static void RegisterHotKeys(void) {
    int failCount = 0;
    if (!RegisterHotKey(g_hwnd, 1, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_UP)) ++failCount;
    if (!RegisterHotKey(g_hwnd, 2, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_DOWN)) ++failCount;
    if (!RegisterHotKey(g_hwnd, 3, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_LEFT)) ++failCount;
    if (!RegisterHotKey(g_hwnd, 4, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_RIGHT)) ++failCount;
    if (failCount > 0) {
        ShowNotification(L"Hotkey Warning", L"Some hotkeys failed to register.", NIIF_WARNING);
    }
}
static BOOL InitializeTrayIcon(void) {
    g_icon = ExtractIcon(GetModuleHandle(NULL), L"shell32.dll", SHELL32_ICON_INDEX);
    if (!g_icon) g_icon = LoadIcon(NULL, IDI_APPLICATION);
    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hwnd;
    g_nid.uID = ID_TRAYICON;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = g_icon;
    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), APP_NAME);
    if (!Shell_NotifyIcon(NIM_ADD, &g_nid)) {
        ShowError(L"Failed to create tray icon.");
        return FALSE;
    }
    g_trayIconAdded = TRUE;
    return TRUE;
}
static BOOL Initialize(HWND hwnd) {
    if (g_initialized) return TRUE;
    g_hwnd = hwnd;
    if (!InitializeTrayIcon()) return FALSE;
    RegisterHotKeys();
    g_initialized = TRUE;
    return TRUE;
}
static void Cleanup(void) {
    int i;
    if (!g_initialized) return;
    for (i = 1; i <= 4; ++i) {
        UnregisterHotKey(g_hwnd, i);
    }
    if (g_trayIconAdded) {
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
        g_trayIconAdded = FALSE;
    }
    if (g_icon) {
        DestroyIcon(g_icon);
        g_icon = NULL;
    }
    g_initialized = FALSE;
}
static void ShowContextMenu(void) {
    POINT pt;
    HMENU hMenu;
    
    GetCursorPos(&pt);
    hMenu = CreatePopupMenu();
    if (!hMenu) return;
    AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"About Screen Rotator");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");
    SetForegroundWindow(g_hwnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, g_hwnd, NULL);
    DestroyMenu(hMenu);
}
static void ShowAboutDialog(void) {
    MessageBox(g_hwnd,
        L"Screen Rotator v" APP_VERSION L"\n\n"
        L"- Ctrl+Alt+Arrow: Rotate monitor under cursor.\n"
        L"- CLI: /up /down /left /right [/display:N]",
        L"About", MB_OK | MB_ICONINFORMATION);
}
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        if (!Initialize(hwnd)) return -1;
        break;
    case WM_HOTKEY:
        switch (wParam) {
            case 1: RotateScreen(DMDO_DEFAULT, TARGET_MOUSE, 0); break;
            case 2: RotateScreen(DMDO_180, TARGET_MOUSE, 0); break;
            case 3: RotateScreen(DMDO_270, TARGET_MOUSE, 0); break;
            case 4: RotateScreen(DMDO_90, TARGET_MOUSE, 0); break;
        }
        break;
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) ShowContextMenu();
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDM_ABOUT: ShowAboutDialog(); break;
            case IDM_EXIT: DestroyWindow(hwnd); break;
        }
        break;
    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
static void ShowHelpDialog(void) {
    MessageBox(NULL,
        L"Usage:\n  ScreenRotator.exe [option] [monitor]\n\n"
        L"Options:\n  /up, /down, /left, /right\n\n"
        L"Monitor:\n  (default)\tPrimary Monitor\n  /display:N\tSpecific Monitor ID (1, 2, ...)\n\n"
        L"Example:\n  ScreenRotator.exe /right /display:2",
        L"Screen Rotator Help", MB_OK | MB_ICONINFORMATION);
}
static void ParseAndExecuteCommandLine(int argc, LPWSTR *argv) {
    DWORD orientation = DMDO_DEFAULT;
    int targetMode = TARGET_PRIMARY;
    int targetIndex = 0;
    BOOL actionFound = FALSE;
    BOOL helpFound = FALSE;
    int i;
    for (i = 1; i < argc; ++i) {
        if (lstrcmpiW(argv[i], L"/up") == 0 || lstrcmpiW(argv[i], L"-up") == 0) {
            orientation = DMDO_DEFAULT; actionFound = TRUE;
        } else if (lstrcmpiW(argv[i], L"/down") == 0 || lstrcmpiW(argv[i], L"-down") == 0) {
            orientation = DMDO_180; actionFound = TRUE;
        } else if (lstrcmpiW(argv[i], L"/left") == 0 || lstrcmpiW(argv[i], L"-left") == 0) {
            orientation = DMDO_270; actionFound = TRUE;
        } else if (lstrcmpiW(argv[i], L"/right") == 0 || lstrcmpiW(argv[i], L"-right") == 0) {
            orientation = DMDO_90; actionFound = TRUE;
        } else if (_wcsnicmp(argv[i], L"/display:", 9) == 0 || _wcsnicmp(argv[i], L"/monitor:", 9) == 0) {
            targetIndex = _wtoi(argv[i] + 9);
            targetMode = TARGET_INDEX;
        } else if (lstrcmpiW(argv[i], L"/?") == 0 || lstrcmpiW(argv[i], L"/help") == 0 || lstrcmpiW(argv[i], L"--help") == 0) {
            helpFound = TRUE;
        }
    }
    if (helpFound) {
        ShowHelpDialog();
    } else if (actionFound) {
        RotateScreen(orientation, targetMode, targetIndex);
    }
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HANDLE hMutex;
    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;
    int argc;
    LPWSTR *argv;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc > 1) {
        ParseAndExecuteCommandLine(argc, argv);
        LocalFree(argv);
        return 0;
    }
    LocalFree(argv);
    hMutex = CreateMutex(NULL, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"Another instance is already running.", APP_NAME, MB_OK | MB_ICONINFORMATION);
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClassEx(&wc)) {
        ShowError(L"Failed to register window class.");
        if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
        return 1;
    }
    hwnd = CreateWindowEx(0, CLASS_NAME, APP_NAME, WS_OVERLAPPEDWINDOW,
        0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        ShowError(L"Failed to create window.");
        if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
        return 1;
    }
    MinimizeMemory();
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return (int)msg.wParam;
}