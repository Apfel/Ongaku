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
#include <intrin.h>
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
static const GUID Notify_GUID = { 0x6de4add7, 0xc4db, 0x416b, { 0x86, 0x14, 0xb, 0xad, 0x5d, 0x29, 0x10, 0xd9 }};

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

        wchar_t* message = TEXT("Ongaku (Setting up, right-click here to quit.)");
        wcscpy_s(nid.szTip, sizeof(wchar_t) * 128, message);

        Shell_NotifyIcon(NIM_DELETE, &nid);

        Shell_NotifyIcon(NIM_ADD, &nid);        
        Shell_NotifyIcon(NIM_SETVERSION, &nid);

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
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(window, message, high, low);
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

    wcscpy_s(nid.szTip, sizeof(wchar_t) * wcslen(message), message);

    bool result = Shell_NotifyIcon(NIM_MODIFY, &nid);
    DestroyIcon(icon);

    return result;
}

void Ready_Handler(const DiscordUser* user)
{
    if (user == NULL) return;

    wchar_t* message = calloc(129, sizeof(wchar_t));
    swprintf_s(message, 128, TEXT("Ongaku (Connected as %hs#%hs, right-click here to quit.)"), user->username, user->discriminator);

    Update_Notification(message, 101);

    free(message);
}

void Disconnect_Message(LPVOID unused)
{
    MessageBox(GetActiveWindow(), TEXT("The connection to Discord got cut. Please restart Ongaku."), TEXT("Ongaku"), MB_OK);
}

void Disconnect_Handler(int error_code, const char* error_message)
{
    wchar_t* message = calloc(129, sizeof(wchar_t));
    swprintf_s(message, 128, TEXT("Ongaku (Disconnected, error %d, right-click here to quit.)"), error_code);

    Update_Notification(message, 102);

    free(message);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Disconnect_Message, NULL, 0, 0);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous_instance, LPWSTR arguments, int show_window)
{
    WNDCLASS window_class;
    ZeroMemory(&window_class, sizeof(window_class));

    window_class.hIcon          = LoadIcon(instance, MAKEINTRESOURCE(100));
    window_class.hInstance      = instance;
    window_class.lpfnWndProc    = Window_Process;
    window_class.lpszClassName  = TEXT("Ongaku");

    RegisterClass(&window_class);

    HWND window = CreateWindow(TEXT("Ongaku"), TEXT("Ongaku"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);
    if (window == NULL)
    {
        wchar_t message[75] = { 0 };
        swprintf_s(message, 75, TEXT("An error occurred while creating a window instance; error code %d."), GetLastError());
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
        swprintf_s(i_message, 75, TEXT("It seems that iTunes may be missing or corrupt; error code %d-%d."), result, iTunes_GetLastError_Proxy(itunes));
        MessageBox(window, i_message, TEXT("Ongaku"), MB_OK);

        Discord_Shutdown();
        DestroyWindow(window);
        
        return 2;

    default:
        wchar_t n_message[75] = { 0 };
        swprintf_s(n_message, 75, TEXT("An error occurred while connecting to iTunes; error code %d-%d."), result, iTunes_GetLastError_Proxy(itunes));
        MessageBox(window, n_message, TEXT("Ongaku"), MB_OK);

        Discord_Shutdown();
        DestroyWindow(window);
        
        return 2;
    }

    MSG message;
    uint8_t return_code = 0;
    while (true)
    {
        while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
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
            swprintf_s(message, 75, TEXT("An error occurred while fetching data from iTunes; error code %d-%d."), result, iTunes_GetLastError_Proxy(itunes));
            MessageBox(NULL, message, TEXT("Ongaku"), MB_OK);

            return_code = 2;
            break;
        }

        if (state.State == iTunes_Player_State_Playing)
        {
            sprintf_s(details, 128, "%ls", state.Title);
            sprintf_s(state_value, 128, "%ls - %ls", state.Artist, state.Album);

            presence.startTimestamp = time_value - (state.Position);
            presence.endTimestamp   = time_value + (state.Length - state.Position);
        }
        else
        {
            sprintf_s(details, 128, "");
            sprintf_s(state_value, 128, "Nothing in queue.");
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

    DestroyWindow(window);

    return return_code;
}
