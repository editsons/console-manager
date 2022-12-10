#pragma once

#include <Windows.h>
#include "helpers.h"

namespace input {
	extern bool quit;
	extern bool spawn_console_flag;
	extern bool destroy_console_flag;
	void thread_callback();
}