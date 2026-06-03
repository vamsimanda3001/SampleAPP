// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// RemoteDesktopProviderSample — exercises Windows.System.RemoteDesktop.Provider APIs
//
// This is the orchestrator: it initializes logging, loads .env, checks package
// identity, creates a window (needed for WindowId-based APIs in later phases),
// then calls into per-class demo modules on WM_CREATE.

#include "pch.h"

#include "utils.h"
#include "RemoteDesktopInfo.h"
#include "RemoteDesktopRegistrar.h"

// ─── Forward declarations ───────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// ─── Window procedure ──────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        if (UnlockLimitedAccessFeature())
        {
            DemoRemoteDesktopInfo();
            DemoRemoteDesktopRegistrar();
        }
        else
        {
            Log(L"LAF unlock failed — skipping Provider API demos.");
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ─── Entry point ────────────────────────────────────────────────────────────
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    winrt::init_apartment();
    InitLog();
    LoadEnvFile();

    Log(L"RemoteDesktopProviderSample starting... (PID=%u)", GetCurrentProcessId());

    CheckPackageIdentity();

    // Register window class
    const wchar_t className[] = L"RDPProviderSampleClass";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        className,
        L"Remote Desktop Provider Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        Log(L"Failed to create window (error %u)", GetLastError());
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CloseLog();
    winrt::uninit_apartment();
    return static_cast<int>(msg.wParam);
}
