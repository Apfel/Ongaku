// Copyright (c) 2020 Apfel
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software.
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <Windows.h>
#include <shellapi.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <discord_rpc.h>
#include <iTunes.h>

#define APP_ID "706172143624388670"
#define LARGE_ICON_NAME "itunes"

const static UINT Notify_Flags  = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;

#ifdef _DEBUG
const static GUID Notify_GUID   = { 0x926ad13b, 0x5c, 0x4849, { 0xb4, 0x7a, 0xab, 0xe7, 0xc5, 0x42, 0x21, 0x33 }};
#else
const static GUID Notify_GUID   = { 0xee2c6e8c, 0xe5d5, 0x47d1, { 0xb9, 0xeb, 0x3b, 0xa5, 0x94, 0x5e, 0x19, 0x3d } };
#endif

LRESULT CALLBACK Window_Process(HWND window, UINT message, WPARAM high, LPARAM low)
{
    static NOTIFYICONDATA nid;
    memset(&nid, 0, sizeof(nid));

    switch (message)
    {
    case WM_CREATE:
        HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(100));

        nid.cbSize              = sizeof(nid);
        nid.guidItem            = Notify_GUID;
        nid.hIcon               = icon;
        nid.hWnd                = window;
        nid.uCallbackMessage    = WM_APP + 1;
        nid.uFlags              = Notify_Flags;

        wchar_t* tip = TEXT("Ongaku (Setting up, right-click here to quit.)");
        wcscpy(nid.szTip, tip);

        if (!Shell_NotifyIcon(NIM_ADD, &nid))
        {
            Shell_NotifyIcon(NIM_DELETE, &nid);

            if (!Shell_NotifyIcon(NIM_ADD, &nid))
            {
                MessageBox(window, TEXT("An error occurred while adding the system tray icon."), TEXT("Ongaku"), MB_OK);
                PostMessageW(window, WM_DESTROY, 0, 0);
                break;
            }
        }

        if (!Shell_NotifyIcon(NIM_SETVERSION, &nid))
        {
            MessageBox(window, TEXT("An error occurred while adding the system tray icon."), TEXT("Ongaku"), MB_OK);
            PostMessageW(window, WM_DESTROY, 0, 0);
            break;
        }

        DestroyIcon(icon);
        return 0;

    case WM_APP + 1:
        switch (low)
        {
        case WM_RBUTTONDOWN:
            PostMessageW(window, WM_DESTROY, 0, 0);
            break;
        }

        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        DestroyWindow(window);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(window, message, high, low);
    }

    return 0;
}

bool Update_Notification(const wchar_t* message, uint32_t icon_id)
{
    HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(icon_id));

    NOTIFYICONDATA nid;

    nid.cbSize              = sizeof(nid);
    nid.hWnd                = GetActiveWindow();
    nid.uFlags              = Notify_Flags;
    nid.uCallbackMessage    = WM_APP + 1;
    nid.hIcon               = icon;
    nid.guidItem            = Notify_GUID;

    wcscpy(nid.szTip, message);

    bool result = Shell_NotifyIcon(NIM_MODIFY, &nid);

    DestroyIcon(icon);

    return result;
}

void Ready_Handler(const DiscordUser* user)
{
    if (user == NULL) return;

    wchar_t* message = calloc(129, sizeof(wchar_t));
    swprintf(message, 128, TEXT("Ongaku (Connected as %hs#%hs, right-click here to quit.)"), user->username, user->discriminator);

    Update_Notification(message, 101);

    free(message);
}

void Disconnect_Handler(int error_code, const char* error_message)
{
    wchar_t* message = calloc(129, sizeof(wchar_t));
    swprintf(message, 128, TEXT("Ongaku (Disconnected, error %d, right-click here to quit.)"), error_code);

    Update_Notification(message, 102);

    free(message);

    MessageBox(GetActiveWindow(), TEXT("The connection to Discord got cut. Ongaku will exit now."), TEXT("Ongaku"), MB_OK);
    PostMessageW(GetActiveWindow(), WM_DESTROY, 0, 0);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous_instance, LPWSTR arguments, int show_window)
{
    WNDCLASSEX window_class;
    ZeroMemory(&window_class, sizeof(window_class));

    window_class.cbSize         = sizeof(WNDCLASSEX);
    window_class.hIcon          = LoadIcon(instance, MAKEINTRESOURCE(100));
    window_class.hInstance      = instance;
    window_class.lpfnWndProc    = Window_Process;
    window_class.lpszClassName  = TEXT("Ongaku");

    if (RegisterClassEx(&window_class) == 0)
    {
        wchar_t message[75] = { 0 };
        swprintf(message, 75, TEXT("An error occurred while registering a window; error code %d."), GetLastError());
        MessageBox(NULL, message, TEXT("Ongaku"), MB_OK);
        return 1;
    }

    HWND window = CreateWindow(TEXT("Ongaku"), TEXT("Ongaku"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);
    if (window == NULL)
    {
        wchar_t message[75] = { 0 };
        swprintf(message, 75, TEXT("An error occurred while opening a window; error code %d."), GetLastError());
        MessageBox(NULL, message, TEXT("Ongaku"), MB_OK);
        return 1;
    }

    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));

    handlers.disconnected   = Disconnect_Handler;
    handlers.errored        = Disconnect_Handler;
    handlers.ready          = Ready_Handler;

    Discord_Initialize(APP_ID, &handlers, 0, NULL);
    Discord_RunCallbacks();

    iTunes_Instance* itunes = iTunes_New();
    if (itunes == NULL)
    {
        MessageBox(window, TEXT("A critical memory allocation error occurred."), TEXT("Ongaku"), MB_OK);

        DestroyWindow(window);

        return 3;
    }

    uint8_t result = iTunes_Initialize(itunes);
    switch (result)
    {
    case iTunes_Result_Success:
        break;

    case iTunes_Result_iTunes_Missing:
        wchar_t i_message[75] = { 0 };
        swprintf(i_message, 75, TEXT("It seems that iTunes may be missing or corrupt; error code %d-%d."), result, iTunes_GetLastError_Proxy(itunes));
        MessageBox(window, i_message, TEXT("Ongaku"), MB_OK);

        Discord_Shutdown();
        DestroyWindow(window);

        return 2;

    default:
        wchar_t n_message[75] = { 0 };
        swprintf(n_message, 75, TEXT("An error occurred while connecting to iTunes; error code %d-%d."), result, iTunes_GetLastError_Proxy(itunes));
        MessageBox(window, n_message, TEXT("Ongaku"), MB_OK);

        Discord_Shutdown();
        DestroyWindow(window);

        return 2;
    }

    MSG message;
    uint8_t return_code = 0;
    while (true)
    {
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        if (message.message == WM_QUIT) break;

        char                details[128];
        DiscordRichPresence presence;
        iTunes_State        state;
        char                state_value[128];
        time_t              time_value;

        memset(&state, 0, sizeof(state));
        memset(&presence, 0, sizeof(presence));

        time(&time_value);

        uint8_t result = iTunes_Get_State(itunes, &state);
        if (result != iTunes_Result_Success)
        {
            Discord_ClearPresence();

            wchar_t message[75] = { 0 };
            swprintf(message, 75, TEXT("An error occurred while fetching data from iTunes; error code %d-%d."), result, iTunes_GetLastError_Proxy(itunes));
            MessageBox(NULL, message, TEXT("Ongaku"), MB_OK);

            return_code = 2;
            break;
        }

        if (state.State == iTunes_Player_State_Playing)
        {
            snprintf(details, 128, "%ls", state.Title);
            snprintf(state_value, 128, "%ls - %ls", state.Artist, state.Album);

            presence.startTimestamp = time_value - (state.Position);
            presence.endTimestamp   = time_value + (state.Length - state.Position);
        }
        else
        {
            snprintf(details, 128, "");
            snprintf(state_value, 128, "Nothing in queue.");
            presence.startTimestamp = 0;
            presence.endTimestamp   = 0;
        }

        presence.state          = state_value;
        presence.details        = details;
        presence.largeImageKey  = LARGE_ICON_NAME;
        presence.largeImageText = "iTunes";

        Discord_UpdatePresence(&presence);
        Discord_RunCallbacks();
    }

    Discord_ClearPresence();
    Discord_Shutdown();

    iTunes_Destroy(itunes);

    UnregisterClass(window_class.lpszClassName, instance);

    return return_code;
}
