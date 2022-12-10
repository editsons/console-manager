#include "helpers.h"

struct Window;

std::string get_this_path() {
    wchar_t this_path_buf[MAX_PATH];
    GetModuleFileName(NULL, this_path_buf, MAX_PATH);
    std::wstring this_path_buf_string(this_path_buf);
    std::string this_path(this_path_buf_string.begin(), this_path_buf_string.end());
    return this_path;
}

std::string get_window_title(HWND window) {
    wchar_t win_title_buf[MAX_PATH];
    GetWindowText(window, win_title_buf, MAX_PATH);
    std::wstring win_title_buf_string(win_title_buf);
    std::string win_title(win_title_buf_string.begin(), win_title_buf_string.end());
    return win_title;
}

DWORD start_cmd(std::string args_without_first) {

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    USES_CONVERSION_EX;
    LPWSTR cmdPath = A2W_EX("C:\\Windows\\System32\\cmd.exe");
    LPWSTR cmdArgs = A2W_EX(("C:\\Windows\\System32\\cmd.exe " + args_without_first).c_str());

    if (!CreateProcess(cmdPath,     // No module name (use command line)
        cmdArgs,                    // Command line
        NULL,                       // Process handle not inheritable
        NULL,                       // Thread handle not inheritable
        FALSE,                      // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,         // No creation flags
        NULL,                       // Use parent's environment block
        NULL,                       // Use parent's starting directory 
        &si,                        // Pointer to STARTUPINFO structure
        &pi)                        // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return 0;
    }

    // Wait until child process exits.
    //WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD process_id = pi.dwProcessId;

    // Close process and thread handles. 
    //CloseHandle(pi.hProcess);
    //CloseHandle(pi.hThread);

    return process_id;
}

// Structure used to communicate data from and to enumeration procedure
struct EnumData {
    DWORD dwProcessId;
    HWND hWnd;
};
// Application-defined callback for EnumWindows
BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam) {
    // Retrieve storage location for communication data
    EnumData& ed = *(EnumData*)lParam;
    DWORD dwProcessId = 0x0;
    // Query process ID for hWnd
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    // Apply filter - if you want to implement additional restrictions,
    // this is the place to do so.
    if (ed.dwProcessId == dwProcessId) {
        // Found a window matching the process ID
        ed.hWnd = hWnd;
        // Report success
        SetLastError(ERROR_SUCCESS);
        // Stop enumeration
        return FALSE;
    }
    // Continue enumeration
    return TRUE;
}
// Main entry
HWND FindWindowFromProcessId(DWORD dwProcessId) {
    EnumData ed = { dwProcessId };
    if (!EnumWindows(EnumProc, (LPARAM)&ed) &&
        (GetLastError() == ERROR_SUCCESS)) {
        return ed.hWnd;
    }
    return NULL;
}

bool spawn_console(std::vector<Window>& windows) {

    //std::string this_path = get_this_path();
    DWORD process_id = start_cmd("");

    if (process_id != 0) {

        const int waiting_duration = 200;
        const int max_waiting_duration = 3000;
        int waiting_duration_sum = 0;

        while (true) {

            HWND window_handle = FindWindowFromProcessId(process_id);
            if (window_handle == 0) {

                if (waiting_duration_sum >= max_waiting_duration) {
                    return false;
                }

                Sleep(waiting_duration);
                waiting_duration_sum += waiting_duration;

            }
            else {
                Window w;
                w.handle = window_handle;
                w.poll_data();
                windows.push_back(w);
                return true;
            }

        }
    }

}

void destroy_console(std::vector<Window>& windows) {

    // find first valid/open console
    for (int i = windows.size() - 1; i >= 0; i--) {
        if (IsWindow(windows[i].handle)) {

            // get process id of console window
            DWORD process_id = 1;
            GetWindowThreadProcessId(windows[i].handle, &process_id);
            if (process_id != NULL) {

                // kill process
                const auto explorer = OpenProcess(PROCESS_TERMINATE, false, process_id);
                TerminateProcess(explorer, 1);
                CloseHandle(explorer);

                // erase from list
                //windows.erase(begin(windows) + i);
                //i--;
            }
            break;
        }
    }

}

bool all_open(std::vector<Window>& windows) {
    for (int i = 0; i < windows.size(); i++) {
        if (!IsWindow(windows[i].handle)) {
            windows.erase(begin(windows) + i);
            i--;
        }
    }
    return windows.size() > 0;
}

bool key_down(int key_code) {
    return GetAsyncKeyState(key_code) >> 1;
}

bool key_released(int key_code) {
    static bool down_keys[255] = { false };
    if (down_keys[key_code] && !key_down(key_code)) {
        down_keys[key_code] = false;
        return true;
    }
    down_keys[key_code] = key_down(key_code);
    return false;
}

void adjust_consoles(std::vector<Window>& windows, const Window& main_window) {

    if (windows.size() == 0) return;

    // find main window index
    int main_window_index = 0;
    for (int i = 0; i < windows.size(); i++) {
        if (windows[i].handle == main_window.handle) {
            main_window_index = i;
            break;
        }
    }

    // adjust all consoles based on main_window
    int i = -main_window_index;
    for (Window& w : windows) {
        if (w.handle != main_window.handle) {   // skip main window
            w.set(main_window.x, main_window.y + (i * main_window.h),
                main_window.w, main_window.h);
        }
        i++;
    }
}

void regroup_consoles(std::vector<Window>& windows) {
    if (windows.size() == 0) return;
    adjust_consoles(windows, windows[0]);
}

void foreground_consoles(std::vector<Window>& windows) {
    for (Window& w : windows) {
        w.put_in_foreground();
    }
}