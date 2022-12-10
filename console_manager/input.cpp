#include "input.h"

namespace input {

	bool quit = false;
	bool spawn_console_flag = false;
	bool destroy_console_flag = false;

	void thread_callback() {
		while (!quit) {

			static bool plus_key_down = false;
			static bool minus_key_down = false;

			if (!plus_key_down) {
				plus_key_down = (GetAsyncKeyState(VK_ADD) >> 1);
			}
			else if (!(GetAsyncKeyState(VK_ADD) >> 1)) {
				// event plus key released
				plus_key_down = false;
				if (GetAsyncKeyState(VK_CONTROL) >> 1) {
					spawn_console_flag = true;
				}
			}

			if (!minus_key_down) {
				minus_key_down = (GetAsyncKeyState(VK_SUBTRACT) >> 1);
			}
			else if (!(GetAsyncKeyState(VK_SUBTRACT) >> 1)) {
				// event minus key released
				minus_key_down = false;
				if (GetAsyncKeyState(VK_CONTROL) >> 1) {
					destroy_console_flag = true;
				}
			}
			
			Sleep(1);

		}
	}
}