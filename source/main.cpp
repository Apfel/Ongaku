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

#define APP_ID 706172143624388670

#pragma warning (disable:26451)
#pragma warning (disable:26812)

#include "icon.h"

#include <chrono>
#include <string>
#include <thread>

#include <csignal>
#include <ctime>

#include <comdef.h>
#include <shellapi.h>
#include <windows.h>

#include <discord.h>
#include <iTunesCOMInterface.h>

static volatile bool interrupted = false;
static HWND window;

class OngakuApp
{
public:
    std::unique_ptr<discord::Core> Core;
    discord::User User;
};

std::string tcharToString(const TCHAR* text)
{
#ifdef UNICODE
    std::vector<char> buffer;
    int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
    if (size > 0)
    {
        buffer.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, text, -1, static_cast<LPSTR>(&buffer[0]), buffer.size(), NULL, NULL);
    }
    else return "";

    std::string content(&buffer[0]);
#else
    std::string content(text);
#endif

    return content;
}

void ErrorBox(const char* message) { MessageBoxA(window, (LPCSTR)message, "Ongaku", MB_OK | MB_ICONERROR); }

HICON GetIcon()
{ 
    HICON icon = NULL;

    int offset = LookupIconIdFromDirectoryEx((BYTE*)&Icon, TRUE, 0, 0, LR_DEFAULTCOLOR);
    if (offset != 0) icon = CreateIconFromResourceEx((BYTE*)&Icon + offset, IconSize, TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);

    return icon;  
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    static NOTIFYICONDATA nid;

    switch (iMsg)
    {
    case WM_CREATE:
        memset(&nid, 0, sizeof(nid));

        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = 0;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_APP + 1;
        nid.hIcon = GetIcon();

        lstrcpy(nid.szTip, L"Ongaku (Running, right-click here to quit.)");

        Shell_NotifyIcon(NIM_ADD, &nid);
        Shell_NotifyIcon(NIM_SETVERSION, &nid);
        return 0;

    case WM_APP + 1:
        switch (lParam)
        {
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_CONTEXTMENU:
                interrupted = true;
                break;
        }

        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, LPSTR cmdLine, int cmdShow)
{
    WNDCLASS wc;
    LPCWSTR winClass = L"__hidden__";

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = nullptr;
    wc.hCursor = nullptr;
    wc.hIcon = GetIcon();
    wc.hInstance = inst;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = winClass;
    wc.lpszMenuName = nullptr;
    wc.style = 0;
    RegisterClass(&wc);

    window = CreateWindow(winClass, winClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, inst, nullptr);

    OngakuApp* app = new OngakuApp;

    discord::Core* core;
    discord::Result result = discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
    if (result != discord::Result::Ok)
    {
        std::string content = "Failed to connect to Discord: \"" + std::to_string(static_cast<int>(result)) + "\".";
        ErrorBox(content.c_str());

        delete core;
        delete app;

        return 1;
    }

    app->Core.reset(core);

    ::CoInitialize(NULL);

    IiTunes* itunes;
    HRESULT comResult = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID*)&itunes);

    if (comResult != S_OK)
    {
        _com_error err(comResult);
        std::string content = "Connecting to iTunes failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
        ErrorBox(content.c_str());        
        ::CoDisconnectObject(NULL, 0);
        return 2;
    }

    discord::Activity activity{};
    std::time_t res = std::time(nullptr);

    activity.SetApplicationId(APP_ID);
    activity.SetType(discord::ActivityType::Listening);
    activity.GetAssets().SetLargeImage("itunes");
    activity.GetAssets().SetLargeText("iTunes");

    long position, length;
    BSTR author, title;
    IITTrack* current;
    ITPlayerState state;
    std::chrono::milliseconds timeMs;
    long long timeLong;

    std::signal(SIGINT, [](int) { interrupted = true; });
    while (!interrupted)
    {
        result = app->Core->RunCallbacks();
        if (result != discord::Result::Ok)
        {
            std::string content = "Failed to run the Discord callback: \"" + std::to_string(static_cast<int>(result)) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        comResult = itunes->get_PlayerState(&state);
        if (comResult != S_OK)
        {
            _com_error err(comResult);
            if (tcharToString(err.ErrorMessage()) == "Call was rejected by callee.") break;

            std::string content = "Fetching the current player state failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        if (state == NULL)
        {
            activity.SetDetails("");
            activity.SetState("Paused.");
            activity.GetTimestamps() = discord::ActivityTimestamps{};

            app->Core->ActivityManager().UpdateActivity(activity, [](discord::Result result)
            {
                if (result == discord::Result::Ok) return;

                std::string content = "Failed to update activity: \"" + std::to_string(static_cast<int>(result)) + "\"";
                ErrorBox(content.c_str());
                interrupted = true;
            });

            continue;
        }

        comResult = itunes->get_CurrentTrack(&current);
        if (comResult != S_OK)
        {
            _com_error err(comResult);
            std::string content = "Fetching the current track failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        comResult = current->get_Name(&title);
        if (comResult != S_OK)
        {
            _com_error err(comResult);
            std::string content = "Fetching the name of the current track failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        comResult = current->get_Artist(&author);
        if (comResult != S_OK)
        {
            _com_error err(comResult);
            std::string content = "Fetching the artist of the current track failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        activity.SetDetails(_com_util::ConvertBSTRToString(title));
        activity.SetState(_com_util::ConvertBSTRToString(author));

        comResult = itunes->get_PlayerPosition(&position);
        if (comResult != S_OK)
        {
            _com_error err(comResult);
            std::string content = "Fetching the current position failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        timeLong = timeMs.count();
        activity.GetTimestamps().SetStart(timeLong - (position * 1000));

        comResult = current->get_Duration(&length);
        if (comResult != S_OK)
        {
            _com_error err(comResult);
            std::string content = "Fetching the length of the current track failed: \"" + tcharToString(err.ErrorMessage()) + "\".";
            ErrorBox(content.c_str());
            break;
        }

        activity.GetTimestamps().SetEnd(timeLong + (length * 1000 - position * 1000));

        app->Core->ActivityManager().UpdateActivity(activity, [](discord::Result result)
        {
            if (result == discord::Result::Ok) return;
            
            std::string content = "Failed to update activity: \"" + std::to_string(static_cast<int>(result)) + "\"";
            ErrorBox(content.c_str());
            interrupted = true;
        });

        std::this_thread::sleep_for(std::chrono::seconds(6));
    }

    itunes->Release();
    app->Core->ActivityManager().ClearActivity(nullptr);

    return 0;
}
