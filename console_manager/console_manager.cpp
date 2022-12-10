#include <Windows.h>
#include "helpers.h"
#include <iostream>
#include "input.h"
#include <thread>

using namespace std;


#define UPDATE_DELAY (1)


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

    // get command line params
    int argc = 0;
    LPWSTR* args_LPWSTR = CommandLineToArgvW(lpCmdLine, &argc);
    vector<string> args(argc);
    for (int i = 0; i < argc; i++) {
        wstring ws(args_LPWSTR[i]);
        args[i] = string(ws.begin(), ws.end());
    }

    // alloc console for debugging
    /*AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);*/

    // windows list: stores current open windows
    vector<Window> windows;

    // spawn master console and set default position and dimensions
    if (!spawn_console(windows)) return 0;
    //windows.back().set_default();

    // spawn more consoles
    //spawn_console(windows);
    //spawn_console(windows);

    // regroup consoles
    regroup_consoles(windows);

    // start input thread
    std::thread input_thread(input::thread_callback);

    // main loop
    while (true) {

        // check if a window has changed its position or dimension (=> main window)
        Window main_window;
        int i = 0;
        for (Window& w : windows) {

            Window w_polled;
            w_polled.handle = w.handle;

            if (w_polled.poll_data() && w.has_unequal_props(w_polled)) {
                w = w_polled;
                main_window = w;
                break;
            }
            i++;
        }

        // if a window has changed then adjust all other windows
        if (main_window.handle != NULL) {
            adjust_consoles(windows, main_window);
        }

        // check if some windows were closed and remove closed windows from windows list
        bool some_windows_closed = false;
        for (int i = 0; i < windows.size(); i++) {
            if (!IsWindow(windows[i].handle)) {
                some_windows_closed = true;
                windows.erase(begin(windows) + i);
                i--;
            }
        }

        // if some windows were closed then break main loop if all were closed else regroup consoles
        if (some_windows_closed) {
            if (windows.size() == 0) {
                break;
            }
            regroup_consoles(windows);
        }

        // check if some window is in foreground
        bool a_window_in_foreground = false;
        HWND foreground_window = GetForegroundWindow();
        for (Window w : windows) {
            if (w.handle == foreground_window) {
                a_window_in_foreground = true;
                break;
            }
        }

        // if windows are in foreground then check for create or destory console flag (is set by input thread)
        if (a_window_in_foreground) {
            if (input::spawn_console_flag) {
                input::spawn_console_flag = false;
                spawn_console(windows);
                regroup_consoles(windows);
            }
            else if (input::destroy_console_flag) {
                input::destroy_console_flag = false;
                destroy_console(windows);
            }
        }

        // check if windows shifted into foreground
        static bool a_window_was_in_foreground = true;
        if (a_window_in_foreground && !a_window_was_in_foreground) {
            foreground_consoles(windows);
        }
        a_window_was_in_foreground = a_window_in_foreground;

        // delay update
        Sleep(UPDATE_DELAY);
    }

    // end input thread
    input::quit = true;
    input_thread.join();
}










