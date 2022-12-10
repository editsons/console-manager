#pragma once

#include <string>
#include <Windows.h>
#include <atlbase.h>
#include <iostream>
#include <vector>

struct Window {
    HWND handle = NULL;
    int x;
    int y;
    int w;
    int h;

    bool poll_data() {
        RECT rect = { NULL };
        if (GetWindowRect(this->handle, &rect)) {
            this->x = rect.left;
            this->y = rect.top;
            this->w = rect.right - rect.left;
            this->h = rect.bottom - rect.top;
            return true;
        }
        return false;
    }

    bool set(int x, int y, int w, int h) {
        if (MoveWindow(this->handle, x, y, w, h, true)) {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
            //if (BringWindowToTop(this->handle)) {
               // return true;
            //}
            return put_in_foreground();
        }
        return false;
    }

    bool put_in_foreground() {
        return SetWindowPos(this->handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW) &&
               SetWindowPos(this->handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }

    void assign(int x, int y, int w, int h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    bool has_equal_props(Window w) {
        if (this->x != w.x || this->y != w.y ||
            this->w != w.w || this->h != w.h)
            return false;
        return true;
    }

    bool has_unequal_props(Window w) {
        return !has_equal_props(w);
    }

    void print() {
        std::cout << "x: " << x << ", y: " << y << ", w: " << w << ", h: " << h << std::endl;
    }

    bool set_default() {
        RECT desktop;
        const HWND hDesktop = GetDesktopWindow();
        GetWindowRect(hDesktop, &desktop);
        int screen_w = desktop.right;
        int screen_h = desktop.bottom;

        int x = screen_w / 8;
        int y = screen_h / 15;
        int w = screen_w - 2 * x;
        int h = screen_h / 3;
        return set(x, y, w, h);
    }
};

std::string get_this_path();
std::string get_window_title(HWND window);
DWORD start_cmd(std::string args_without_first);
HWND FindWindowFromProcessId(DWORD dwProcessId);
bool spawn_console(std::vector<Window>& windows);
void destroy_console(std::vector<Window>& windows);
bool all_open(std::vector<Window>& windows);
void adjust_consoles(std::vector<Window>& windows, const Window& main_window);
void regroup_consoles(std::vector<Window>& windows);
void foreground_consoles(std::vector<Window>& windows);